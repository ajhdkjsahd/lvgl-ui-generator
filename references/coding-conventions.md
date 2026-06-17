# LVGL 代码规范 & 常见陷阱

> 基于 `lv_port_linux` 项目实践。遵循此规范确保生成的代码可直接编译运行。

---

## 1. 文件模板

### 头文件 (.h)

```c
/**
 * @file my_page.h
 * @brief 我的页面
 *
 * @author LVGL UI Generator
 */

#ifndef MY_PAGE_H
#define MY_PAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

/**
 * @brief 创建我的页面
 * @return 屏幕对象指针
 */
lv_obj_t * my_page_create(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MY_PAGE_H */
```

### 源文件 (.c)

```c
/**
 * @file my_page.c
 * @brief 我的页面实现
 */

/*********************
 *      INCLUDES
 *********************/
#include "my_page.h"
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/
#define CARD_COUNT 6
#define HEADER_HEIGHT 64

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_obj_t * card;
    lv_obj_t * title_label;
} card_data_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_style_t style_card;
static lv_style_t style_title;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void on_card_clicked(lv_event_t * e);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * my_page_create(void)
{
    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, 1440, 800);

    // ... 构建页面

    return screen;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void on_card_clicked(lv_event_t * e)
{
    lv_obj_t * target = lv_event_get_target(e);
    LV_LOG_USER("卡片被点击");
}
```

---

## 2. 命名约定

| 类别 | 约定 | 示例 |
|------|------|------|
| 文件名 | snake_case，`_page` 后缀 | `home_page.c`、`settings_page.c` |
| 创建函数 | `<name>_create`，返回 `lv_obj_t *` | `home_page_create()` |
| 静态样式 | `style_` 前缀 | `style_card`、`style_title` |
| 静态回调 | `on_` 前缀 | `on_btn_click`、`on_slider_change` |
| 控件变量 | 语义化名称 | `title_bar`、`card_container`、`nav_btn` |
| 常量宏 | `UPPER_SNAKE_CASE` | `CARD_COUNT`、`HEADER_HEIGHT` |

---

## 3. 内存管理规则

### LVGL 控件：不需要手动 free
```c
lv_obj_t * btn = lv_button_create(parent);  // LVGL 内部管理
// 删除时调用
lv_obj_delete(btn, 0);  // 立即删除（延迟 0ms）
// 不要用 free(btn)！
```

### lv_malloc / lv_free：LVGL 分配的数据用 LVGL 释放
```c
void * buf = lv_malloc(size);
// ...
lv_free(buf);
// 不要混用标准 malloc/free
```

### 回调中的 user_data：生命周期由创建者管理
```c
// 如果 user_data 是动态分配的
static char * label_text = lv_malloc(64);
lv_obj_add_event_cb(btn, on_click, LV_EVENT_CLICKED, label_text);

// 在 LV_EVENT_DELETE 中释放
static void on_click(lv_event_t * e) {
    if(lv_event_get_code(e) == LV_EVENT_DELETE) {
        void * data = lv_event_get_user_data(e);
        lv_free(data);
    }
}
```

### 定时器：不用手动删除
```c
lv_timer_t * timer = lv_timer_create(timer_cb, 1000, NULL);
// LVGL 会在定时器不再需要时自动清理
// 手动停止：lv_timer_delete(timer);
```

---

## 4. 样式最佳实践

### 复用样式
```c
// ✅ 好：一个样式多次使用
static lv_style_t style_card;
lv_style_init(&style_card);
lv_style_set_bg_color(&style_card, lv_color_hex(0xFFFFFF));
lv_style_set_radius(&style_card, 8);

for(int i = 0; i < 6; i++) {
    lv_obj_t * card = lv_obj_create(container);
    lv_obj_add_style(card, &style_card, LV_PART_MAIN | LV_STATE_DEFAULT);
}
```

### 内联样式用于一次性设置
```c
// ✅ 好：单个控件临时样式
lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF5722), LV_PART_MAIN);
lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN);
```

### 样式选择器组合
```c
// 不同状态不同样式
lv_obj_add_style(btn, &style_default, LV_PART_MAIN | LV_STATE_DEFAULT);
lv_obj_add_style(btn, &style_pressed, LV_PART_MAIN | LV_STATE_PRESSED);
lv_obj_add_style(btn, &style_disabled, LV_PART_MAIN | LV_STATE_DISABLED);
```

---

## 5. 屏幕适配规则

