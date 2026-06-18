---
name: lvgl-ui-generator
description: "Use when user wants to design or generate LVGL UI pages for embedded Linux (ARM/Cross-compile). Covers full workflow: resolution collection → layout design → code generation → font handling → build integration → deploy. Also use when user asks about LVGL v9 API patterns, style system, flex/grid layout, or Chinese font rendering on LVGL."
---

# LVGL UI Code Generator

生成嵌入式 Linux 平台的 LVGL UI 代码，覆盖全流程：需求分析 → 布局设计 → C 代码生成 → 构建集成 → 部署。

**适用平台**: LVGL v9 + fbdev/evdev 嵌入式 Linux（ARM 交叉编译）

---

## 核心原则

1. **先理解后编码**：不急于写代码，先用 ASCII 示意图确认布局
2. **一屏一文件**：每个页面独立 `.c` + `.h`，放在 `src/ui/pages/`
3. **v9 API 为准**：不生成 v8 API（`lv_scr_act` → `lv_screen_active` 等）
4. **可编译可运行**：生成的代码必须配套构建修改和部署命令
5. **所有 UI 资源统一收纳在 `src/ui/`**：fonts、pages 等子目录，方便整体移植
6. **严禁出现非预期的滚动条**（见下方「滚动条铁律」）

---

## 滚动条铁律（必读）

嵌入式设备的 LVGL 界面**不允许出现**让用户拖拽滚动的滚动条——手指无法精细滚动，且违背一次性看全信息的设计原则。

### 根因（为什么会出现滚动条）

只要满足以下任一条件，LVGL 就会**自动**给容器加滚动条：
1. 子元素的 `width` 或 `height` 超出容器尺寸
2. `lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP)` 且文字换行后总高 > 容器可用高度
3. 容器本身没有用 `lv_obj_set_size` 限定，且内容溢出

### 设计层面的根本要求

布局必须做到 **「内容 ≤ 容器」**：
- 卡片内的所有文字、图标必须在给定的卡片尺寸内放得下
- 列宽 × 行数 = 总宽 × 总高，所有像素用尽，不留溢出
- 文字过长时**截断**（`LONG_CLIP`），**不换行**（`LONG_WRAP`）
- 字号要按容器可用宽度反推：宽度 200px ÷ 16px 字号 ≈ 12 个汉字

### 代码层面的硬性要求

每个控件创建后，**必须**立即设置滚动行为：

```c
/* 1. 容器本身：禁止滚动 */
lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

/* 2. 文字标签：固定宽度 + 截断模式 */
lv_obj_set_width(label, 200);                          /* 固定像素 */
lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);     /* 截断，禁止 WRAP */

/* 3. Tab 内容页等长页面：可整体滚动，但内部所有 row/card 禁滚 */
lv_obj_set_scroll_dir(parent, LV_DIR_VER);             /* 只允许垂直 */
```

### 工厂函数（推荐使用，避免重复代码）

在 `src/ui/pages/smart_home.h` 中声明 helper：

```c
lv_obj_t * create_inline_row(lv_obj_t * parent, lv_coord_t w, lv_coord_t h);
```

实现（默认已禁滚动）：

```c
lv_obj_t * create_inline_row(lv_obj_t * parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_set_size(row, w, h);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);  /* 行容器不拦截点击, 让事件穿透到父对象 */
    return row;
}
```

### 标签字号速算

按 16px 字体（中英文等宽）：

| 容器宽度 | 最多汉字 | 最多英文 | 实际可写 |
|----------|---------|---------|---------|
| 100px | 6 个 | 12 字符 | 短标签（"客厅灯"） |
| 200px | 12 个 | 24 字符 | 设备名 + 短描述 |
| 350px | 22 个 | 44 字符 | 完整描述 |

若要展示更长内容：**拆成多行**（label + label），不要用 `LONG_WRAP`。

### 自检清单

- [ ] 所有卡片内部 `lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF)`
- [ ] 所有 row 容器使用 `create_inline_row` 或手写 `LV_SCROLLBAR_MODE_OFF`
- [ ] 所有 `lv_label_create` 后跟 `lv_obj_set_width` + `LV_LABEL_LONG_CLIP`
- [ ] 全局 grep `LV_LABEL_LONG_WRAP` 应**无匹配**
- [ ] Tab 内容页（设备页等）允许整页滚动，但其内所有子容器都禁滚

### 常见错误

| 现象 | 原因 | 修复 |
|------|------|------|
| 标签下方出现滑条 | 文字宽度超过标签 width | 加大 width 或缩短文字 |
| 卡片下方出现滑条 | 卡片内多行 label 总高 > 卡片高 | 改用截断 / 减少内容 / 加大卡片 |
| Tab 内容有滑条 | 内容总高 > Tab 高度 | 整页允许滚（合理），但**子容器**不能滚 |

