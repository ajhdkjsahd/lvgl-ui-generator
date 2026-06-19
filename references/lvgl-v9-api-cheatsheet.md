# LVGL v9 API 速查表

> 从 `lvgl/include/lvgl/` 提取验证的 API。v9 签名与 v8 不同，务必使用此表。

---

## 1. 控件创建函数

### 基础控件

```c
// 屏幕 (screen)
lv_obj_t * lv_obj_create(lv_obj_t * parent);  // parent=NULL 创建屏幕
lv_obj_t * lv_screen_active(void);             // 获取当前活动屏幕
void lv_screen_load(lv_obj_t * screen);        // 切换屏幕（无动画）
void lv_screen_load_anim(lv_obj_t * screen, lv_screen_load_anim_t anim_type,
                         uint32_t duration, uint32_t delay);
// anim_type: LV_SCREEN_LOAD_ANIM_NONE / FADE_IN / FADE_OUT / OVER_LEFT/RIGHT/TOP/BOTTOM / MOVE_LEFT/RIGHT/TOP/BOTTOM / OUT_LEFT/RIGHT/TOP/BOTTOM

// 按钮
lv_obj_t * lv_button_create(lv_obj_t * parent);
// 注意: v9 是 lv_button_create，不是 v8 的 lv_btn_create

// 标签
lv_obj_t * lv_label_create(lv_obj_t * parent);
void lv_label_set_text(lv_obj_t * obj, const char * text);
void lv_label_set_text_fmt(lv_obj_t * obj, const char * fmt, ...);
void lv_label_set_text_static(lv_obj_t * obj, const char * text);
void lv_label_set_long_mode(lv_obj_t * obj, lv_label_long_mode_t long_mode);
// long_mode: LV_LABEL_LONG_WRAP / DOTS / SCROLL / SCROLL_CIRCULAR / CLIP
void lv_label_set_max_lines(lv_obj_t * obj, int32_t lines);
void lv_label_set_recolor(lv_obj_t * obj, bool en);  // 允许 #FF0000 颜色标记
const char * lv_label_get_text(const lv_obj_t * obj);

// 图片
lv_obj_t * lv_image_create(lv_obj_t * parent);
void lv_image_set_src(lv_obj_t * obj, const void * src);
void lv_image_set_rotation(lv_obj_t * obj, int32_t angle);     // 0.1度单位
void lv_image_set_scale(lv_obj_t * obj, uint32_t zoom);        // 256=100%
void lv_image_set_antialias(lv_obj_t * obj, bool en);
void lv_image_set_inner_align(lv_obj_t * obj, lv_image_align_t align);
// align: LV_IMAGE_ALIGN_DEFAULT / STRETCH / TILE / CONTAIN / COVER

// 开关
lv_obj_t * lv_switch_create(lv_obj_t * parent);
void lv_switch_set_orientation(lv_obj_t * obj, lv_switch_orientation_t orientation);
// orientation: LV_SWITCH_ORIENTATION_AUTO / HORIZONTAL / VERTICAL
// 状态通过 lv_obj_add_state(obj, LV_STATE_CHECKED) / lv_obj_remove_state 控制

// 复选框
lv_obj_t * lv_checkbox_create(lv_obj_t * parent);
void lv_checkbox_set_text(lv_obj_t * obj, const char * text);
void lv_checkbox_set_text_static(lv_obj_t * obj, const char * text);
const char * lv_checkbox_get_text(const lv_obj_t * obj);

// 滑块
lv_obj_t * lv_slider_create(lv_obj_t * parent);
void lv_slider_set_range(lv_obj_t * obj, int32_t min, int32_t max);
void lv_slider_set_value(lv_obj_t * obj, int32_t value, lv_anim_enable_t anim);
int32_t lv_slider_get_value(const lv_obj_t * obj);
bool lv_slider_is_dragged(const lv_obj_t * obj);

// 进度条
lv_obj_t * lv_bar_create(lv_obj_t * parent);
void lv_bar_set_range(lv_obj_t * obj, int32_t min, int32_t max);
void lv_bar_set_value(lv_obj_t * obj, int32_t value, lv_anim_enable_t anim);
void lv_bar_set_mode(lv_obj_t * obj, lv_bar_mode_t mode);
// mode: LV_BAR_MODE_NORMAL / SYMMETRICAL / RANGE
int32_t lv_bar_get_value(const lv_obj_t * obj);

// 圆弧
lv_obj_t * lv_arc_create(lv_obj_t * parent);
void lv_arc_set_range(lv_obj_t * obj, int32_t min, int32_t max);
void lv_arc_set_value(lv_obj_t * obj, int32_t value);
void lv_arc_set_angles(lv_obj_t * obj, uint32_t start, uint32_t end);
void lv_arc_set_rotation(lv_obj_t * obj, uint32_t rotation);
int32_t lv_arc_get_value(const lv_obj_t * obj);

// 下拉
lv_obj_t * lv_dropdown_create(lv_obj_t * parent);
void lv_dropdown_set_options(lv_obj_t * obj, const char * options);  // 换行分隔
void lv_dropdown_set_selected(lv_obj_t * obj, uint32_t selected);
uint32_t lv_dropdown_get_selected(const lv_obj_t * obj);
void lv_dropdown_open(lv_obj_t * obj);
void lv_dropdown_close(lv_obj_t * obj);

// 滚轮
lv_obj_t * lv_roller_create(lv_obj_t * parent);
void lv_roller_set_options(lv_obj_t * obj, const char * options, lv_roller_mode_t mode);
void lv_roller_set_selected(lv_obj_t * obj, uint32_t selected, lv_anim_enable_t anim);
void lv_roller_set_visible_row_count(lv_obj_t * obj, uint8_t row_count);

// 文本输入
lv_obj_t * lv_textarea_create(lv_obj_t * parent);
void lv_textarea_set_text(lv_obj_t * obj, const char * text);
void lv_textarea_set_placeholder_text(lv_obj_t * obj, const char * text);
void lv_textarea_set_one_line(lv_obj_t * obj, bool en);
void lv_textarea_set_password_mode(lv_obj_t * obj, bool en);
void lv_textarea_set_max_length(lv_obj_t * obj, uint32_t num);
const char * lv_textarea_get_text(const lv_obj_t * obj);

// 微调框
lv_obj_t * lv_spinbox_create(lv_obj_t * parent);
void lv_spinbox_set_range(lv_obj_t * obj, int32_t min, int32_t max);
void lv_spinbox_set_value(lv_obj_t * obj, int32_t value);
void lv_spinbox_set_step(lv_obj_t * obj, uint32_t step);
int32_t lv_spinbox_get_value(lv_obj_t * obj);

// 加载动画
lv_obj_t * lv_spinner_create(lv_obj_t * parent);

// LED
lv_obj_t * lv_led_create(lv_obj_t * parent);
void lv_led_set_color(lv_obj_t * obj, lv_color_t color);
void lv_led_set_brightness(lv_obj_t * obj, uint8_t bright);
void lv_led_on(lv_obj_t * obj);
void lv_led_off(lv_obj_t * obj);
void lv_led_toggle(lv_obj_t * obj);

// 折线
lv_obj_t * lv_line_create(lv_obj_t * parent);
void lv_line_set_points(lv_obj_t * obj, const lv_point_precise_t points[], uint32_t point_num);

// 消息框
lv_obj_t * lv_msgbox_create(lv_obj_t * parent);
lv_obj_t * lv_msgbox_add_title(lv_obj_t * obj, const char * title);
lv_obj_t * lv_msgbox_add_text(lv_obj_t * obj, const char * text);
lv_obj_t * lv_msgbox_add_footer_button(lv_obj_t * obj, const char * text);
lv_obj_t * lv_msgbox_add_close_button(lv_obj_t * obj);
void lv_msgbox_close(lv_obj_t * obj);
```

