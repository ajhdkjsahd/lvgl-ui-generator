# 组件复用模式

LVGL 没有像 React/Vue 那样的组件系统，但通过工厂函数 + 上下文结构体可以构建高度可复用的 UI 组件。

---

## 三种复用层级

| 层级 | 方式 | 复用粒度 | 示例 |
|------|------|----------|------|
| L1 基础工厂 | 纯函数封装 | 单个控件创建 | `create_label(parent, txt, font, color)` |
| L2 复合组件 | 工厂函数 + ctx | 容器+子控件组合 | `create_sensor_card(parent, icon, title)` |
| L3 自包含 Widget | ctx + 生命周期 | 有内部状态和事件 | `create_motor_panel(parent, motor_id)` |

---

## L1: 基础工厂函数

减少重复代码的最简单方式——把常用控件创建封装成参数化函数：

```c
lv_obj_t * create_label(lv_obj_t * parent, const char * txt,
                         const lv_font_t * font, lv_color_t color)
{
    lv_obj_t * lbl = lv_label_create(parent);
    if (font) lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    lv_label_set_text(lbl, txt);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    return lbl;
}

lv_obj_t * create_card(lv_obj_t * parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_pad_all(card, 12, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}
```

**适用**：同类型控件在页面上出现 ≥3 次。

---

## L2: 复合组件（带上下文）

当一组控件总是**一起出现**时，用一个函数创建它们并返回**上下文结构体**来管理引用。

### 模式模板

```c
/* 1. 定义上下文结构体：保存需要外部访问的控件引用 */
typedef struct {
    lv_obj_t * container;    /* 组件根容器 */
    lv_obj_t * icon_label;   /* 需要动态更新的子控件 */
    lv_obj_t * value_label;
    lv_obj_t * unit_label;
} sensor_card_t;

/* 2. 创建函数：返回上下文 */
sensor_card_t create_sensor_card(lv_obj_t * parent,
                                  const char * icon_text,
                                  const char * title,
                                  const char * unit)
{
    sensor_card_t sc;

    /* 根容器 */
    sc.container = lv_obj_create(parent);
    lv_obj_set_size(sc.container, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(sc.container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sc.container, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(sc.container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(sc.container, LV_OBJ_FLAG_SCROLLABLE);

    /* 图标 + 标题 */
    sc.icon_label = lv_label_create(sc.container);
    lv_obj_set_style_text_font(sc.icon_label, &lv_font_demo_16, 0);
    lv_label_set_text(sc.icon_label, title);  /* FA6合并字体: 单label */

    /* 数值 */
    sc.value_label = lv_label_create(sc.container);
    lv_obj_set_style_text_font(sc.value_label, &lv_font_demo_24, 0);
    lv_label_set_text(sc.value_label, "--");

    /* 单位 */
    sc.unit_label = lv_label_create(sc.container);
    lv_obj_set_style_text_font(sc.unit_label, &lv_font_demo_16, 0);
    lv_label_set_text(sc.unit_label, unit);

    return sc;  /* 按值返回，struct 只有 4 个指针，开销极小 */
}

/* 3. 更新函数：通过上下文更新动态数据 */
void sensor_card_update(sensor_card_t * sc, const char * value) {
    lv_label_set_text(sc->value_label, value);
}
```

### 使用方式

```c
sensor_card_t cards[4];
cards[0] = create_sensor_card(grid, "🌡 温度", "度");
cards[1] = create_sensor_card(grid, "💧 湿度", "%");
// ...
sensor_card_update(&cards[0], "26.5");
sensor_card_update(&cards[1], "58");
```

**适用**：仪表盘卡片、列表项、开关行——即控件组 ≥2 次出现且有动态数据。

---

## L3: 自包含 Widget（带生命周期）

当组件有**内部交互逻辑**或**需要管理自身内存**时，需要完整的 create / update / destroy 生命周期。

### 切换行组件示例

```c
typedef struct {
    lv_obj_t * container;
    lv_obj_t * label;
    lv_obj_t * sw;
    void (*on_changed)(bool state, void * user_data);
    void * user_data;
} toggle_row_t;

/* create: 分配 ctx，注册事件 */
toggle_row_t * toggle_row_create(lv_obj_t * parent,
                                  const char * text,
                                  bool initial_state,
                                  void (*on_changed)(bool, void *),
                                  void * user_data)
{
    toggle_row_t * tr = lv_malloc(sizeof(toggle_row_t));

    tr->container = lv_obj_create(parent);
    lv_obj_set_size(tr->container, lv_pct(100), lv_dpx(48));
    lv_obj_set_flex_flow(tr->container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tr->container, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(tr->container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(tr->container, LV_OBJ_FLAG_SCROLLABLE);

    tr->label = lv_label_create(tr->container);
    lv_label_set_text(tr->label, text);

    tr->sw = lv_switch_create(tr->container);
    if (initial_state) lv_obj_add_state(tr->sw, LV_STATE_CHECKED);

    tr->on_changed = on_changed;
    tr->user_data = user_data;

    /* 用 ctx 作为 user_data，事件回调中取回 */
    lv_obj_add_event_cb(tr->sw, on_toggle, LV_EVENT_VALUE_CHANGED, tr);

    /* 容器删除时释放 ctx */
    lv_obj_add_event_cb(tr->container, on_toggle_row_delete,
                        LV_EVENT_DELETE, tr);

    return tr;
}

/* 事件回调：从 user_data 取回 ctx */
static void on_toggle(lv_event_t * e) {
    toggle_row_t * tr = lv_event_get_user_data(e);
    bool st = lv_obj_has_state(tr->sw, LV_STATE_CHECKED);
    if (tr->on_changed) tr->on_changed(st, tr->user_data);
}

/* 析构 */
static void on_toggle_row_delete(lv_event_t * e) {
    toggle_row_t * tr = lv_event_get_user_data(e);
    lv_free(tr);
}
```

### 使用方式

```c
void on_wifi_changed(bool on, void * data) {
    LV_LOG_USER("WiFi: %s", on ? "ON" : "OFF");
}

toggle_row_t * wifi_row = toggle_row_create(panel, "WiFi",
    false, on_wifi_changed, NULL);
```

**适用**：需要在多处复用的交互组件（设置项、列表项、开关行）。有内部状态和回调时用这一层。

---

## 快速决策

```
这段 UI 会在页面上重复 ≥3 次？
├── 只是静态布局 → L1 工厂函数
├── 有动态数据需要外部更新 → L2 复合组件 (ctx 结构体)
└── 有内部交互逻辑/需管理内存 → L3 自包含 Widget (malloc + 生命周期)
```