---

## Flex vs Grid 选择指南

LVGL v9 两种布局引擎，根据场景二选一：

### Flex（弹性盒子）

**适合：一维排列**

```c
// 横向行：图标+文字、开关行、滑块行、底部按钮栏
lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

// 纵向列：卡片内图标→标题→描述 从上到下
lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
```

| Flex 场景 | FLOW | ALIGN |
|-----------|------|-------|
| 图标 + 文字 同行 | `ROW` | `START, CENTER, CENTER` |
| 卡片内纵向堆叠 | `COLUMN` | `CENTER, CENTER, CENTER` |
| 开关行（标签+开关） | `ROW` | `SPACE_BETWEEN, CENTER, CENTER` |
| 底部按钮栏 | `ROW` | `SPACE_EVENLY, CENTER, CENTER` |

### Grid（网格）

**适合：二维矩阵排列**

```c
lv_obj_set_size(grid, 776, 330);
static int32_t col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
static int32_t row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, col, 1,
                            LV_GRID_ALIGN_STRETCH, row, 1);
```

| Grid 场景 | 行列 | 列宽/行高 |
|-----------|------|-----------|
| 场景卡片 2×4 | `row_dsc={FR,FR}` `col_dsc={FR×4}` | 均分 |
| 仪表盘 2×2 | `row_dsc={FR,FR}` `col_dsc={FR,FR}` | 均分 |
| 设置菜单 5×1 | `row_dsc={FR×5}` `col_dsc={FR}` | 单列多行 |

### 决策树

```
内容是单行/单列？
  ├── 是 → Flex (create_inline_row)
  └── 否 → 二维矩阵？
            ├── 是 → Grid + LV_GRID_FR(1)
            └── 否 → Flex COLUMN 嵌套 Flex ROW
```

---

## 交互模式（常用模板）

### snprintf 动态文本

状态卡片、传感器数值等需要拼接中文文本时，必须用 `snprintf`（禁止 `sprintf`）：

```c
char val_buf[32];

// 布尔状态 + 数值 + 中文单位
snprintf(val_buf, sizeof(val_buf), "%s %d度 %s",
    g_home_state.ac_power ? "运行中" : "已关闭",
    g_home_state.ac_temp,
    g_ac_mode_names[g_home_state.ac_mode]);

// 数值 + 百分比
snprintf(val_buf, sizeof(val_buf), "%.1f度", (double)g_sensor_data.temperature);
snprintf(val_buf, sizeof(val_buf), "%s 亮度 %d%%",
    g_home_state.living_light ? "已开启" : "已关闭",
    g_home_state.living_light_brightness);

// 多个设备状态拼接（用空格分隔）
snprintf(val_buf, sizeof(val_buf), "插座%s  热水器%s",
    g_home_state.outlet ? "已开启" : "已关闭",
    g_home_state.water_heater ? "已开启" : "已关闭");
```

**规则：**
- buf 至少 32 字节，长文本 64
- 中英文空格分隔（`%s %d度 %s`）
- `float` 转 `double` 再传：`(double)g_sensor_data.temperature`
- `%%` 输出字面百分号（`%d%%`）

### 事件回调模式

```c
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void on_btn_click(lv_event_t * e);
static void on_switch_change(lv_event_t * e);
static void on_slider_change(lv_event_t * e);

/**********************
 *   注册（创建时）
 **********************/
lv_obj_add_event_cb(btn, on_btn_click, LV_EVENT_CLICKED, (void *)(uintptr_t)index);
lv_obj_add_event_cb(sw, on_switch_change, LV_EVENT_VALUE_CHANGED, NULL);
lv_obj_add_event_cb(slider, on_slider_change, LV_EVENT_VALUE_CHANGED, NULL);

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void on_btn_click(lv_event_t * e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    lv_obj_t * target = lv_event_get_target(e);
    LV_LOG_USER("按钮 %lu 被点击", (unsigned long)idx);
}

static void on_switch_change(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    bool state = lv_obj_has_state(sw, LV_STATE_CHECKED);
    LV_LOG_USER("开关切换: %s", state ? "ON" : "OFF");
}
```

**常用事件码：**

| 事件码 | 适用控件 | 触发时机 |
|--------|---------|---------|
| `LV_EVENT_CLICKED` | button, 可点击容器 | 按下+释放 |
| `LV_EVENT_VALUE_CHANGED` | switch, slider, dropdown | 值改变 |
| `LV_EVENT_PRESSED` | 任意控件 | 按下瞬间 |
| `LV_EVENT_DELETE` | 任意控件 | 控件被删除 |

