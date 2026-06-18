# LVGL v9 主题系统

## 核心原则

**先主题，后覆盖**。用 `lv_theme_default_init` 建立全局风格基准，只对差异化部分使用内联样式。能减少 60%+ 的手写样式代码。

---

## 基础用法

```c
// main.c 中初始化显示后，一行设置全局主题
lv_theme_t * th = lv_theme_default_init(disp,
    lv_palette_main(LV_PALETTE_BLUE),    // 主色调
    lv_palette_main(LV_PALETTE_GREY),    // 辅色调
    LV_THEME_DEFAULT_DARK,               // 深色/浅色
    &lv_font_app_16);                    // 默认字体
```

### 参数说明

| 参数 | 说明 | 常用值 |
|------|------|--------|
| `disp` | 显示设备 | `lv_display_get_default()` |
| `primary_color` | 主色调（按钮、开关、选中态） | `lv_palette_main(LV_PALETTE_BLUE)` |
| `secondary_color` | 辅色调（滑块轨道、进度条背景） | `lv_palette_main(LV_PALETTE_GREY)` |
| `dark_light` | 深色/浅色模式 | `LV_THEME_DEFAULT_DARK` 或 `LV_THEME_DEFAULT_LIGHT` |
| `default_font` | 全局默认字体 | `&lv_font_app_16` |

---

## 调色板

LVGL 内置 Material Design 色板，16 种颜色 × 多级亮度：

```c
// 主色
lv_palette_main(LV_PALETTE_RED)
lv_palette_main(LV_PALETTE_BLUE)
lv_palette_main(LV_PALETTE_GREEN)
lv_palette_main(LV_PALETTE_ORANGE)
// ... 完整列表见 lvgl-v9-api-cheatsheet.md §7

// 变体（亮度 1-5）
lv_palette_lighten(LV_PALETTE_BLUE, 3)  // 浅蓝
lv_palette_darken(LV_PALETTE_BLUE, 2)   // 深蓝
```

---

## 主题 vs 内联样式决策

```
这个样式是全局统一风格吗？
├── 是 → 用主题
│   例如：所有按钮用蓝色、所有卡片圆角 8px、默认字体 16px
│
└── 否 → 用内联样式
    例如：某个警告按钮用红色、标题用 24px 字体、特定卡片有阴影
```

### 主题覆盖（推荐）

```c
// ✅ 全局风格用主题，单个差异用内联覆盖
lv_theme_default_init(disp,
    lv_palette_main(LV_PALETTE_BLUE),
    lv_palette_main(LV_PALETTE_GREY),
    true, &lv_font_app_16);  /* 注意：直接传 true，不是 LV_THEME_DEFAULT_DARK */

// 只有警告按钮单独覆盖颜色
lv_obj_set_style_bg_color(warn_btn, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);

// 页面标题单独加大字号
lv_obj_set_style_text_font(title, &lv_font_app_24, 0);
```

### 全手写样式（不推荐，除非主题不可用）

```c
// ❌ 避免：每个控件手写样式，代码量 3×+
lv_obj_set_style_bg_color(btn1, lv_color_hex(0x2196F3), LV_PART_MAIN);
lv_obj_set_style_radius(btn1, 8, LV_PART_MAIN);
lv_obj_set_style_text_font(btn1, &lv_font_app_16, LV_PART_MAIN);
lv_obj_set_style_bg_color(btn2, lv_color_hex(0x2196F3), LV_PART_MAIN);
// ... 重复 N 次
```

---

## 深色/浅色主题切换

```c
static lv_theme_t * current_theme = NULL;

void switch_theme(bool dark) {
    if (current_theme) {
        lv_theme_delete(current_theme);
    }
    current_theme = lv_theme_default_init(lv_display_get_default(),
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_GREY),
        dark ? LV_THEME_DEFAULT_DARK : LV_THEME_DEFAULT_LIGHT,
        &lv_font_app_16);
}
```

---

## 主题系统不支持时

如果项目 `lv_conf.h` 未启用主题（`LV_USE_THEME_DEFAULT = 0`），则使用**复用样式对象**模式：

```c
// 创建共享样式，多次应用到不同控件
static lv_style_t style_card;
lv_style_init(&style_card);
lv_style_set_bg_color(&style_card, lv_color_hex(0xFFFFFF));
lv_style_set_radius(&style_card, 8);

for (int i = 0; i < 6; i++) {
    lv_obj_t * card = lv_obj_create(container);
    lv_obj_add_style(card, &style_card, LV_PART_MAIN | LV_STATE_DEFAULT);
}
```

---

## 主题对滚动条的影响

`lv_theme_default_init` 会自动给容器添加默认的滚动条样式。如果布局中某容器已通过主题获得了滚动条样式，仍需显式关闭：

```c
lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
```

主题的样式优先级低于直接 `lv_obj_add_style` / `lv_obj_set_style_*`，所以显式设置会覆盖主题默认值。

---

## ⚠️ 常见陷阱

### 1. `LV_THEME_DEFAULT_DARK` 是配置宏，不是 true

`LV_THEME_DEFAULT_DARK` 在 `lv_conf.h` 中默认定义为 `0`（false=浅色），它**不是**一个表示深色模式的枚举值。直接传它到 `dark` 参数会导致浅色主题：

```c
// ❌ 错误：LV_THEME_DEFAULT_DARK = 0 → 浅色主题！
lv_theme_default_init(disp, ..., LV_THEME_DEFAULT_DARK, &my_font);

// ✅ 正确：直接传布尔值
lv_theme_default_init(disp, ..., true, &my_font);   // 深色
lv_theme_default_init(disp, ..., false, &my_font);  // 浅色
```

### 2. 自定义字体导致控件内部符号缺失

把主题字体设为自定义位图字体时，LVGL 控件内部渲染可能依赖的符号不在自定义字体内 → 方框：

| 控件 | 依赖符号 | 码位 | 现象 |
|------|---------|------|------|
| Checkbox | `LV_SYMBOL_OK` | `U+F00C` | 勾选后不显示勾 |
| Spinner | `LV_SYMBOL_PLUS/MINUS` | `U+F067/F068` | 加减按钮空白 |
| Dropdown | `LV_SYMBOL_DOWN` | `U+F078` | 箭头消失（但 FA6 通常已包含） |

**修复**：生成自定义字体时，追加这些符号码位。见 `references/font-pipeline.md`。
