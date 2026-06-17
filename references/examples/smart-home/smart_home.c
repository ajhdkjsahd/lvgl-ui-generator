#include "smart_home.h"
#include <stdio.h>

/* ==================== 全局状态 ==================== */
smart_home_state_t g_home_state = {
    .living_light            = true,
    .living_light_brightness = 80,
    .bedroom_light           = false,
    .bedroom_light_brightness = 50,
    .kitchen_light           = true,

    .living_curtain      = false,
    .living_curtain_pos  = 0,
    .bedroom_curtain     = true,
    .bedroom_curtain_pos = 100,

    .ac_power = true,
    .ac_temp  = 26,
    .ac_mode  = 0,   /* 制冷 */
    .ac_fan   = 0,   /* 自动 */

    .outlet       = true,
    .water_heater = false,
};

sensor_data_t g_sensor_data = {
    .temperature = 26.5f,
    .humidity    = 65,
    .light_lux   = 850,
    .pm25        = 15,
    .battery     = 85,
};

const char * g_ac_mode_names[] = { "制冷", "制热", "通风", "除湿" };
const char * g_ac_fan_names[]  = { "自动", "低风", "中风", "高风" };
const char * g_scene_names[]   = {
    "回家", "离家", "睡眠", "观影",
    "阅读", "用餐", "起床", "节能"
};

/* ==================== 样式 ==================== */
static lv_style_t style_bg;

/* ==================== 主界面创建 ==================== */
lv_obj_t * smart_home_create(void)
{
    /* ---- 初始化样式 ---- */
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, lv_color_hex(COLOR_BG));
    lv_style_set_bg_opa(&style_bg, LV_OPA_COVER);

    /* ---- 创建屏幕 ---- */
    lv_obj_t * screen = lv_obj_create(NULL);
    lv_obj_add_style(screen, &style_bg, LV_PART_MAIN);

    /* ---- 创建 TabView ---- */
    lv_obj_t * tv = lv_tabview_create(screen);
    lv_obj_set_size(tv, 800, 480);
    lv_obj_align(tv, LV_ALIGN_CENTER, 0, 0);
    lv_tabview_set_tab_bar_position(tv, LV_DIR_BOTTOM);
    lv_obj_add_style(tv, &style_bg, LV_PART_MAIN);

    /* 设置 Tab 按钮字体为中文 - 通过 tab bar 设置 */
    lv_obj_t * tab_bar = lv_tabview_get_tab_bar(tv);
    lv_obj_set_style_text_font(tab_bar, &lv_font_smart_home_16, 0);

    /* ---- 添加三个 Tab ---- */
    lv_obj_t * tab_home   = lv_tabview_add_tab(tv, "首页");
    lv_obj_t * tab_device = lv_tabview_add_tab(tv, "设备控制");
    lv_obj_t * tab_scene  = lv_tabview_add_tab(tv, "智能场景");

    /* 设置 Tab 内容背景色 */
    lv_obj_add_style(tab_home,   &style_bg, LV_PART_MAIN);
    lv_obj_add_style(tab_device, &style_bg, LV_PART_MAIN);
    lv_obj_add_style(tab_scene,  &style_bg, LV_PART_MAIN);

    /* ---- 构建各页面内容 ---- */
    home_page_create(tab_home);
    device_page_create(tab_device);
    scene_page_create(tab_scene);

    return screen;
}

/* ==================== 工具函数 ==================== */

lv_obj_t * create_section_label(lv_obj_t * parent, const char * icon, const char * text)
{
    lv_obj_t * row = create_inline_row(parent, 776, 28);
    lv_obj_set_style_pad_top(row, 8, 0);
    lv_obj_set_style_pad_bottom(row, 4, 0);
    lv_obj_set_style_pad_left(row, 12, 0);

    lv_obj_t * icon_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(COLOR_PRIMARY), 0);
    lv_label_set_text(icon_lbl, icon);

    lv_obj_t * text_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(text_lbl, &lv_font_smart_home_16, 0);
    lv_obj_set_style_text_color(text_lbl, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_pad_left(text_lbl, 6, 0);
    lv_label_set_text(text_lbl, text);

    return row;
}

lv_obj_t * create_switch_row(lv_obj_t * parent, const char * label_text,
                              bool init_state, lv_event_cb_t cb)
{
    /* 行容器 */
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_set_size(row, 760, 44);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    /* 标签 */
    lv_obj_t * label = lv_label_create(row);
    lv_obj_set_style_text_font(label, &lv_font_smart_home_16, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(label, 600);
    lv_label_set_text(label, label_text);

    /* 开关 */
    lv_obj_t * sw = lv_switch_create(row);
    lv_obj_set_size(sw, 52, 28);
    if(init_state) lv_obj_add_state(sw, LV_STATE_CHECKED);
    if(cb) lv_obj_add_event_cb(sw, cb, LV_EVENT_VALUE_CHANGED, NULL);

    return sw;
}

lv_obj_t * create_slider_row(lv_obj_t * parent, const char * label_text,
                              int min, int max, int init_val,
                              lv_event_cb_t cb)
{
    /* 行容器 */
    lv_obj_t * row = lv_obj_create(parent);
    lv_obj_set_size(row, 760, 50);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    /* 标签 */
    lv_obj_t * label = lv_label_create(row);
    lv_obj_set_style_text_font(label, &lv_font_smart_home_16, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(label, 600);
    lv_label_set_text(label, label_text);

    /* 滑块 */
    lv_obj_t * slider = lv_slider_create(row);
    lv_obj_set_size(slider, 300, 16);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, init_val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_PRIMARY),
                              LV_PART_INDICATOR);
    if(cb) lv_obj_add_event_cb(slider, cb, LV_EVENT_VALUE_CHANGED, NULL);

    return slider;
}

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
    /* 关键：禁止内部滚动 */
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    return row;
}