**user_data 传参规范：**
```c
// 传整数索引：双重转换
(void *)(uintptr_t)i              // 注册时
uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);  // 取出时

// 传字符串：直接转
lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, (void *)"hello");
const char * s = (const char *)lv_event_get_user_data(e);

// 传结构体指针：需要保证生命周期
lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, &my_data);
```

---

### Step 1 需求收集

向用户确认以下信息（已有默认值的可跳过）：

| 项目 | 默认值 | 用户可覆盖 |
|------|--------|-----------|
| 屏幕分辨率 | 800×480 | 自定义 |
| 颜色深度 | 16 (RGB565) | 32 |
| 主题风格 | 浅色/深色 | 自定义色板 |
| 页面结构 | 单页 | 多页 + 导航 |
| 字体 | lv_font_conv + chars.txt（多字号） | 调整 --size 参数 |
| 交互方式 | 触摸 | 触摸 + 按键 |

### Step 2 布局设计

在写代码之前，先用 ASCII 示意图向用户展示布局方案。

### Step 3 代码生成

**3.1 字体生成（必须，在写页面代码之后完成）**

工作流：
```
写页面 .c/.h 代码（所有中文已确定）
  → Python 扫描 src/ui/pages/ 收集汉字 → src/ui/fonts/chars.txt
  → lv_font_conv 按需生成各字号 .c → src/ui/fonts/app_16.c, app_24.c, ...
  → 写 src/ui/fonts/app_fonts.h 声明所有字体变量
  → CMakeLists.txt GLOB 收集 src/ui/fonts/*.c + src/ui/pages/*.c
  → 代码中 #include "src/ui/fonts/app_fonts.h" 引用字体
```

**3.2 页面代码**：按文件约定章节的规范生成。

### Step 4 构建集成 / Step 5 编译部署 / Step 6 质量自检

---

## 文件约定

### 目录结构

```
src/
├── main.c
└── ui/                       # 所有 UI 资源，方便整体移植
    ├── fonts/
    │   ├── chars.txt          # Python 自动收集的汉字列表
    │   ├── app_16.c           # lv_font_conv 生成的 16px 字库
    │   ├── app_24.c           # lv_font_conv 生成的 24px 字库
    │   └── app_fonts.h        # 字体 extern 声明
    └── pages/
        ├── smart_home.h       # 共享头文件
        ├── smart_home.c
        ├── home_page.c
        ├── device_page.c
        └── scene_page.c
```

---

## 图标显示规范

**LVGL 内置符号**基于 Font Awesome，通过 `LV_SYMBOL_*` 宏使用，详见 `lvgl/include/lvgl/font/lv_symbol_def.h`。

**核心规则：`LV_SYMBOL_*` 的码位在 Montserrat 字体的私有区，和中文字体不互通。同一个 label 不能同时渲染中文和图标 — 必须拆成两个 label，各用各的字体：**

```c
// ✅ 图标 label（montserrat） + 中文 label（自定义字体），用 row 包裹
lv_obj_t * row = create_inline_row(parent, 776, 36);

lv_obj_t * icon = lv_label_create(row);
lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
lv_label_set_text(icon, LV_SYMBOL_HOME);

lv_obj_t * text = lv_label_create(row);
lv_obj_set_style_text_font(text, &lv_font_smart_home_24, 0);
lv_obj_set_style_pad_left(text, 8, 0);   // 间距用 padding，不写空格
lv_label_set_text(text, "智能家居控制中心");
```

**常用符号映射表**