### 容器控件

```c
// TabView
lv_obj_t * lv_tabview_create(lv_obj_t * parent);
lv_obj_t * lv_tabview_add_tab(lv_obj_t * obj, const char * name);
void lv_tabview_set_active(lv_obj_t * obj, uint32_t idx, lv_anim_enable_t anim);
void lv_tabview_set_tab_bar_position(lv_obj_t * obj, lv_dir_t dir);
// dir: LV_DIR_TOP / BOTTOM / LEFT / RIGHT

// TileView
lv_obj_t * lv_tileview_create(lv_obj_t * parent);
lv_obj_t * lv_tileview_add_tile(lv_obj_t * obj, uint8_t col_id, uint8_t row_id, lv_dir_t dir);
void lv_tileview_set_tile(lv_obj_t * obj, lv_obj_t * tile_obj, lv_anim_enable_t anim);
void lv_tileview_set_tile_by_index(lv_obj_t * obj, uint32_t col, uint32_t row, lv_anim_enable_t anim);

// Win
lv_obj_t * lv_win_create(lv_obj_t * parent);
lv_obj_t * lv_win_add_title(lv_obj_t * obj, const char * title);
lv_obj_t * lv_win_add_button(lv_obj_t * obj, const char * icon, int32_t btn_w);
lv_obj_t * lv_win_get_header(lv_obj_t * obj);
lv_obj_t * lv_win_get_content(lv_obj_t * obj);

// List
lv_obj_t * lv_list_create(lv_obj_t * parent);
lv_obj_t * lv_list_add_text(lv_obj_t * obj, const char * txt);
lv_obj_t * lv_list_add_button(lv_obj_t * obj, const char * icon, const char * txt);
```

### 高级控件

