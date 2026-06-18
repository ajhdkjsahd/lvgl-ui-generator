# Motor Control UI — LVGL v9 参考模板

## 概览

240×320 竖屏电机控制系统 UI，3 页 Tab 导航：
- **概览页** — 系统状态 + 两电机卡片 + 进度条
- **步进电机页** — 速度/步数滑块 + 细分选择 + 方向/使能按钮
- **直流电机页** — PWM 滑块 + V/I/P 实时显示 + 保护配置 + 方向/使能按钮

## 设计特点

| 特性 | 实现 |
|------|------|
| 配色 | 工业深色金属风 (`#0F1419` 暗底 + `#00E5FF` 青 / `#FFB300` 橙) |
| 布局 | **顶层屏幕绝对定位** + 子容器 flex 布局 |
| Tab 切换 | 200ms 淡入淡出动画，底栏 3 按钮 |
| 动画 | 状态点呼吸动画 (800ms α 循环)、进度条弹性动画 (500ms ease-out) |
| 字体 | 14px SourceHanSansSC (`lv_font_app_14`) + Montserrat 14 (符号) |
| 交互 | 按钮点击、滑块拖动、chip 选择（C 命名函数回调，非 lambda） |

## 文件清单

```
motor_control.h      — 色板、类型定义、全局状态、工厂函数声明
motor_control.c      — 主屏幕骨架、顶栏、底栏 Tab、动画、页面切换
page_overview.h/c    — 概览页（系统状态条 + 电机卡片 + 底部信息）
page_stepper.h/c     — 步进电机页（滑块 + 细分 chip + 控制按钮）
page_dc.h/c          — 直流电机页（PWM + V/I/P + 保护 + 控制按钮）
app_fonts.h          — 字体 extern 声明
chars.txt            — 中文字符表 (lv_font_conv 的 --symbols 参数)
```

## 关键模式（值得复用）

### 1. 顶层绝对定位 + 子容器 flex
```c
// 屏幕层不用 flex，手动 set_pos
lv_obj_t *scr = lv_obj_create(NULL);
lv_obj_set_pos(top_bar, 0, 0);
lv_obj_set_pos(content, 0, 30);
lv_obj_set_pos(tab_bar, 0, 284);

// 子容器内部用 flex 布局
lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
```

### 2. 工厂函数封装
```c
create_inline_row(parent, w, h)   // 横向 flex 行（已禁滚动 + 不拦截点击）
create_card(parent, w, h, border) // 工业风卡片
create_label(parent, txt, font, color, w)  // 固定宽度 label (LONG_CLIP)
create_chip(parent, txt, w, h)    // 可点击 chip
style_button_industrial(btn, accent)       // 按钮一键工业风装扮
```

### 3. 三页 Tab 导航
- 3 个 page 容器绝对定位叠放，通过 `LV_OBJ_FLAG_HIDDEN` 切换
- 切换时旧页 `lv_obj_fade_out` + 新页 `lv_obj_fade_in` (200ms)
- 顶栏标题动态更新 + Tab 按钮描边高亮切换

### 4. 全局状态 + 跨页引用
- `motor_ctrl_t g_motor` 全局单例
- 各页面的动态控件引用保存在 `g_motor.sp_*` / `g_motor.dc_*` 中
- 滑块回调中直接更新对应 label 的文本

### 5. C 语言规范
- 所有回调用 `static` 命名函数，无 lambda
- `lv_color_t` 数组运行时赋值，不用 `static const` 初始化
- `lv_obj_set_style_width/height` 设 knob 尺寸，不用 `lv_obj_set_size`

## 字体生成

```bash
lv_font_conv \
  --font SourceHanSansSC-Normal.otf \
  --size 14 --bpp 4 --format lvgl \
  -r 0x20-0x7F --symbols "$(cat chars.txt)" \
  --no-compress \
  -o app_14.c --lv-font-name lv_font_app_14
```

## main.c 集成

```c
#include "src/ui/pages/motor_control.h"

sdl_hal_init(240, 320);
lv_obj_t *motor_scr = motor_control_create();
lv_screen_load(motor_scr);
```

## CMakeLists.txt

```cmake
file(GLOB LV_PAGE_SRC src/ui/pages/*.c)
file(GLOB LV_FONT_SRC src/ui/fonts/*.c)
list(APPEND MAIN_SOURCES ${LV_PAGE_SRC} ${LV_FONT_SRC})
```