| 含义 | LVGL 符号 | 备注 |
|------|----------|------|
| 首页/回家 | `LV_SYMBOL_HOME` | 房屋图标 |
| 电源/开关 | `LV_SYMBOL_POWER` | 电源按钮 |
| 关闭/离开 | `LV_SYMBOL_CLOSE` | X 号 |
| 确认/开启 | `LV_SYMBOL_OK` | 勾号 |
| 设置 | `LV_SYMBOL_SETTINGS` | 齿轮 |
| 播放/视频 | `LV_SYMBOL_PLAY` / `LV_SYMBOL_VIDEO` | 播放三角 / 影片 |
| 暂停 | `LV_SYMBOL_PAUSE` | 双竖线 |
| 上一曲/下一曲 | `LV_SYMBOL_PREV` / `LV_SYMBOL_NEXT` | 左右箭头 |
| 音量 | `LV_SYMBOL_VOLUME_MID` / `LV_SYMBOL_VOLUME_MAX` | 音量图标 |
| 静音 | `LV_SYMBOL_MUTE` | 静音图标 |
| 电池/充电 | `LV_SYMBOL_CHARGE` | 闪电 |
| WiFi | `LV_SYMBOL_WIFI` | WiFi 图标 |
| 刷新/循环 | `LV_SYMBOL_REFRESH` / `LV_SYMBOL_LOOP` | 循环箭头 |
| 上传/下载 | `LV_SYMBOL_UPLOAD` / `LV_SYMBOL_DOWNLOAD` | 箭头 |
| 文件 | `LV_SYMBOL_FILE` | 文件图标 |
| 图像 | `LV_SYMBOL_IMAGE` | 图片框 |
| 左/右/上/下 | `LV_SYMBOL_LEFT` / `LV_SYMBOL_RIGHT` / `LV_SYMBOL_UP` / `LV_SYMBOL_DOWN` | 方向箭头 |
| 加/减 | `LV_SYMBOL_PLUS` / `LV_SYMBOL_MINUS` | + / - |
| 警告 | `LV_SYMBOL_WARNING` | 三角感叹号 |
| 铃声 | `LV_SYMBOL_BELL` | 铃铛 |
| 剪贴板 | `LV_SYMBOL_COPY` / `LV_SYMBOL_PASTE` | 复制粘贴 |
| 保存 | `LV_SYMBOL_SAVE` | 软盘图标 |
| 列表 | `LV_SYMBOL_LIST` / `LV_SYMBOL_BARS` | 列表图标 |
| 目录 | `LV_SYMBOL_DIRECTORY` | 文件夹 |
| 电话 | `LV_SYMBOL_CALL` | 电话 |
| GPS | `LV_SYMBOL_GPS` | 定位 |
| 键盘 | `LV_SYMBOL_KEYBOARD` | 键盘 |
| 邮件 | `LV_SYMBOL_ENVELOPE` | 信封 |

### 完整符号列表

参见：`lvgl/include/lvgl/font/lv_symbol_def.h`

---

## 中文显示（核心规范）

**禁止：内置 CJK 字体**
```c
// lv_font_source_han_sans_sc_16_cjk 汉字极少，严禁使用！
```

**唯一方案：lv_font_conv + chars.txt**

**Step A: 收集字符**
```bash
python3 -c "
import os, codecs
cjk = set()
for f in os.listdir('src/ui/pages'):
    if f.endswith(('.c','.h')):
        for ch in open(f'src/ui/pages/{f}', encoding='utf-8').read():
            if '一' <= ch <= '鿿':
                cjk.add(ch)

# 固定补充：空格（中英混排必备），如需其他符号（如 °）也在此添加
fixed = ' '

with open('src/ui/fonts/chars.txt', 'w', encoding='utf-8') as out:
    out.write(fixed + ''.join(sorted(cjk)))
"
echo \"Total chars: $(wc -m < src/ui/fonts/chars.txt)\"
```

**Step B: 按字号生成字库（每个字号一个 .c 文件）**

| 用途 | 推荐字号 |
|------|---------|
| 页面大标题 | 24px |
| 正文/标签/按钮 | 16px |
| 小字/辅助信息 | 14px |

```bash
SYMBOLS=$(cat src/ui/fonts/chars.txt)

lv_font_conv \
  --font lvgl/scripts/built_in_font/SourceHanSansSC-Normal.otf \
  --size 16 --bpp 4 --format lvgl \
  -r 0x20-0x7F --symbols "$SYMBOLS" \
  --no-compress \
  --lv-include "lvgl/lvgl.h" \
  --lv-font-name "lv_font_app_16" \
  -o src/ui/fonts/app_16.c
```

**必须加 `--no-compress`**，否则 `LV_USE_FONT_COMPRESSED` 未开启时字形读不出，文字不显示、布局错乱。

**WSL 1 注意**：`lv_font_conv` 是 Node.js 工具，WSL 1 不支持原生安装 Node.js，但可以通过 Windows 侧的 Node.js 运行。wsl 的 `bash` 无法直接调用 `node.exe`（路径转换问题），需要用 **PowerShell** 作为中转：

```bash
# WSL 1 字体生成方法（lv_font_conv 已装在 Windows D:\nodejs\ 下）
# 1. 先把文件拷到 Windows 原生路径
PROJDIR=~/26粤嵌园区实训/02-lv_port_linux/fhb_lv_port_linux
mkdir -p /mnt/d/temp_lv_font_conv
cp "$PROJDIR/lvgl/scripts/built_in_font/SourceHanSansSC-Normal.otf" /mnt/d/temp_lv_font_conv/
cp "$PROJDIR/src/ui/fonts/chars.txt" /mnt/d/temp_lv_font_conv/

# 2. 用 PowerShell 调用 Windows 的 node.exe 执行 lv_font_conv
SYMBOLS=$(cat /mnt/d/temp_lv_font_conv/chars.txt)
powershell.exe -Command "cd D:\\temp_lv_font_conv; D:\\nodejs\\node.exe D:\\nodejs\\node_modules\\lv_font_conv\\lv_font_conv.js --font SourceHanSansSC-Normal.otf --size 16 --bpp 4 --format lvgl -r 0x20-0x7F --symbols $SYMBOLS --no-compress --no-prefilter --lv-font-name lv_font_app_16 -o app_16.c"

# 3. 拷回项目
cp /mnt/d/temp_lv_font_conv/app_16.c "$PROJDIR/src/ui/fonts/"
```