```c
// 键盘
lv_obj_t * lv_keyboard_create(lv_obj_t * parent);
void lv_keyboard_set_textarea(lv_obj_t * kb, lv_obj_t * ta);
void lv_keyboard_set_mode(lv_obj_t * kb, lv_keyboard_mode_t mode);
// mode: LV_KEYBOARD_MODE_TEXT_LOWER / TEXT_UPPER / NUMBER

// 表格
lv_obj_t * lv_table_create(lv_obj_t * parent);
void lv_table_set_row_count(lv_obj_t * obj, uint32_t rows);
void lv_table_set_column_count(lv_obj_t * obj, uint32_t cols);
void lv_table_set_cell_value(lv_obj_t * obj, uint32_t row, uint32_t col, const char * txt);
const char * lv_table_get_cell_value(lv_obj_t * obj, uint32_t row, uint32_t col);

// 图表
lv_obj_t * lv_chart_create(lv_obj_t * parent);
void lv_chart_set_type(lv_obj_t * obj, lv_chart_type_t type);
// type: LV_CHART_TYPE_NONE / LINE / CURVE / BAR / STACKED / SCATTER
lv_chart_series_t * lv_chart_add_series(lv_obj_t * obj, lv_color_t color, lv_chart_axis_t axis);
void lv_chart_set_next_value(lv_obj_t * obj, lv_chart_series_t * ser, lv_coord_t value);
void lv_chart_set_range(lv_obj_t * obj, lv_chart_axis_t axis, int32_t min, int32_t max);
void lv_chart_set_point_count(lv_obj_t * obj, uint32_t cnt);
void lv_chart_set_update_mode(lv_obj_t * obj, lv_chart_update_mode_t mode);

// 日历
lv_obj_t * lv_calendar_create(lv_obj_t * parent);
void lv_calendar_set_today_date(lv_obj_t * obj, uint32_t year, uint32_t month, uint32_t day);
void lv_calendar_set_showed_date(lv_obj_t * obj, uint32_t year, uint32_t month);

// 按钮矩阵
lv_obj_t * lv_buttonmatrix_create(lv_obj_t * parent);
void lv_buttonmatrix_set_map(lv_obj_t * obj, const char ** map);

// 刻度尺
lv_obj_t * lv_scale_create(lv_obj_t * parent);
void lv_scale_set_mode(lv_obj_t * obj, lv_scale_mode_t mode);
void lv_scale_set_total_tick_count(lv_obj_t * obj, uint32_t total_tick_count);
void lv_scale_set_major_tick_every(lv_obj_t * obj, uint32_t major_tick_every);
void lv_scale_set_range(lv_obj_t * obj, int32_t min, int32_t max);
```

### 特效控件

```c
// 动画图像序列
lv_obj_t * lv_animimg_create(lv_obj_t * parent);
void lv_animimg_set_src(lv_obj_t * obj, const void * dsc[], size_t num);
void lv_animimg_set_duration(lv_obj_t * obj, uint32_t duration);
void lv_animimg_set_repeat_count(lv_obj_t * obj, uint32_t count);  // LV_ANIM_REPEAT_INFINITE
void lv_animimg_start(lv_obj_t * obj);

// GIF
lv_obj_t * lv_gif_create(lv_obj_t * parent);
void lv_gif_set_src(lv_obj_t * obj, const void * src);

// Lottie 动画
lv_obj_t * lv_lottie_create(lv_obj_t * parent);
void lv_lottie_set_src_data(lv_obj_t * obj, const void * data, size_t data_size);
void lv_lottie_set_buffer_size(lv_obj_t * obj, size_t size);
```

---

## 2. 对象通用操作

