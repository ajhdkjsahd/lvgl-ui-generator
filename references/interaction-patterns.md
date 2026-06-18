# 交互模式模板

## snprintf 动态文本

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

---

## 事件回调模式

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

### 常用事件码

| 事件码 | 适用控件 | 触发时机 |
|--------|---------|---------|
| `LV_EVENT_CLICKED` | button, 可点击容器 | 按下+释放 |
| `LV_EVENT_VALUE_CHANGED` | switch, slider, dropdown | 值改变 |
| `LV_EVENT_PRESSED` | 任意控件 | 按下瞬间 |
| `LV_EVENT_LONG_PRESSED` | 任意控件 | 长按 |
| `LV_EVENT_DELETE` | 任意控件 | 控件被删除 |
| `LV_EVENT_SCREEN_LOADED` | screen | 屏幕加载完成 |

### user_data 传参规范

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

## 页面级状态管理

推荐为每个页面创建上下文结构体，在 `LV_EVENT_DELETE` 时释放：

```c
typedef struct {
    lv_obj_t * screen;
    lv_obj_t * title_label;
    lv_obj_t * value_label;
    lv_obj_t ** cards;
    int card_count;
} page_ctx_t;

static void on_screen_delete(lv_event_t * e) {
    page_ctx_t * ctx = lv_event_get_user_data(e);
    lv_free(ctx->cards);
    lv_free(ctx);
}

lv_obj_t * my_page_create(void) {
    page_ctx_t * ctx = lv_malloc(sizeof(page_ctx_t));
    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_add_event_cb(screen, on_screen_delete, LV_EVENT_DELETE, ctx);
    ctx->screen = screen;
    // ...
    return screen;
}
```

---

## 组件工厂函数模式

当页面有多个同类型组件（如仪表盘卡片），用工厂函数 + 上下文结构体封装创建和更新逻辑：

```c
/* 上下文结构体：保存组件的关键控件引用 */
typedef struct {
    lv_obj_t * card;         // 容器本身（用于 grid_cell 定位）
    lv_obj_t * value_label;  // 动态更新的控件
    lv_obj_t * unit_label;
} card_ctx_t;

/* 工厂函数：创建并返回上下文 */
static card_ctx_t create_card(lv_obj_t * parent, const char * icon_sym,
                               const char * title, const char * unit) {
    card_ctx_t ctx;
    ctx.card = lv_obj_create(parent);
    // ... 构建 card 内部布局 ...
    ctx.value_label = lv_label_create(ctx.card);
    ctx.unit_label  = lv_label_create(ctx.card);
    return ctx;  // 按值返回，结构体小（3 个指针）
}

/* 更新函数：通过上下文更新动态数据 */
static void update_card(card_ctx_t * card, const char * val_buf) {
    lv_label_set_text(card->value_label, val_buf);
}

/* 使用 */
card_ctx_t cards[4];
cards[0] = create_card(grid, LV_SYMBOL_CHARGE, "温度", "度");
// ...
update_card(&cards[0], "26.5");
```

**关键点**：ctx 中必须保留 container 引用（用于 `lv_obj_set_grid_cell` 等父级操作），不能只保留内部 label。

---

## 交互反馈

```c
// Toast 提示（临时弹出后自动消失）
static void show_toast(const char * msg) {
    lv_obj_t * toast = lv_label_create(lv_layer_top());
    lv_label_set_text(toast, msg);
    lv_obj_set_style_bg_color(toast, lv_color_hex(0x333333), 0);
    lv_obj_set_style_text_color(toast, lv_color_white(), 0);
    lv_obj_set_style_radius(toast, 8, 0);
    lv_obj_set_style_pad_all(toast, 16, 0);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_delete(toast, 2000);  // 2 秒后自动删除
}

// 确认对话框
static void show_confirm_dialog(const char * title, const char * msg) {
    lv_obj_t * mb = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mb, title);
    lv_msgbox_add_text(mb, msg);
    lv_msgbox_add_footer_button(mb, "取消");
    lv_obj_t * ok_btn = lv_msgbox_add_footer_button(mb, "确定");
    lv_obj_add_event_cb(ok_btn, on_dialog_ok, LV_EVENT_CLICKED, mb);
}
```