> 如果已在 Linux 环境或 WSL 2 中，直接用 `lv_font_conv` 命令即可，无需上述 workaround。

**`-r 0x20-0x7F` 提供空格、数字、大小写字母、常用标点**，保证中英混排时 "亮度 80%" 这类文本正常显示。

**Step C: 字体头文件**
```c
// src/ui/fonts/app_fonts.h
#ifndef APP_FONTS_H
#define APP_FONTS_H
#include "lvgl/lvgl.h"
extern const lv_font_t lv_font_app_16;
extern const lv_font_t lv_font_app_24;
#endif
```

**Step D: 代码中使用**
```c
#include "src/ui/fonts/app_fonts.h"
lv_obj_set_style_text_font(title, &lv_font_app_24, 0);
lv_obj_set_style_text_font(label, &lv_font_app_16, 0);
```

**CMakeLists.txt**：
```cmake
file(GLOB LV_PAGE_SRC src/ui/pages/*.c)
file(GLOB LV_FONT_SRC src/ui/fonts/*.c)
add_executable(lvglsim src/main.c ${LV_PAGE_SRC} ${LV_FONT_SRC})
```

---

## 构建集成

### CMakeLists.txt

```cmake
file(GLOB LV_PAGE_SRC src/ui/pages/*.c)
file(GLOB LV_FONT_SRC src/ui/fonts/*.c)
add_executable(lvglsim src/main.c ${LV_PAGE_SRC} ${LV_FONT_SRC})
```

### main.c

```c
#include "src/ui/pages/smart_home.h"

// 替换 demo
lv_obj_t * smart_home = smart_home_create();
lv_screen_load(smart_home);
```

---

## 编译部署

```bash
rm -rf build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=./user_cross_compile_setup.cmake
cmake --build build -j$(nproc)

scp build/bin/lvglsim root@<板子IP>:/root/
ssh root@<板子IP> ./lvglsim
```

---

## 质量检查清单

- [ ] 所有 API 是 v9 风格（`lv_button_create` / `lv_screen_active` / `lv_obj_add_event_cb`）
- [ ] 每个页面独立 `.c` + `.h`，头文件有 include guard
- [ ] 所有 UI 资源在 `src/ui/` 下，**不要散落根目录**
- [ ] Python 扫描 `src/ui/pages/` → `src/ui/fonts/chars.txt`
- [ ] 按字号分别生成字库，每个生成命令都加 `--no-compress`
- [ ] 字体声明在 `src/ui/fonts/xxx_fonts.h`，通过 `#include "src/ui/fonts/xxx_fonts.h"` 引用
- [ ] **禁止**使用 `lv_font_source_han_sans_sc_16_cjk`
- [ ] CMakeLists.txt 已 `GLOB` `src/ui/pages/*.c` 和 `src/ui/fonts/*.c`
- [ ] main.c 调用了 `xxx_create()` + `lv_screen_load()`

---

## 常见错误（⚠️ 实战经验，每条都是血泪教训）