```c
// 位置和尺寸
void lv_obj_set_pos(lv_obj_t * obj, lv_coord_t x, lv_coord_t y);
void lv_obj_set_x(lv_obj_t * obj, lv_coord_t x);
void lv_obj_set_y(lv_obj_t * obj, lv_coord_t y);
void lv_obj_set_size(lv_obj_t * obj, lv_coord_t w, lv_coord_t h);
void lv_obj_set_width(lv_obj_t * obj, lv_coord_t w);
void lv_obj_set_height(lv_obj_t * obj, lv_coord_t h);
lv_coord_t lv_obj_get_width(const lv_obj_t * obj);
lv_coord_t lv_obj_get_height(const lv_obj_t * obj);

// 对齐
void lv_obj_align(lv_obj_t * obj, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs);
void lv_obj_align_to(lv_obj_t * obj, const lv_obj_t * base, lv_align_t align,
                     lv_coord_t x_ofs, lv_coord_t y_ofs);
void lv_obj_center(lv_obj_t * obj);

// 对齐枚举
// LV_ALIGN_DEFAULT        LV_ALIGN_TOP_LEFT       LV_ALIGN_TOP_MID
// LV_ALIGN_TOP_RIGHT      LV_ALIGN_BOTTOM_LEFT    LV_ALIGN_BOTTOM_MID
// LV_ALIGN_BOTTOM_RIGHT   LV_ALIGN_LEFT_MID       LV_ALIGN_RIGHT_MID
// LV_ALIGN_CENTER         LV_ALIGN_OUT_TOP_LEFT   (等 OUT_ 系列)

// Flag（控制控件行为）
void lv_obj_add_flag(lv_obj_t * obj, lv_obj_flag_t f);
void lv_obj_remove_flag(lv_obj_t * obj, lv_obj_flag_t f);
bool lv_obj_has_flag(const lv_obj_t * obj, lv_obj_flag_t f);

// 常用 Flag:
// LV_OBJ_FLAG_HIDDEN              - 隐藏
// LV_OBJ_FLAG_CLICKABLE           - 可点击
// LV_OBJ_FLAG_SCROLLABLE          - 可滚动
// LV_OBJ_FLAG_SCROLL_ELASTIC      - 弹性滚动
// LV_OBJ_FLAG_CLICK_FOCUSABLE     - 点击后获取焦点
// LV_OBJ_FLAG_EVENT_BUBBLE        - 事件冒泡
// LV_OBJ_FLAG_CHECKABLE           - 可勾选
// LV_OBJ_FLAG_SNAPPABLE           - 可吸附
// LV_OBJ_FLAG_OVERFLOW_VISIBLE    - 允许溢出可见

// State（控制视觉状态）
void lv_obj_add_state(lv_obj_t * obj, lv_state_t state);
void lv_obj_remove_state(lv_obj_t * obj, lv_state_t state);
bool lv_obj_has_state(const lv_obj_t * obj, lv_state_t state);

// 常用 State:
// LV_STATE_DEFAULT      LV_STATE_CHECKED     LV_STATE_FOCUSED
// LV_STATE_PRESSED      LV_STATE_DISABLED    LV_STATE_HOVERED
// LV_STATE_SCROLLED     LV_STATE_FOCUS_KEY   LV_STATE_EDITED

// 删除/清理
void lv_obj_delete(lv_obj_t * obj, uint32_t delay_ms);
void lv_obj_clean(lv_obj_t * obj);  // 删除所有子对象

// 树结构
lv_obj_t * lv_obj_get_parent(const lv_obj_t * obj);
lv_obj_t * lv_obj_get_child(const lv_obj_t * obj, int32_t idx);
uint32_t lv_obj_get_child_count(const lv_obj_t * obj);
void lv_obj_set_parent(lv_obj_t * obj, lv_obj_t * parent);
```

---

## 3. 样式系统

