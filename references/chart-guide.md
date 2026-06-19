# lv_chart 数据图表模板

完整的 LVGL v9 折线图页面模板：多图表滚动 + 坐标轴单位 + 点击查看数值 + 自动消失。

---

## 适用场景

数据监控面板、传感器趋势图、历史数据查看等需要多张图表的页面。

---

## 核心模式

### 1. 可滚动面板 + 多图表垂直堆叠

```c
lv_obj_t * panel = lv_obj_create(parent);
lv_obj_set_size(panel, lv_pct(100), lv_pct(100));
lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_row(panel, lv_dpx(14), 0);
lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_ON);
// 注意：不要 lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE)

for (int i = 0; i < N; i++) {
    create_chart_section(panel, &params[i], &charts[i], i);
}
```

**关键**: 不要禁用 SCROLLABLE，内容超出时自动滚动。

### 2. 图表区域结构

每个图表 section 的层级：

```
[容器 cont] (flex column, LV_SIZE_CONTENT, transparent)
  ├─ [标题 label]
  ├─ [图表行 row] (flex row, CHART_HEIGHT)
  │   ├─ [Y轴刻度列] (flex column, space-between)
  │   │   ├─ 最大值 label
  │   │   ├─ 中值 label
  │   │   └─ 最小值 label
  │   └─ [lv_chart] (flex_grow:1)
  └─ [X轴时间标签行] (flex row, space-between, pad_left 对齐图表)
      ├─ "0h" "4h" "8h" "12h" "16h" "20h"
```

### 3. 小数支持（lv_chart 仅支持 int32_t）

```c
typedef struct {
    int32_t y_min, y_max;  // 存储值范围
    int32_t scale;         // 1=整数, 10=1位小数, 100=2位小数
} param_def_t;

// 显示时换算：实际值 = 存储值 / scale
static void format_value(char * buf, size_t len, int32_t stored,
                          int32_t scale, const char * unit) {
    if (scale <= 1)
        snprintf(buf, len, "%d %s", (int)stored, unit);
    else if (scale == 10)
        snprintf(buf, len, "%d.%01d %s", (int)(stored/10), (int)(abs(stored%10)), unit);
    else
        snprintf(buf, len, "%d.%02d %s", (int)(stored/100), (int)(abs(stored%100)), unit);
}
```

**示例**: pH 7.3 → 存 73, scale=10；氨氮 0.15 → 存 15, scale=100。

### 4. Y 轴刻度（手动标签）

LVGL v9 的 `lv_chart_set_axis_tick` 不一定存在，**手动创建 Y 轴标签更可靠**：

```c
// Y 轴刻度列 — 上/中/下三个值
lv_obj_t * y_col = lv_obj_create(chart_row);
lv_obj_set_flex_flow(y_col, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(y_col, LV_FLEX_ALIGN_SPACE_BETWEEN, ...);

char buf[16];
format_value(buf, sizeof(buf), y_max, scale, "");  // 上
make_small_label(y_col, buf);
format_value(buf, sizeof(buf), (y_min+y_max)/2, scale, "");  // 中
make_small_label(y_col, buf);
format_value(buf, sizeof(buf), y_min, scale, "");  // 下
make_small_label(y_col, buf);
```

Y 轴标签用 `LV_OPA_60` 降低视觉权重。

### 5. X 轴时间标签

```c
lv_obj_t * x_row = lv_obj_create(cont);
lv_obj_set_flex_flow(x_row, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(x_row, LV_FLEX_ALIGN_SPACE_BETWEEN, ...);
lv_obj_set_style_pad_left(x_row, lv_dpx(54), 0);  // 对齐图表左侧

const char * labels[] = {"0h","4h","8h","12h","16h","20h"};
for (int i = 0; i < 6; i++) make_small_label(x_row, labels[i]);
```

### 6. 点击查看数值

#### 为什么用 `LV_EVENT_PRESSED` 而不是 `LV_EVENT_CLICKED`

```c
// ❌ LV_EVENT_CLICKED — lv_chart_get_pressed_point() 已被清空
//    原因: chart 在 LV_EVENT_RELEASED 时把 pressed_point_id 重置为 NONE

// ✅ LV_EVENT_PRESSED — 坐标信息可用
lv_obj_add_event_cb(chart, on_chart_click, LV_EVENT_PRESSED, (void*)(uintptr_t)idx);
```

#### 为什么不用 `lv_chart_get_pressed_point`

```c
// ❌ lv_chart_get_pressed_point() — user callback 在 chart class handler 之前执行
//    此时 pressed_point_id 还是上一帧的值

// ✅ 直接从坐标计算
lv_area_t area;
lv_obj_get_content_coords(chart, &area);
lv_point_t p;
lv_indev_get_point(lv_indev_active(), &p);

uint32_t point_id = (uint32_t)((p.x - area.x1) * POINT_COUNT / lv_area_get_width(&area));
```

#### Y 坐标精确定位

```c
// ❌ lv_chart_get_point_pos_by_id 的 Y 可能偏 5px+
// ✅ 根据数据值线性插值
int32_t ch    = lv_area_get_height(&area);
int32_t range = y_max - y_min;
int32_t sy    = area.y1 + (int32_t)((int64_t)(y_max - stored) * ch / range);
```

### 7. 高亮圆点（非跨图游标）