### 使用相对布局而非绝对像素
```c
// ✅ 好：用对齐实现响应式
lv_obj_t * header = lv_obj_create(screen);
lv_obj_set_size(header, lv_pct(100), 64);        // 宽度 100%，高度固定 64
lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

lv_obj_t * content = lv_obj_create(screen);
lv_obj_set_size(content, lv_pct(100), lv_pct(100));  // 占满剩余空间
lv_obj_align(content, LV_ALIGN_TOP_LEFT, 0, 64);
// 或者直接用 Flex 让空间自动分配

// ✅ 好：用 Flex/Grid 自动分配
lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

// ❌ 差：硬编码绝对像素
lv_obj_set_pos(btn, 100, 50);
lv_obj_set_size(btn, 200, 60);
```

### 触摸友好尺寸
```c
// 按钮最小 40×40 像素
lv_obj_set_size(btn, LV_SIZE_CONTENT, 48);  // 高度至少 48px
// 列表项高度 60+ px
```

---

## 6. 中文处理

### UTF-8 编码注意
```c
// ✅ 源文件必须是 UTF-8 编码
// 编译器不会自动转换 GBK → UTF-8
lv_label_set_text(label, "智能家居控制中心");  // OK

// ❌ 避免在宏里放中文（某些编译器有问题）
#define TITLE "智能家居"  // 可能有编码问题，用 const char * 更安全
static const char * TITLE = "智能家居";  // ✅
```

### 字体选择策略
```c
// 16px 中文 → 用内置 CJK 字体（零成本）
lv_obj_set_style_text_font(obj, &lv_font_source_han_sans_sc_16_cjk, 0);

// 20+ 中文 → 用 Tiny TTF（需要 .ttf 文件部署到板子）
static lv_font_t * cjk_24;
cjk_24 = lv_tiny_ttf_create_file("/root/fonts/NotoSansSC-Regular.ttf", 24);
lv_obj_set_style_text_font(title, cjk_24, 0);

// 英文/数字 → 用 Montserrat（内建，清晰美观）
lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, 0);
```

---

## 7. v8 → v9 迁移陷阱表

| 类别 | v8 API（不要用） | v9 API（正确） |
|------|------------------|-----------------|
| 按钮控件类名 | `lv_btn` | `lv_button` |
| 按钮创建 | `lv_btn_create(parent)` | `lv_button_create(parent)` |
| 图片控件类名 | `lv_img` | `lv_image` |
| 图片创建 | `lv_img_create(parent)` | `lv_image_create(parent)` |
| 动画图像 | `lv_animimg` | `lv_animimg`（同名，但部分函数变了） |
| 活动屏幕 | `lv_scr_act()` | `lv_screen_active()` |
| 加载屏幕 | `lv_scr_load(scr)` | `lv_screen_load(scr)` |
| 屏幕加载动画 | `lv_scr_load_anim(...)` | `lv_screen_load_anim(...)` |
| 获取顶层 | `lv_layer_top()` | `lv_layer_top()`（同名） |
| 获取系统层 | `lv_layer_sys()` | `lv_layer_sys()`（同名） |
| 设置显示 | `lv_scr_load` | `lv_screen_load` |
| tileview 切换 | `lv_obj_set_tile(obj, tile, anim)` | `lv_tileview_set_tile(tv, tile, anim)` |
| tileview 按索引 | `lv_obj_set_tile_id(obj, col, row, anim)` | `lv_tileview_set_tile_by_index(tv, col, row, anim)` |
| 样式属性 | `lv_style_set_bg_opa` → `lv_style_set_bg_opa`（同名 OK） | 129 个属性基本同名 |
| 事件注册 | 参数顺序: `(obj, cb, filter, data)` | 相同 `(obj, cb, filter, data)` — 但 `filter` 类型改名为 `lv_event_code_t` |
| FLEX flow | `lv_obj_set_flex_flow`（同名 OK） | API 未变 |
| GRID 描述符 | `lv_obj_set_grid_dsc_array`（同名 OK） | API 未变 |
| 对齐到对象 | `lv_obj_align_to(obj, ref, align, x, y)`（同名 OK） | API 未变 |

---

## 8. 常见编译错误

### `implicit declaration of function`
```c
// 原因：缺少 #include
// 修复：确保每个 .c 文件顶部有 #include "lvgl/lvgl.h"
```

### `undefined reference to 'xxx_page_create'`
```c
// 原因：.c 文件没有加入 CMakeLists.txt 的 add_executable
// 修复：在 CMakeLists.txt 的 add_executable(lvglsim ...) 中添加对应的 .c 文件
```