```c
// 样式创建
void lv_style_init(lv_style_t * style);
void lv_style_reset(lv_style_t * style);

// 常用样式 setter（完整列表 129 个，这里是最常用 30 个）
void lv_style_set_width(lv_style_t * style, int32_t value);
void lv_style_set_height(lv_style_t * style, int32_t value);
void lv_style_set_min_width(lv_style_t * style, int32_t value);
void lv_style_set_max_width(lv_style_t * style, int32_t value);
void lv_style_set_x(lv_style_t * style, int32_t value);
void lv_style_set_y(lv_style_t * style, int32_t value);
void lv_style_set_pad_all(lv_style_t * style, int32_t value);   // 统一设置四边内边距
void lv_style_set_pad_top(lv_style_t * style, int32_t value);
void lv_style_set_pad_bottom(lv_style_t * style, int32_t value);
void lv_style_set_pad_left(lv_style_t * style, int32_t value);
void lv_style_set_pad_right(lv_style_t * style, int32_t value);
void lv_style_set_margin_top(lv_style_t * style, int32_t value);
void lv_style_set_margin_bottom(lv_style_t * style, int32_t value);
void lv_style_set_radius(lv_style_t * style, int32_t value);
void lv_style_set_bg_color(lv_style_t * style, lv_color_t value);
void lv_style_set_bg_opa(lv_style_t * style, lv_opa_t value);
void lv_style_set_bg_grad_color(lv_style_t * style, lv_color_t value);
void lv_style_set_bg_grad_dir(lv_style_t * style, lv_grad_dir_t value);
void lv_style_set_border_width(lv_style_t * style, int32_t value);
void lv_style_set_border_color(lv_style_t * style, lv_color_t value);
void lv_style_set_border_opa(lv_style_t * style, lv_opa_t value);
void lv_style_set_outline_width(lv_style_t * style, int32_t value);
void lv_style_set_outline_color(lv_style_t * style, lv_color_t value);
void lv_style_set_text_color(lv_style_t * style, lv_color_t value);
void lv_style_set_text_font(lv_style_t * style, const lv_font_t * value);
void lv_style_set_text_align(lv_style_t * style, lv_text_align_t value);
void lv_style_set_text_letter_space(lv_style_t * style, int32_t value);
void lv_style_set_text_line_space(lv_style_t * style, int32_t value);
void lv_style_set_opa(lv_style_t * style, lv_opa_t value);
void lv_style_set_shadow_width(lv_style_t * style, int32_t value);
void lv_style_set_shadow_ofs_x(lv_style_t * style, int32_t value);
void lv_style_set_shadow_ofs_y(lv_style_t * style, int32_t value);
void lv_style_set_shadow_color(lv_style_t * style, lv_color_t value);
void lv_style_set_shadow_opa(lv_style_t * style, lv_opa_t value);
void lv_style_set_transform_rotation(lv_style_t * style, int32_t value);  // 0.1度
void lv_style_set_transform_scale_x(lv_style_t * style, int32_t value);   // 256=100%
void lv_style_set_anim_duration(lv_style_t * style, uint32_t value);
	void lv_style_set_shadow_spread(lv_style_t * style, int32_t value);
	// Drop Shadow
	void lv_style_set_drop_shadow_radius(lv_style_t * style, int32_t value);
	void lv_style_set_drop_shadow_offset_x(lv_style_t * style, int32_t value);
	void lv_style_set_drop_shadow_offset_y(lv_style_t * style, int32_t value);
	void lv_style_set_drop_shadow_color(lv_style_t * style, lv_color_t value);
	void lv_style_set_drop_shadow_opa(lv_style_t * style, lv_opa_t value);
	// Blur
	void lv_style_set_blur_radius(lv_style_t * style, int32_t value);
	void lv_style_set_blur_backdrop(lv_style_t * style, bool value);
	// Transform Advanced
	void lv_style_set_transform_scale_y(lv_style_t * style, int32_t value);
	void lv_style_set_transform_pivot_x(lv_style_t * style, int32_t value);
	void lv_style_set_transform_pivot_y(lv_style_t * style, int32_t value);
	void lv_style_set_transform_skew_x(lv_style_t * style, int32_t value);
	void lv_style_set_transform_skew_y(lv_style_t * style, int32_t value);
	void lv_style_set_translate_x(lv_style_t * style, int32_t value);
	void lv_style_set_translate_y(lv_style_t * style, int32_t value);
	void lv_style_set_translate_radial(lv_style_t * style, int32_t value);
	// Blend Mode & Filter
	void lv_style_set_blend_mode(lv_style_t * style, lv_blend_mode_t value);
	void lv_style_set_recolor(lv_style_t * style, lv_color_t value);
	void lv_style_set_recolor_opa(lv_style_t * style, lv_opa_t value);
	void lv_style_set_bitmap_mask_src(lv_style_t * style, const void * value);
	// Image Effects
	void lv_style_set_image_recolor(lv_style_t * style, lv_color_t value);
	void lv_style_set_image_recolor_opa(lv_style_t * style, lv_opa_t value);
	// Transition
	void lv_style_set_transition(lv_style_t * style, const lv_style_transition_dsc_t * value);
	// Pad Row/Column (Flex spacing)
	void lv_style_set_pad_row(lv_style_t * style, int32_t value);
	void lv_style_set_pad_column(lv_style_t * style, int32_t value);

// 将样式应用到对象
void lv_obj_add_style(lv_obj_t * obj, lv_style_t * style, lv_style_selector_t selector);
void lv_obj_remove_style(lv_obj_t * obj, lv_style_t * style, lv_style_selector_t selector);
void lv_obj_remove_style_all(lv_obj_t * obj);

// 样式选择器 = PART | STATE 组合
// LV_PART_MAIN         LV_PART_SCROLLBAR       LV_PART_INDICATOR
// LV_PART_KNOB         LV_PART_SELECTED        LV_PART_ITEMS
// LV_PART_CURSOR       LV_PART_CUSTOM_FIRST    LV_PART_ANY
// 例如: LV_PART_MAIN | LV_STATE_PRESSED

// 内联样式快捷方式（不创建 lv_style_t，直接设置）
void lv_obj_set_style_width(lv_obj_t * obj, int32_t value, lv_style_selector_t selector);
void lv_obj_set_style_height(lv_obj_t * obj, int32_t value, lv_style_selector_t selector);
void lv_obj_set_style_bg_color(lv_obj_t * obj, lv_color_t value, lv_style_selector_t selector);
void lv_obj_set_style_bg_opa(lv_obj_t * obj, lv_opa_t value, lv_style_selector_t selector);
void lv_obj_set_style_radius(lv_obj_t * obj, int32_t value, lv_style_selector_t selector);
void lv_obj_set_style_text_color(lv_obj_t * obj, lv_color_t value, lv_style_selector_t selector);
void lv_obj_set_style_text_font(lv_obj_t * obj, const lv_font_t * value, lv_style_selector_t selector);
void lv_obj_set_style_border_width(lv_obj_t * obj, int32_t value, lv_style_selector_t selector);
void lv_obj_set_style_pad_all(lv_obj_t * obj, int32_t value, lv_style_selector_t selector);
void lv_obj_set_style_opa(lv_obj_t * obj, lv_opa_t value, lv_style_selector_t selector);
void lv_obj_set_style_shadow_width(lv_obj_t * obj, int32_t value, lv_style_selector_t selector);
// 更多样式属性类似，共 129 个 lv_obj_set_style_xxx 函数
```

---

## 4. 布局系统