```c
// ❌ lv_chart_cursor_t — 画一整条竖线跨整个图表
// ✅ 独立 lv_obj 小圆点，精确定位在数据点上

// 用独立 lv_style_t 防止主题覆盖颜色：
static lv_style_t g_dot_style;
lv_style_init(&g_dot_style);
lv_style_set_bg_opa(&g_dot_style, LV_OPA_COVER);
lv_style_set_border_width(&g_dot_style, lv_dpx(2));
lv_style_set_border_color(&g_dot_style, lv_color_white());
lv_style_set_radius(&g_dot_style, lv_dpx(5));

g_dot = lv_obj_create(screen);  // 屏幕层，浮在所有内容之上
lv_obj_set_size(g_dot, lv_dpx(10), lv_dpx(10));
lv_obj_add_style(g_dot, &g_dot_style, 0);
lv_obj_add_flag(g_dot, LV_OBJ_FLAG_HIDDEN);

// 点击时换色 + 定位：
lv_style_set_bg_color(&g_dot_style, series_color);
lv_obj_refresh_style(g_dot, LV_PART_MAIN, LV_STYLE_PROP_ANY);
lv_obj_set_pos(g_dot, sx - 5, sy - 5);
lv_obj_remove_flag(g_dot, LV_OBJ_FLAG_HIDDEN);
```

**高亮圆点必须在屏幕层**（`lv_obj_create(screen)`），否则会被图表裁剪。

### 8. 自动消失（三种路径）

```c
// ① 点击弹窗自身 → 消失
lv_obj_add_flag(g_popup, LV_OBJ_FLAG_CLICKABLE);
lv_obj_add_event_cb(g_popup, on_popup_hide, LV_EVENT_CLICKED, NULL);

// ② 滚动页面 → 消失
lv_obj_add_event_cb(panel, on_panel_scroll_begin, LV_EVENT_SCROLL_BEGIN, NULL);

// ③ 切换 Tab → 消失 (TabView VALUE_CHANGED)
lv_obj_add_event_cb(tabview, on_tab_changed, LV_EVENT_VALUE_CHANGED, NULL);

static void hide_popup_and_dot(void) {
    lv_obj_add_flag(g_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_dot, LV_OBJ_FLAG_HIDDEN);
}
```

**注意**: LVGL v9 没有 `LV_EVENT_HIDDEN`，Tab 切换需通过 TabView 的 `LV_EVENT_VALUE_CHANGED` 感知。

---

## 完整代码模板

```c
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>

#define POINT_COUNT  20
#define CHART_HEIGHT lv_dpx(150)

// ----- 参数定义 -----
typedef struct {
    const char * title;      // 图表标题
    const char * y_unit;     // Y 轴单位
    int32_t      y_min;      // Y 轴下限 (存储值)
    int32_t      y_max;      // Y 轴上限 (存储值)
    int32_t      scale;      // 10=1位小数, 100=2位小数, 1=整数
    lv_color_t   color;      // 折线颜色
} chart_param_t;

typedef struct {
    lv_obj_t          * chart;
    lv_chart_series_t * ser;
    int32_t             values[POINT_COUNT];
    int32_t             y_min, y_max, scale;
    const char         * unit;
} chart_data_t;

// ----- 全局 -----
static chart_data_t g_charts[N];
static lv_obj_t    * g_popup = NULL;
static lv_obj_t    * g_dot   = NULL;
static lv_style_t    g_dot_style;
static lv_color_t    g_colors[N];  // 与 params 数组对应

// ----- 创建页面 -----
lv_obj_t * chart_page_create(lv_obj_t * parent) {
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_set_size(panel, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(panel, lv_dpx(14), 0);

    lv_obj_t * screen = lv_obj_get_screen(panel);

    // 弹窗 + 圆点 (屏幕层)
    g_popup = lv_label_create(screen);
    lv_obj_add_flag(g_popup, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_CLICKABLE);

    lv_style_init(&g_dot_style);
    lv_style_set_bg_opa(&g_dot_style, LV_OPA_COVER);
    lv_style_set_radius(&g_dot_style, lv_dpx(5));
    g_dot = lv_obj_create(screen);
    lv_obj_set_size(g_dot, lv_dpx(10), lv_dpx(10));
    lv_obj_add_style(g_dot, &g_dot_style, 0);
    lv_obj_add_flag(g_dot, LV_OBJ_FLAG_HIDDEN);

    // 滚动消失
    lv_obj_add_event_cb(panel, on_scroll, LV_EVENT_SCROLL_BEGIN, NULL);

    return panel;
}
```

---

## 常见坑

| 现象 | 根因 | 修复 |
|------|------|------|
| 点击无反应 | 用了 `LV_EVENT_CLICKED`，`pressed_point_id` 已被 chart 清空 | 改用 `LV_EVENT_PRESSED` |
| `lv_chart_get_pressed_point` 返回旧值 | user callback 比 chart class handler 先执行 | 直接用坐标手动算 |
| 高亮圆点颜色不随图表 | 主题样式覆盖了 `lv_obj_set_style_bg_color` | 用独立 `lv_style_t` + `lv_obj_refresh_style` |
| 高亮圆点 Y 偏 ~5px | `lv_chart_get_point_pos_by_id` 坐标不精确 | Y 坐标用数据值线性插值计算 |
| `lv_chart_set_axis_tick` 编译失败 | 该版本 LVGL 无此 API | 手动创建 Y 轴刻度 label |
| 高亮竖线跨全图 | 用了 `lv_chart_cursor_t` | 改用独立 `lv_obj` 小圆点 |
| 切换 Tab 弹窗不消失 | LVGL v9 无 `LV_EVENT_HIDDEN` | 监听 TabView `LV_EVENT_VALUE_CHANGED` |
| `lv_color_t` 设置后颜色不对 | 32-bit 色深可能含 alpha，struct 初始化未设 | 显式设置 `.red .green .blue` 或用 `lv_color_hex()` |
| 图表无水平网格线 | 未调用 `lv_chart_set_div_line_count` | `lv_chart_set_div_line_count(chart, 4, 5)` |