| 现象 | 根因 | 正确做法 |
|------|------|----------|
| `lv_btn_create` (v8) | API 过时 | `lv_button_create` |
| `lv_scr_act()` (v8) | API 过时 | `lv_screen_active()` |
| **`error: expected expression` 在回调函数定义处** 🔥 | 用了 C++ lambda `[](lv_event_t *e) { ... }`，C 语言不支持 lambda | 全部改为 `static void on_xxx(lv_event_t *e)` 命名函数，在文件顶部声明 prototype |
| **`error: initializer element is not constant`** 🔥 | `lv_color_hex(0x...)` 不是编译期常量（是函数调用），不能用于 `static const lv_color_t arr[N]` 的初始化 | 改为运行时赋值：`static lv_color_t arr[N]; arr[0]=COL_X; arr[1]=COL_Y;` |
| **`error: too many arguments to function 'lv_obj_set_size'`** 🔥 | `lv_obj_set_size(obj, w, h, LV_PART_KNOB)` 传了第4个参数，但该函数只接受 3 个参数（obj, w, h），不支持按 part 设置尺寸 | 用 `lv_obj_set_style_width(obj, w, LV_PART_KNOB)` + `lv_obj_set_style_height(obj, h, LV_PART_KNOB)` 分开设置 |
| 中文乱码/缺字 | 字库里没这个字 | 重跑 Python 扫描 → chars.txt → lv_font_conv 重新生成 |
| 字体不显示+控件乱跳 | 缺少 `--no-compress` | lv_font_conv 命令必须加 `--no-compress` |
| 字体/页面散落根目录 | 目录不规范 | 统一放进 `src/ui/{fonts,pages}/` |
| 样式不生效 | 未 init style | 先 `lv_style_init(&style)` 再设置属性 |
| **图标显示方框** 🔥 | LV_SYMBOL 和中文字体在同一个 label | 图标+中文必须拆成两个 label，图标用 `lv_font_montserrat_*`，中文用自定义字体 |
| **Dropdown 箭头显示方框** 🔥 | `lv_obj_set_style_text_font(dd, &chinese_font, 0)` 覆盖了指示器字体 | 必须额外设置 `LV_PART_INDICATOR`：`lv_obj_set_style_text_font(dd, &lv_font_montserrat_16, LV_PART_INDICATOR)` |
| **掉电符（°C）显示方框** | ° 是 U+00B0，SourceHanSansSC 可能没有 / 没在 chars.txt 里 | 直接用中文「度」代替 °C → `"26度"` |
| **cjk 字体格式 0 无 glyph** | 用 `-r 0x20-0x7F` 代替 `--symbols` 提供 ASCII | `-r 0x20-0x7F` 和 `--symbols "$SYMBOLS"` **必须同时使用** |
| **页面不该滚却滚了** | 内容总高 > 480 - TabBar高度(≈48px) = 432px | 精确计算：pad + title + grid + gaps ≤ 432px |
| **charmap 字体总大小 0** | 只改了 chars.txt 没重新生成字体 .c 文件 | cmake 只负责**编译**，不负责**生成**字体 — 必须先 `lv_font_conv` 再 cmake |
| **Grid 卡片内描述太长** | 单行文字超过卡片宽度导致截断 | 用 `\n` 拆成两行，`LV_LABEL_LONG_CLIP` 模式下 `\n` 依然生效 |
| **windows npx/unc 路径报错** | WSL 1 无法直接调用 Windows 的 node.exe（路径转换问题） | 用 PowerShell 中转（见上方 Step B 的 WSL 1 说明） |
| **按钮中部点击无反应，只有边缘一小块能点** 🔥 | 按钮和 label 之间夹了 `create_inline_row` 容器；`lv_label` 构造时移除 CLICKABLE 但 `lv_obj` 保留 CLICKABLE，中间容器拦截点击且 EVENT_BUBBLE 未开启无法冒泡到按钮 | ① `create_inline_row` **必须**加 `lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE)` ② label 必须是按钮的**直接子对象**，中间不要夹容器 |
| **Flex 布局导致底部 Tab 栏被裁切/不显示** | 顶层屏幕用 flex 布局时，子控件布局在首个 `lv_timer_handler()` 完成后才生效，`build` 阶段 `lv_obj_get_width/height` 返回 0 | 顶层屏幕用绝对定位：`lv_obj_set_pos(top_bar, 0, 0); lv_obj_set_pos(content, 0, 30); lv_obj_set_pos(tab_bar, 0, 284)`，不用 flex |
| **按钮的 label 同时包含图标和中文，中文显示方框** | `LV_SYMBOL_*` 的码位在 Montserrat 字体私有区，和中文字体不互通 | 图标 label（`lv_font_montserrat_*`）和中文 label（自定义字体）拆成两个独立 label，用 `create_inline_row` 包裹 |

---

## 🔴 深度教训：按钮点击失灵（LVGL v9 命中测试机制）

### 现象

按钮中部点击无任何反应，只有边缘 2-3px 的区域能触发 `LV_EVENT_CLICKED`。

### 完整的命中测试链路

LVGL v9 通过 `lv_indev_search_obj` 递归搜索目标对象，规则是**最深优先（depth-first, last-child-first）**：

```
lv_indev_search_obj(obj, point):
  1. 检查 point 是否在 obj->coords 范围内
  2. 如果在 → 递归搜索所有子对象（从最后一个子对象向前遍历）
  3. 如果某个子对象命中 → 立即返回该子对象（不再查找）
  4. 如果没有子对象命中 → 检查 obj 自己的 lv_obj_hit_test()
     → 检查 LV_OBJ_FLAG_CLICKABLE
     → 检查 point 是否在 lv_obj_get_click_area() 内
     → 如果通过 → 返回 obj，否则返回 NULL
```

### 关键事实

| 对象类型 | `LV_OBJ_FLAG_CLICKABLE` 默认值 | 说明 |
|----------|-------------------------------|------|
| `lv_obj` (普通) | **ON** | 所有对象默认可点击 |
| `lv_label` | **OFF** (构造时移除) | `lv_label.c:789` `lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE)` |
| `lv_button` | **ON** | 继承自 `lv_obj` |