```c
// ===== Flex 布局 =====
void lv_obj_set_flex_flow(lv_obj_t * obj, lv_flex_flow_t flow);
// flow: LV_FLEX_FLOW_ROW / COLUMN / ROW_WRAP / COLUMN_WRAP / ROW_REVERSE / COLUMN_REVERSE / ROW_WRAP_REVERSE / COLUMN_WRAP_REVERSE

void lv_obj_set_flex_align(lv_obj_t * obj, lv_flex_align_t main_place,
                           lv_flex_align_t cross_place, lv_flex_align_t track_cross_place);
// align: LV_FLEX_ALIGN_START / END / CENTER / SPACE_EVENLY / SPACE_AROUND / SPACE_BETWEEN

void lv_obj_set_flex_grow(lv_obj_t * obj, uint8_t grow);  // 子控件伸展权重

// ===== Grid 布局 =====
void lv_obj_set_grid_dsc_array(lv_obj_t * obj, const int32_t col_dsc[], const int32_t row_dsc[]);
// 列/行模板值: LV_GRID_FR(x) / LV_GRID_CONTENT / LV_GRID_TEMPLATE_LAST

void lv_obj_set_grid_align(lv_obj_t * obj, lv_grid_align_t column_align, lv_grid_align_t row_align);
// align: LV_GRID_ALIGN_START / CENTER / END / STRETCH / SPACE_EVENLY / SPACE_AROUND / SPACE_BETWEEN

void lv_obj_set_grid_cell(lv_obj_t * obj, lv_grid_align_t column_align, int32_t col_pos,
                          int32_t col_span, lv_grid_align_t row_align, int32_t row_pos,
                          int32_t row_span);
```

---

## 5. 事件系统

```c
// 注册事件回调
lv_event_dsc_t * lv_obj_add_event_cb(lv_obj_t * obj, lv_event_cb_t event_cb,
                                     lv_event_code_t filter, void * user_data);
// 注意 v9 参数顺序: (obj, cb, event_code, user_data)
// v8 是: (obj, cb, event_code, user_data) — 相同，但 filter 现在叫 lv_event_code_t

// 在回调中获取信息
lv_obj_t * lv_event_get_target(lv_event_t * e);          // 事件目标
lv_obj_t * lv_event_get_current_target(lv_event_t * e);  // 冒泡时的当前对象
lv_event_code_t lv_event_get_code(lv_event_t * e);       // 事件类型
void * lv_event_get_user_data(lv_event_t * e);           // 用户数据
lv_indev_t * lv_event_get_indev(lv_event_t * e);         // 输入设备
uint32_t lv_event_get_key(lv_event_t * e);               // 按键码

// 全部事件类型（70+）
enum lv_event_code_t {
    // 输入设备事件
    LV_EVENT_PRESSED,          LV_EVENT_PRESSING,
    LV_EVENT_PRESS_LOST,       LV_EVENT_SHORT_CLICKED,
    LV_EVENT_SINGLE_CLICKED,   LV_EVENT_DOUBLE_CLICKED,
    LV_EVENT_TRIPLE_CLICKED,   LV_EVENT_LONG_PRESSED,
    LV_EVENT_LONG_PRESSED_REPEAT, LV_EVENT_CLICKED,
    LV_EVENT_RELEASED,         LV_EVENT_SCROLL_BEGIN,
    LV_EVENT_SCROLL_THROW_BEGIN, LV_EVENT_SCROLL_END,
    LV_EVENT_SCROLL,           LV_EVENT_GESTURE,
    LV_EVENT_KEY,              LV_EVENT_ROTARY,
    LV_EVENT_FOCUSED,          LV_EVENT_DEFOCUSED,
    LV_EVENT_LEAVE,            LV_EVENT_HIT_TEST,
    LV_EVENT_INDEV_RESET,      LV_EVENT_HOVER_OVER,
    LV_EVENT_HOVER_LEAVE,

    // 值变化
    LV_EVENT_VALUE_CHANGED,

    // 生命周期
    LV_EVENT_CREATE,           LV_EVENT_DELETE,
    LV_EVENT_CHILD_CHANGED,    LV_EVENT_CHILD_CREATED,
    LV_EVENT_CHILD_DELETED,    LV_EVENT_SIZE_CHANGED,

    // 屏幕
    LV_EVENT_SCREEN_UNLOAD_START, LV_EVENT_SCREEN_LOAD_START,
    LV_EVENT_SCREEN_LOADED,    LV_EVENT_SCREEN_UNLOADED,

    // 绘制（通常不需要处理）
    LV_EVENT_DRAW_MAIN,        LV_EVENT_DRAW_MAIN_BEGIN,
    LV_EVENT_DRAW_MAIN_END,    LV_EVENT_DRAW_POST,
    LV_EVENT_COVER_CHECK,      LV_EVENT_REFR_EXT_DRAW_SIZE,
    LV_EVENT_REFR_REQUEST,     LV_EVENT_REFR_START,
    LV_EVENT_REFR_READY,
    // ... 更多
};

// 停止冒泡
void lv_event_stop_bubbling(lv_event_t * e);
```

