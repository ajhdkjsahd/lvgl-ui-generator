# 多分辨率适配指南

## 核心理念

LVGL 的布局系统天然支持响应式。**优先用相对布局，绝对像素仅用于不可变元素**（如最小触摸尺寸 48px）。

---

## 1. 适配工具箱

### `lv_dpx(n)` — DPI 感知尺寸

将"设计稿像素"按实际 DPI 缩放。`LV_DPI_DEF` 在 `lv_conf.h` 中定义（默认 130）。

```c
// 设计稿基准 DPI = 130 时，200px 按钮
// 在 260 DPI 屏幕上自动变成 400px
lv_obj_set_width(btn, lv_dpx(200));
lv_obj_set_height(btn, lv_dpx(48));
```

**使用规则**：所有用户交互元素的尺寸用 `lv_dpx()`，布局间距可以用固定像素。

### `lv_pct(x)` — 百分比尺寸

相对父容器内容区的百分比。

```c
lv_obj_set_width(panel, lv_pct(50));   // 父容器宽度的一半
lv_obj_set_height(header, lv_pct(10)); // 父容器高度的 10%
```

### `LV_SIZE_CONTENT` — 适应内容

```c
lv_obj_set_width(label, LV_SIZE_CONTENT);  // 文字多宽就多宽
lv_obj_set_height(btn, LV_SIZE_CONTENT);   // 内容+padding 决定高度
```

### Flex grow — 弹性比例分配

```c
lv_obj_set_flex_grow(left_panel, 1);   // 占 1 份
lv_obj_set_flex_grow(right_panel, 2);  // 占 2 份 → 1:2 比例
```

### 尺寸约束

```c
lv_obj_set_style_min_width(btn, lv_dpx(80), 0);   // 不小于 80dp
lv_obj_set_style_max_width(label, lv_pct(80), 0);  // 不超过父容器 80%
```

---

## 2. 分辨率适配策略

### 策略 A：Flex/Grid 弹性布局（首选）

用 Flex/Grid 让容器自动填满空间，元素按比例分配——**适配所有分辨率**。

```c
/* 全屏容器：flex 列布局 */
lv_obj_t * root = lv_obj_create(screen);
lv_obj_set_size(root, lv_pct(100), lv_pct(100));  // 占满屏幕
lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);

/* 头部：固定高度，宽度自适应 */
lv_obj_t * header = lv_obj_create(root);
lv_obj_set_size(header, lv_pct(100), lv_dpx(48));
lv_obj_set_flex_grow(header, 0);  // 不伸缩

/* 内容区：自动填充剩余空间 */
lv_obj_t * content = lv_obj_create(root);
lv_obj_set_size(content, lv_pct(100), LV_SIZE_CONTENT);
lv_obj_set_flex_grow(content, 1);  // 占据所有剩余空间

/* 底部栏：固定高度 */
lv_obj_t * footer = lv_obj_create(root);
lv_obj_set_size(footer, lv_pct(100), lv_dpx(48));
lv_obj_set_flex_grow(footer, 0);
```

### 策略 B：Grid FR 均分（卡片布局首选）

Grid 的 `LV_GRID_FR(n)` 自动按比例分配，换分辨率不用改任何数值：

```c
/* 2×2 仪表盘：任何分辨率都自动均分 */
static int32_t col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
static int32_t row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
/* 800×480 → 每格 400×240
   320×240 → 每格 160×120  ← 无需改代码 */
```

### 策略 C：分辨率分支（迫不得已时）

当布局结构在不同分辨率下需要**完全不同**时（如横屏 vs 竖屏）：

```c
lv_coord_t w = lv_display_get_horizontal_resolution(NULL);
lv_coord_t h = lv_display_get_vertical_resolution(NULL);

if (w >= 800) {
    create_desktop_layout(screen);   // 大屏：2×2 grid + 侧边栏
} else if (w >= 480) {
    create_tablet_layout(screen);    // 中屏：2×1 grid
} else {
    create_phone_layout(screen);     // 小屏：单列列表
}
```

---

## 3. 字体随屏缩放

```c
/* 基准：800×480 设计稿，16px/24px */
/* 小屏 (320×240)：自动缩到 10px/16px */
/* 大屏 (1024×600)：自动扩到 ~18px/28px */

#define FONT_SCALE(w)  ((w) / 800.0f)

lv_coord_t sw = lv_display_get_horizontal_resolution(NULL);
int body_font  = (int)(16 * FONT_SCALE(sw) + 0.5f);
int title_font = (int)(24 * FONT_SCALE(sw) + 0.5f);

/* 用 Tiny TTF 生成对应字号的字体 */
lv_font_t * f_body  = lv_tiny_ttf_create_file("/root/font.otf", body_font);
lv_font_t * f_title = lv_tiny_ttf_create_file("/root/font.otf", title_font);
```

> `lv_font_conv` 生成的位图字体是固定字号的，多分辨率适配时更推荐 Tiny TTF 方案。

---

## 4. 常见分辨率速算表

| 分辨率 | 宽高比 | 典型 DPI | 卡片数 | 推荐字号 | 适用设备 |
|--------|--------|----------|--------|----------|----------|
| 320×240 | 4:3 | ~100 | 1×1 | 12/18 | 小 TFT |
| 480×272 | 16:9 | ~120 | 2×1 | 14/20 | 基础 LCD |
| 480×320 | 3:2 | ~130 | 2×1 | 14/22 | 手机横屏 |
| 800×480 | 5:3 | ~160 | 2×2 | 16/24 | 工控屏 |
| 1024×600 | 16:10 | ~180 | 2×3 | 18/28 | 平板 |

---

## 5. 实战示例：气象站适配 320×240

从 800×480 的 2×2 Grid 仪表盘，适配到 320×240 小屏：

```c
lv_obj_t * weather_page_create(void)
{
    lv_coord_t sw = lv_display_get_horizontal_resolution(NULL);

    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_set_size(screen, lv_pct(100), lv_pct(100));

    /* 标题栏：百分比宽度 + dpx 高度 */
    lv_obj_t * title_bar = lv_obj_create(screen);
    lv_obj_set_size(title_bar, lv_pct(100), lv_dpx(36));  // DPI 感知高度
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);

    /* Grid：FR 自动均分 */
    lv_obj_t * grid = lv_obj_create(screen);
    lv_obj_set_size(grid, lv_pct(100), lv_pct(100));
    lv_obj_set_style_pad_all(grid, lv_dpx(8), 0);
    lv_obj_set_style_pad_row(grid, lv_dpx(8), 0);
    lv_obj_set_style_pad_column(grid, lv_dpx(8), 0);

    static int32_t col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static int32_t row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

    /* 卡片内字号按屏幕宽度缩放 */
    int font_s = sw >= 800 ? 24 : 16;  // 大屏用 24px，小屏用 16px
    int font_xs = sw >= 800 ? 16 : 12;

    /* ... 用 font_s / font_xs 创建卡片 ... */
}
```

关键点：
- 宽度用 `lv_pct(100)` — 自动填满
- 间距用 `lv_dpx(8)` — DPI 感知
- Grid `LV_GRID_FR(1)` — 自适应均分
- 字号按 `sw` 条件选择 — 小屏不挤

---

## 6. 快速决策

```
这个尺寸需要跨分辨率适配吗？
├── 否 → 固定像素 (lv_obj_set_size(obj, 200, 48))
├── 是，但同一 DPI 等级 → lv_dpx(200)
├── 是，需要随容器缩放 → lv_pct(50)
├── 是，需要按比例分配 → Flex grow / Grid FR
└── 是，布局结构都不同 → 分辨率分支
```