### 事件冒泡规则 (`event_is_bubbled`)

- `LV_EVENT_CLICKED` **默认不冒泡**
- 只有设置了 `LV_OBJ_FLAG_EVENT_BUBBLE` 的对象才会将 CLICKED 向上传递
- `lv_obj` 和 `lv_label` **默认都不设此 flag**

### 为什么会失灵

当按钮结构是 `button → inner_row(create_inline_row) → icon_label + text_label` 时：

```
点击按钮中部
  → lv_indev_search_obj 先找到 text_label
    → lv_label::hit_test → CLICKABLE=OFF → 穿透 ✗
  → 再找 icon_label
    → lv_label::hit_test → CLICKABLE=OFF → 穿透 ✗
  → 两个 label 都没命中 → 回到 inner_row
    → lv_obj::hit_test → CLICKABLE=ON → 命中！✓
  → indev_obj_act = inner_row
  → LV_EVENT_CLICKED 发给 inner_row
    → inner_row 没 EVENT_BUBBLE → 事件死在这里 ✗
    → button 的 on_tab_click 永远不会被调用 ✗
```

只有点击按钮边缘、inner_row 未覆盖的 2-3px 区域时，搜索直接命中 button 本身，事件才正常工作。

### 为什么 DC/步进页面的按钮正常

那些按钮的结构是 `button → label`（label 是 button 的**直接子对象**）：

```
点击按钮
  → lv_indev_search_obj 先找 label
    → lv_label::hit_test → CLICKABLE=OFF → 穿透 ✗
  → label 没命中 → 回到 button
    → lv_button::hit_test → CLICKABLE=ON → 命中！✓
  → indev_obj_act = button
  → button 的 handler 被正确调用 ✓
```

### 修复方案

**方案一（推荐）**：在 `create_inline_row` 工厂函数中清除 CLICKABLE：

```c
lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);  // 行容器不拦截点击
```

**方案二**：让 label 成为 button 的直接子对象，不要在中间夹容器。

**方案三**：给中间容器加 EVENT_BUBBLE：
```c
lv_obj_add_flag(inner, LV_OBJ_FLAG_EVENT_BUBBLE);
```
⚠️ 不推荐：这会让所有事件都冒泡，可能引发意外副作用。

### 最佳实践

```c
// ✅ 正确：label 是 button 的直接子对象
lv_obj_t *btn = lv_button_create(parent);
lv_obj_t *lbl = lv_label_create(btn);   // 直接子对象
lv_label_set_text(lbl, "点击我");

// ✅ 正确：如果必须用容器，清 CLICKABLE
lv_obj_t *btn = lv_button_create(parent);
lv_obj_t *inner = create_inline_row(btn, 64, 20);  // 已含 clear CLICKABLE
lv_obj_t *icon = lv_label_create(inner);
lv_obj_t *text = lv_label_create(inner);

// ❌ 错误：容器在 button 和 label 之间，且未清 CLICKABLE
lv_obj_t *btn = lv_button_create(parent);
lv_obj_t *inner = lv_obj_create(btn);   // CLICKABLE=ON，会拦截点击！
lv_obj_t *lbl = lv_label_create(inner);
```

---

## 🔴 深度教训：Flex 布局导致控件位置/尺寸异常

### 现象

用 flex 布局的顶层屏幕，底部 tab 栏或控件在 `build` 阶段 `lv_obj_get_width/height` 返回 0，位置错乱或被裁切。

### 根因

LVGL v9 的布局更新在 `lv_timer_handler()` 循环中异步执行。控件创建和 `lv_obj_set_size` 只设置**样式值**，不立即更新 `obj->coords`。`obj->coords` 在 `lv_obj_refr_size()` 中更新，发生在 `lv_timer_handler()` → 布局更新流程中。

```c
// build 阶段（lv_timer_handler 还没跑）
lv_obj_t *bar = create_tab_bar(parent);  // lv_obj_set_size(bar, 240, 36)
lv_obj_get_width(bar);   // → 0！（coords 还没更新）
lv_obj_get_height(bar);  // → 0！（coords 还没更新）
```

### 修复方案

顶层屏幕使用**绝对定位**，不用 flex 布局：

```c
lv_obj_t *scr = lv_obj_create(NULL);
lv_obj_set_size(scr, 240, 320);
// 不设 flex_flow，手动 set_pos

g_motor.top_bar = build_top_bar(scr);
lv_obj_set_pos(g_motor.top_bar, 0, 0);     // y=0, h=30

g_motor.content = lv_obj_create(scr);
lv_obj_set_size(g_motor.content, 240, 254);
lv_obj_set_pos(g_motor.content, 0, 30);     // y=30, h=254

g_motor.tab_bar = build_tab_bar(scr);
lv_obj_set_pos(g_motor.tab_bar, 0, 284);    // y=284, h=36
// 0+30+254+36 = 320 ✓ 像素严格分配，无重叠无间隙
```