### `'LV_PART_xxx' undeclared`
```c
// 原因：v8 的 LV_PART_xxx 在 v9 中可能改名或移除
// 常见：LV_PART_ANY 已移除，用 LV_PART_MAIN
```

### `conflicting types for 'lv_xxx'`
```c
// 原因：同时包含了 v8 和 v9 的头文件，或旧代码与新代码冲突
// 修复：确保只用 v9 API
```

---

## 9. 多页面导航模式

### 预创建所有页面（推荐，内存足够时）
```c
// main.c
static lv_obj_t * home_screen;
static lv_obj_t * settings_screen;
static lv_obj_t * device_screen;

// 初始化时创建
home_screen = home_page_create();
settings_screen = settings_page_create();
device_screen = device_page_create();

// 设置回调：点击设置按钮 → 切换
static void on_nav_settings(lv_event_t * e) {
    lv_screen_load_anim(settings_screen, LV_SCREEN_LOAD_ANIM_OVER_LEFT, 300, 0, false);
}
```

### 延迟创建（内存紧张时）
```c
// 只在需要时创建
static void on_nav_settings(lv_event_t * e) {
    lv_obj_t * settings = settings_page_create();  // 创建
    lv_screen_load(settings);
    // 旧屏幕会自动隐藏但不会被删除（除非 auto_del=true）
}
```

### 返回上一页
```c
// 在返回按钮的回调中
static void on_back_click(lv_event_t * e) {
    lv_obj_t * current = lv_screen_active();
    lv_obj_t * previous = get_previous_screen();  // 自己管理屏幕栈
    if(previous) {
        lv_screen_load_anim(previous, LV_SCREEN_LOAD_ANIM_OVER_RIGHT, 300, 0, false);
    }
}
```

---

## 10. 交互反馈最佳实践

```c
// 按钮点击：给视觉反馈
static void on_btn_pressed(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_set_style_bg_opa(btn, LV_OPA_80, LV_PART_MAIN);  // 按下时变暗
}

static void on_btn_released(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN); // 恢复
}

// 消息弹窗
static void show_confirm_dialog(const char * title, const char * msg) {
    lv_obj_t * mb = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mb, title);
    lv_msgbox_add_text(mb, msg);
    lv_msgbox_add_footer_button(mb, "取消");
    lv_obj_t * ok_btn = lv_msgbox_add_footer_button(mb, "确定");
    lv_obj_add_event_cb(ok_btn, on_dialog_ok, LV_EVENT_CLICKED, mb);
}

// Toast 提示（临时弹出后自动消失）
static void show_toast(const char * msg) {
    lv_obj_t * toast = lv_label_create(lv_layer_top());
    lv_label_set_text(toast, msg);
    lv_obj_set_style_bg_color(toast, lv_color_hex(0x333333), 0);
    lv_obj_set_style_text_color(toast, lv_color_white(), 0);
    lv_obj_set_style_radius(toast, 8, 0);
    lv_obj_set_style_pad_all(toast, 16, 0);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -40);
    // 2 秒后自动删除
    lv_obj_delete(toast, 2000);
}
```

---

## 11. 项目特定信息

### 屏幕和硬件
- 分辨率: 1440×800
- 色深: 16-bit (RGB565)
- 触摸: ft5x0x, `/dev/input/event0`
- 帧缓冲: `/dev/fb0`
- 平台: GEC6818 (ARM Cortex-A53, S5P6818)

### 项目路径
- 项目根: `lv_port_linux/`
- 页面代码: `src/pages/`
- 编译产物: `build/bin/lvglsim`
- 配置文件: `lv_conf.defaults`
- 交叉编译: `user_cross_compile_setup.cmake`

### lp_conf.defaults 已启用的关键功能
- LV_COLOR_DEPTH = 16
- LV_USE_LINUX_FBDEV = 1
- LV_USE_EVDEV = 1
- LV_USE_TINY_TTF = 1
- LV_USE_FS_STDIO = 1
- LV_BUILD_DEMOS = 1
- LV_USE_DEMO_WIDGETS / BENCHMARK / STRESS / MUSIC = 1
- LV_FONT_SOURCE_HAN_SANS_SC_16_CJK = 1
- LV_USE_LOG = 1
- LV_USE_PERF_MONITOR = 1
- LV_USE_VECTOR_GRAPHIC = 1

### 交叉编译命令
```bash
cd ~/26粤嵌园区实训/02-lv_port_linux/lv_port_linux
rm -rf build
cmake -B build -DCMAKE_TOOLCHAIN_FILE=./user_cross_compile_setup.cmake
cmake --build build -j$(nproc)
scp build/bin/lvglsim root@<IP>:/root/
```