---

## 6. 动画系统

```c
// 基础动画
void lv_anim_init(lv_anim_t * a);
void lv_anim_set_var(lv_anim_t * a, void * var);               // 动画目标（通常是 lv_obj_t *）
void lv_anim_set_exec_cb(lv_anim_t * a, lv_anim_exec_xcb_t exec_cb); // 执行函数
// 常用 exec_cb: lv_obj_set_x, lv_obj_set_y, lv_obj_set_width,
//              lv_obj_set_height, lv_obj_set_opa
void lv_anim_set_values(lv_anim_t * a, int32_t start, int32_t end);
void lv_anim_set_duration(lv_anim_t * a, uint32_t duration);   // ms
void lv_anim_set_delay(lv_anim_t * a, uint32_t delay);         // ms
void lv_anim_set_path_cb(lv_anim_t * a, lv_anim_path_cb_t path_cb);
void lv_anim_set_early_apply(lv_anim_t * a, bool en);
void lv_anim_set_repeat_count(lv_anim_t * a, uint32_t cnt);    // LV_ANIM_REPEAT_INFINITE
void lv_anim_set_repeat_delay(lv_anim_t * a, uint32_t delay);
void lv_anim_set_start_cb(lv_anim_t * a, lv_anim_start_cb_t start_cb);
void lv_anim_set_completed_cb(lv_anim_t * a, lv_anim_completed_cb_t completed_cb);
void lv_anim_set_user_data(lv_anim_t * a, void * user_data);
lv_anim_t * lv_anim_start(const lv_anim_t * a);

// 预设缓动路径
lv_anim_path_linear             // 线性
lv_anim_path_ease_in            // 缓入
lv_anim_path_ease_out           // 缓出
lv_anim_path_ease_in_out        // 缓入缓出
lv_anim_path_overshoot          // 过冲
lv_anim_path_bounce             // 弹跳
lv_anim_path_step               // 步进

// 缓动宏（设置贝塞尔曲线参数）
LV_ANIM_SET_EASE_IN_SINE(anim)
LV_ANIM_SET_EASE_OUT_SINE(anim)
LV_ANIM_SET_EASE_IN_OUT_SINE(anim)
LV_ANIM_SET_EASE_IN_QUAD(anim)
LV_ANIM_SET_EASE_OUT_QUAD(anim)
LV_ANIM_SET_EASE_IN_OUT_QUAD(anim)
LV_ANIM_SET_EASE_IN_CUBIC(anim)
LV_ANIM_SET_EASE_OUT_CUBIC(anim)
LV_ANIM_SET_EASE_IN_OUT_CUBIC(anim)
LV_ANIM_SET_EASE_IN_QUART(anim)
LV_ANIM_SET_EASE_OUT_QUART(anim)
LV_ANIM_SET_EASE_IN_OUT_QUART(anim)
LV_ANIM_SET_EASE_IN_QUINT(anim)
LV_ANIM_SET_EASE_OUT_QUINT(anim)
LV_ANIM_SET_EASE_IN_OUT_QUINT(anim)
LV_ANIM_SET_EASE_IN_EXPO(anim)
LV_ANIM_SET_EASE_OUT_EXPO(anim)
LV_ANIM_SET_EASE_IN_OUT_EXPO(anim)
LV_ANIM_SET_EASE_IN_CIRC(anim)
LV_ANIM_SET_EASE_OUT_CIRC(anim)
LV_ANIM_SET_EASE_IN_OUT_CIRC(anim)
LV_ANIM_SET_EASE_IN_BACK(anim)
LV_ANIM_SET_EASE_OUT_BACK(anim)
LV_ANIM_SET_EASE_IN_OUT_BACK(anim)

// 动画控制
void lv_anim_pause(lv_anim_t * a);
void lv_anim_resume(lv_anim_t * a);
bool lv_anim_is_paused(lv_anim_t * a);
void lv_anim_delete(void * var, lv_anim_exec_xcb_t exec_cb);
void lv_anim_delete_all(void);

// 定时器（周期性任务）
lv_timer_t * lv_timer_create(lv_timer_cb_t timer_cb, uint32_t period_ms, void * user_data);
void lv_timer_ready(lv_timer_t * timer);    // 立即触发一次
void lv_timer_set_period(lv_timer_t * timer, uint32_t period);
void lv_timer_pause(lv_timer_t * timer);
void lv_timer_resume(lv_timer_t * timer);
void lv_timer_delete(lv_timer_t * timer);
```

---

## 7. 颜色与透明度