### 什么时候该用 Flex

Flex 仍然适合**子容器内部**的布局（按钮栏、卡片内行、标题行等）。规则：**顶层屏幕用绝对定位，子容器内用 flex**。

---

## 🔴 深度教训：C 语言编译陷阱（LVGL 示例 vs C 项目实战）

LVGL 官网文档和 demo 大量使用 C++ 特性（lambda、constexpr 等），在纯 C 项目中直接照搬会导致编译失败。

### 陷阱 1：Lambda 表达式（C 不支持）

**错误代码（照搬 LVGL 文档）：**
```c
lv_obj_add_event_cb(btn, [](lv_event_t *e) {
    LV_LOG_USER("clicked");
}, LV_EVENT_CLICKED, NULL);
```
**编译器报错**: `error: expected expression`

**C 语言正确写法**：
```c
// 文件开头声明
static void on_btn_click(lv_event_t *e);

// 创建时注册
lv_obj_add_event_cb(btn, on_btn_click, LV_EVENT_CLICKED, (void *)(uintptr_t)idx);

// 文件末尾实现
static void on_btn_click(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    LV_LOG_USER("按钮 %lu 被点击", (unsigned long)idx);
}
```

**规范**：
- 所有回调必须是 `static` 命名函数
- 函数声明在文件顶部 `STATIC PROTOTYPES` 区域
- 用 `(void *)(uintptr_t)idx` 传递整数参数

### 陷阱 2：`lv_color_hex()` 不是编译期常量

**错误代码**：
```c
static const lv_color_t accents[4] = {
    lv_color_hex(0x00E5FF),  // COL_CYAN
    lv_color_hex(0xFF1744),  // COL_STOP
    // ...
};
```
**编译器报错**: `error: initializer element is not constant`

**C 语言正确写法**：
```c
// 方案 A：运行时赋值（推荐）
static lv_color_t accents[4];
accents[0] = COL_CYAN;
accents[1] = COL_STOP;
accents[2] = COL_CYAN;
accents[3] = COL_BORDER;

// 方案 B：用宏（如果已经是宏定义的）
#define COL_CYAN  lv_color_hex(0x00E5FF)
// 但 lv_color_hex 内部是函数调用，宏展开后同理
// 结论：C 中不要用 static const lv_color_t arr[] = {...lv_color_hex...}
```

### 陷阱 3：`lv_obj_set_size()` 只接受 3 个参数

**错误代码（照搬某些 v9 迁移指南）**：
```c
lv_obj_set_size(slider, 8, 16, LV_PART_KNOB);  // 想设置 knob 尺寸
```
**编译器报错**: `error: too many arguments to function 'lv_obj_set_size'`

**C 语言正确写法**：
```c
// lv_obj_set_size 只能设置对象整体尺寸，不支持按 part
lv_obj_set_size(slider, 232, 16);                           // 整体
lv_obj_set_style_width(slider, 8, LV_PART_KNOB);            // knob 宽度
lv_obj_set_style_height(slider, 16, LV_PART_KNOB);          // knob 高度
```

### C vs C++ 速查表

| 写法 | C 编译器 | 说明 |
|------|---------|------|
| `[](lv_event_t *e) { ... }` | ❌ 编译失败 | C 不支持 lambda |
| `static void cb(lv_event_t *e)` | ✅ | 命名函数 |
| `static const lv_color_t x = lv_color_hex(0x...)` | ❌ 编译失败 | 不是编译期常量 |
| `static lv_color_t x; x = lv_color_hex(0x...);` | ✅ | 运行时赋值 |
| `lv_obj_set_size(obj, w, h, part)` | ❌ 编译失败 | 参数过多 |
| `lv_obj_set_style_width/height(obj, v, part)` | ✅ | 按 part 设置尺寸 |
| `lv_obj_set_style_*(obj, ...)` | ✅ | v9 推荐写法 |

### 为什么会踩坑

LVGL 官方文档和 PC 模拟器示例代码常以 **C++** 编译（`.cpp` 文件），而嵌入式项目通常用 **C** 编译（`.c` 文件）。两者的语法差异在编译时才会暴露。**所有生成的 `.c` 文件必须用纯 C 语法，不可包含任何 C++ 特性。**

---

## 参考资料

- 详细 API: `references/lvgl-v9-api-cheatsheet.md`
- 代码规范: `references/coding-conventions.md`
- LVGL 官方: https://docs.lvgl.io/