```c
// 颜色创建
lv_color_t lv_color_hex(uint32_t hex);                    // 0x00BBGGRR 或 0xRRGGBB（取决于配置）
lv_color_t lv_color_make(uint8_t r, uint8_t g, uint8_t b);
lv_color_t lv_color_white(void);
lv_color_t lv_color_black(void);

// 调色板（16 种颜色 × 10 级亮度）
lv_color_t lv_palette_main(lv_palette_t p);
lv_color_t lv_palette_lighten(lv_palette_t p, uint8_t lvl);   // lvl: 1-5
lv_color_t lv_palette_darken(lv_palette_t p, uint8_t lvl);    // lvl: 1-4
// palette: LV_PALETTE_RED / PINK / PURPLE / DEEP_PURPLE / INDIGO / BLUE
//           / LIGHT_BLUE / CYAN / TEAL / GREEN / LIGHT_GREEN / LIME / YELLOW
//           / AMBER / ORANGE / DEEP_ORANGE / BROWN / BLUE_GREY / GREY

// 透明度
lv_opa_t LV_OPA_TRANSP   // = 0
lv_opa_t LV_OPA_COVER    // = 255
lv_opa_t LV_OPA_10       // ... LV_OPA_90（每 10% 一档）

// 渐变方向
// LV_GRAD_DIR_NONE / HOR / VER / LV_GRAD_DIR_NONE
```

---

## 8. LVGL 内置符号

```c
LV_SYMBOL_AUDIO          // "\xEF\x80\x81"
LV_SYMBOL_VIDEO          // "\xEF\x80\x88"
LV_SYMBOL_LIST           // "\xEF\x80\x8B"
LV_SYMBOL_OK             // "\xEF\x80\x8C"
LV_SYMBOL_CLOSE          // "\xEF\x80\x8D"
LV_SYMBOL_POWER          // "\xEF\x80\x91"
LV_SYMBOL_SETTINGS       // "\xEF\x80\x93"
LV_SYMBOL_HOME           // "\xEF\x80\x95"
LV_SYMBOL_DOWNLOAD       // "\xEF\x80\x99"
LV_SYMBOL_REFRESH        // "\xEF\x80\xA1"
LV_SYMBOL_SEARCH         // "\xEF\x80\x82"
LV_SYMBOL_SAVE           // "\xEF\x83\x87"
LV_SYMBOL_TRASH          // "\xEF\x87\xB8"
LV_SYMBOL_EDIT           // "\xEF\x8C\x84"
LV_SYMBOL_WARNING        // "\xEF\x81\xB1"
LV_SYMBOL_PLAY           // "\xEF\x81\x8B"
LV_SYMBOL_PAUSE          // "\xEF\x81\x8C"
LV_SYMBOL_PLUS           // "\xEF\x81\xA7"
LV_SYMBOL_MINUS          // "\xEF\x81\xA8"
LV_SYMBOL_WIFI           // "\xEF\x87\xAB"
LV_SYMBOL_BLUETOOTH      // "\xEF\x8A\x93"
LV_SYMBOL_BATTERY_FULL   // "\xEF\x89\x80"
LV_SYMBOL_BATTERY_EMPTY  // "\xEF\x89\x84"
LV_SYMBOL_CHARGE         // "\xEF\x83\xA7"
LV_SYMBOL_HEART          // "\xEF\x80\x84"
LV_SYMBOL_STAR           // "\xEF\x80\x85"
LV_SYMBOL_USER           // "\xEF\x80\x87"
LV_SYMBOL_SD_CARD        // "\xEF\x9F\x82"
LV_SYMBOL_IMAGE          // "\xEF\x80\xBE"
LV_SYMBOL_CALL           // "\xEF\x82\x95"
LV_SYMBOL_CUT            // "\xEF\x83\x84"
LV_SYMBOL_COPY           // "\xEF\x83\x85"
LV_SYMBOL_PASTE          // "\xEF\x83\xAA"
LV_SYMBOL_EYE_OPEN       // "\xEF\x81\xAE"
LV_SYMBOL_EYE_CLOSE      // "\xEF\x81\xB0"
LV_SYMBOL_BELL           // "\xEF\x83\xB3"
LV_SYMBOL_MUTE           // "\xEF\x9A\xA8"
LV_SYMBOL_VOLUME_MID     // "\xEF\x9A\xA9"
// 这些符号需要字体支持。CJK 字体可能不支持 FontAwesome 符号。
// 使用符号时请确保有 LV_FONT_MONTSERRAT_xxx 字体可用。
```

---

## 9. 内置字体列表

```c
// Montserrat 系列（项目已全开 12-48px）
&lv_font_montserrat_12    // 到 &lv_font_montserrat_48（共 19 个尺寸）

// 中文字体
&lv_font_source_han_sans_sc_16_cjk   // 思源黑体 16px（项目已启用）

// 其他
&lv_font_dejavu_16_persian_hebrew    // 波斯希伯来语
&lv_font_unscii_8                     // 等宽像素字体
&lv_font_fmt_txt_large                // 大号字体
```

---

## 10. 日志输出

```c
LV_LOG_USER("用户自定义消息: %d", value);
LV_LOG_WARN("警告: %s", msg);
LV_LOG_ERROR("错误代码: %d", code);
LV_LOG_INFO("信息: 屏幕分辨率 %dx%d", w, h);
// 需要 lv_conf.defaults 中 LV_USE_LOG = 1
```
