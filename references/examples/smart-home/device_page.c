#include "smart_home.h"
#include <stdio.h>

/* ==================== 样式 ==================== */
static lv_style_t style_section_card;
static lv_style_t style_ctrl_btn;
static lv_style_t style_ctrl_btn_active;
static lv_style_t style_temp_btn;

/* ==================== 回调声明 ==================== */
static void on_light_switch(lv_event_t * e);
static void on_light_brightness(lv_event_t * e);
static void on_curtain_btn(lv_event_t * e);
static void on_curtain_slider(lv_event_t * e);
static void on_ac_power(lv_event_t * e);
static void on_ac_mode(lv_event_t * e);
static void on_ac_temp(lv_event_t * e);
static void on_ac_fan(lv_event_t * e);
static void on_outlet_switch(lv_event_t * e);
static void on_heater_switch(lv_event_t * e);

/* 控件引用（供回调更新UI） */
static lv_obj_t * light1_brightness_label;
static lv_obj_t * light2_brightness_label;
static lv_obj_t * curtain1_pos_label;
static lv_obj_t * curtain2_pos_label;
static lv_obj_t * ac_mode_label;
static lv_obj_t * ac_temp_label;
static lv_obj_t * ac_fan_label;
static lv_obj_t * ac_power_label;

/* ==================== 辅助：创建带边框的区域容器 ==================== */
static lv_obj_t * create_section_container(lv_obj_t * parent)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_width(cont, 776);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_add_style(cont, &style_section_card, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(cont, 10, 0);
    lv_obj_set_style_pad_row(cont, 6, 0);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    return cont;
}

/* ==================== 辅助：创建开关+标签+亮度值的行 ==================== */
static void create_light_row(lv_obj_t * container,
    const char * name, bool init_state, int init_brightness,
    lv_event_cb_t switch_cb, lv_event_cb_t slider_cb,
    lv_obj_t ** brightness_label_out)
{
    /* 行容器 */
    lv_obj_t * row = create_inline_row(container, 750, 50);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* 名称标签 */
    lv_obj_t * name_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(name_lbl, &lv_font_smart_home_16, 0);
    lv_obj_set_style_text_color(name_lbl, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_width(name_lbl, 120);
    lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
    lv_label_set_text(name_lbl, name);

    /* 开关 */
    lv_obj_t * sw = lv_switch_create(row);
    lv_obj_set_size(sw, 48, 26);
    if(init_state) lv_obj_add_state(sw, LV_STATE_CHECKED);
    if(switch_cb) lv_obj_add_event_cb(sw, switch_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* 亮度文字 */
    char buf[16];
    snprintf(buf, sizeof(buf), "亮度:%d%%", init_brightness);
    lv_obj_t * bright_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(bright_lbl, &lv_font_smart_home_16, 0);
    lv_obj_set_style_text_color(bright_lbl, lv_color_hex(COLOR_TEXT_SEC), 0);
    lv_obj_set_width(bright_lbl, 90);
    lv_label_set_long_mode(bright_lbl, LV_LABEL_LONG_CLIP);
    lv_label_set_text(bright_lbl, buf);
    if(brightness_label_out) *brightness_label_out = bright_lbl;

    /* 滑块（放在下一行） */
    lv_obj_t * slider_row = create_inline_row(container, 750, 36);
    lv_obj_set_flex_align(slider_row, LV_FLEX_ALIGN_END,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * slider = lv_slider_create(slider_row);
    lv_obj_set_size(slider, 550, 14);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, init_brightness, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_PRIMARY),
                              LV_PART_INDICATOR);
    if(slider_cb) lv_obj_add_event_cb(slider, slider_cb,
                                      LV_EVENT_VALUE_CHANGED, NULL);
}

/* ==================== 设备页面创建 ==================== */
void device_page_create(lv_obj_t * parent)
{
    /* ---- 初始化样式 ---- */
    lv_style_init(&style_section_card);
    lv_style_set_bg_color(&style_section_card, lv_color_hex(COLOR_CARD_BG));
    lv_style_set_bg_opa(&style_section_card, LV_OPA_COVER);
    lv_style_set_radius(&style_section_card, 10);
    lv_style_set_shadow_width(&style_section_card, 2);
    lv_style_set_shadow_color(&style_section_card, lv_color_hex(0xCCCCCC));
    lv_style_set_shadow_ofs_y(&style_section_card, 1);

    lv_style_init(&style_ctrl_btn);
    lv_style_set_bg_color(&style_ctrl_btn, lv_color_hex(0xE0E0E0));
    lv_style_set_bg_opa(&style_ctrl_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_ctrl_btn, 8);
    lv_style_set_text_font(&style_ctrl_btn, &lv_font_smart_home_16);
    lv_style_set_pad_all(&style_ctrl_btn, 6);

    lv_style_init(&style_ctrl_btn_active);
    lv_style_set_bg_color(&style_ctrl_btn_active, lv_color_hex(COLOR_PRIMARY));
    lv_style_set_bg_opa(&style_ctrl_btn_active, LV_OPA_COVER);
    lv_style_set_radius(&style_ctrl_btn_active, 8);
    lv_style_set_text_font(&style_ctrl_btn_active, &lv_font_smart_home_16);
    lv_style_set_text_color(&style_ctrl_btn_active, lv_color_white());
    lv_style_set_pad_all(&style_ctrl_btn_active, 6);

    lv_style_init(&style_temp_btn);
    lv_style_set_bg_color(&style_temp_btn, lv_color_hex(COLOR_PRIMARY));
    lv_style_set_bg_opa(&style_temp_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_temp_btn, LV_RADIUS_CIRCLE);
    lv_style_set_text_font(&style_temp_btn, &lv_font_smart_home_16);
    lv_style_set_text_color(&style_temp_btn, lv_color_white());
    lv_style_set_pad_all(&style_temp_btn, 8);

    /* ---- 启用滚动（设备页内容多，整页可滚） ---- */
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(parent, 10, 0);
    /* Tab 页面默认可滚动 */

    /* ===== 1. 灯光控制区域 ===== */
    create_section_label(parent, LV_SYMBOL_IMAGE, "灯光控制");
    lv_obj_t * light_section = create_section_container(parent);

    create_light_row(light_section,
        "客厅主灯", g_home_state.living_light,
        g_home_state.living_light_brightness,
        on_light_switch, on_light_brightness,
        &light1_brightness_label);

    /* 分割线 */
    lv_obj_t * sep1 = lv_obj_create(light_section);
    lv_obj_set_size(sep1, 740, 1);
    lv_obj_set_style_bg_color(sep1, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_border_width(sep1, 0, 0);

    create_light_row(light_section,
        "卧室灯", g_home_state.bedroom_light,
        g_home_state.bedroom_light_brightness,
        on_light_switch, on_light_brightness,
        &light2_brightness_label);

    lv_obj_t * sep2 = lv_obj_create(light_section);
    lv_obj_set_size(sep2, 740, 1);
    lv_obj_set_style_bg_color(sep2, lv_color_hex(0xE8E8E8), 0);
    lv_obj_set_style_border_width(sep2, 0, 0);

    /* 厨房灯：只有开关 */
    {
        lv_obj_t * row = create_inline_row(light_section, 750, 44);

        lv_obj_t * name_lbl = lv_label_create(row);
        lv_obj_set_style_text_font(name_lbl, &lv_font_smart_home_16, 0);
        lv_obj_set_style_text_color(name_lbl, lv_color_hex(COLOR_TEXT), 0);
        lv_obj_set_width(name_lbl, 120);
        lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
        lv_label_set_text(name_lbl, "厨房灯");

        lv_obj_t * sw = lv_switch_create(row);
        lv_obj_set_size(sw, 48, 26);
        if(g_home_state.kitchen_light) lv_obj_add_state(sw, LV_STATE_CHECKED);
        lv_obj_add_event_cb(sw, on_light_switch, LV_EVENT_VALUE_CHANGED, NULL);
    }

    /* ===== 2. 窗帘控制区域 ===== */
    create_section_label(parent, LV_SYMBOL_LEFT, "窗帘控制");
    lv_obj_t * curtain_section = create_section_container(parent);

    /* 客厅窗帘 */
    {
        lv_obj_t * row = create_inline_row(curtain_section, 750, 44);

        lv_obj_t * lbl = lv_label_create(row);
        lv_obj_set_style_text_font(lbl, &lv_font_smart_home_16, 0);
        lv_obj_set_width(lbl, 100);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_label_set_text(lbl, "客厅窗帘");

        lv_obj_t * btn_open = lv_button_create(row);
        lv_obj_add_style(btn_open, &style_ctrl_btn, 0);
        lv_obj_t * lbl_open = lv_label_create(btn_open);
        lv_label_set_text(lbl_open, "打开");
        lv_obj_add_event_cb(btn_open, on_curtain_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)1);

        lv_obj_t * btn_stop = lv_button_create(row);
        lv_obj_add_style(btn_stop, &style_ctrl_btn, 0);
        lv_obj_t * lbl_stop = lv_label_create(btn_stop);
        lv_label_set_text(lbl_stop, "暂停");
        lv_obj_add_event_cb(btn_stop, on_curtain_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)2);

        lv_obj_t * btn_close = lv_button_create(row);
        lv_obj_add_style(btn_close, &style_ctrl_btn, 0);
        lv_obj_t * lbl_close = lv_label_create(btn_close);
        lv_label_set_text(lbl_close, "关闭");
        lv_obj_add_event_cb(btn_close, on_curtain_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)0);

        char buf[16];
        snprintf(buf, sizeof(buf), "位置:%d%%", g_home_state.living_curtain_pos);
        lv_obj_t * pos_lbl = lv_label_create(row);
        lv_obj_set_style_text_font(pos_lbl, &lv_font_smart_home_16, 0);
        lv_label_set_text(pos_lbl, buf);
        curtain1_pos_label = pos_lbl;

        /* 窗帘位置滑块 */
        lv_obj_t * slider_row2 = create_inline_row(curtain_section, 750, 36);
        lv_obj_set_flex_align(slider_row2, LV_FLEX_ALIGN_END,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t * slider = lv_slider_create(slider_row2);
        lv_obj_set_size(slider, 550, 14);
        lv_slider_set_range(slider, 0, 100);
        lv_slider_set_value(slider, g_home_state.living_curtain_pos, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_PRIMARY),
                                  LV_PART_INDICATOR);
        lv_obj_add_event_cb(slider, on_curtain_slider,
                            LV_EVENT_VALUE_CHANGED, NULL);
    }

    /* 分割线 */
    {
        lv_obj_t * sep = lv_obj_create(curtain_section);
        lv_obj_set_size(sep, 740, 1);
        lv_obj_set_style_bg_color(sep, lv_color_hex(0xE8E8E8), 0);
        lv_obj_set_style_border_width(sep, 0, 0);
        lv_obj_set_scrollbar_mode(sep, LV_SCROLLBAR_MODE_OFF);
    }

    /* 卧室窗帘 */
    {
        lv_obj_t * row = create_inline_row(curtain_section, 750, 44);

        lv_obj_t * lbl = lv_label_create(row);
        lv_obj_set_style_text_font(lbl, &lv_font_smart_home_16, 0);
        lv_obj_set_width(lbl, 100);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_label_set_text(lbl, "卧室窗帘");

        lv_obj_t * btn_open = lv_button_create(row);
        lv_obj_add_style(btn_open, &style_ctrl_btn, 0);
        lv_obj_t * lbl_open = lv_label_create(btn_open);
        lv_label_set_text(lbl_open, "打开");
        lv_obj_add_event_cb(btn_open, on_curtain_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)3);

        lv_obj_t * btn_stop = lv_button_create(row);
        lv_obj_add_style(btn_stop, &style_ctrl_btn, 0);
        lv_obj_t * lbl_stop = lv_label_create(btn_stop);
        lv_label_set_text(lbl_stop, "暂停");
        lv_obj_add_event_cb(btn_stop, on_curtain_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)4);

        lv_obj_t * btn_close = lv_button_create(row);
        lv_obj_add_style(btn_close, &style_ctrl_btn, 0);
        lv_obj_t * lbl_close = lv_label_create(btn_close);
        lv_label_set_text(lbl_close, "关闭");
        lv_obj_add_event_cb(btn_close, on_curtain_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)5);

        char buf[16];
        snprintf(buf, sizeof(buf), "位置:%d%%", g_home_state.bedroom_curtain_pos);
        lv_obj_t * pos_lbl = lv_label_create(row);
        lv_obj_set_style_text_font(pos_lbl, &lv_font_smart_home_16, 0);
        lv_label_set_text(pos_lbl, buf);
        curtain2_pos_label = pos_lbl;
    }

    /* ===== 3. 空调控制区域 ===== */
    create_section_label(parent, LV_SYMBOL_CHARGE, "空调控制");
    lv_obj_t * ac_section = create_section_container(parent);

    /* 开关行 */
    {
        lv_obj_t * row = create_inline_row(ac_section, 750, 44);

        lv_obj_t * lbl = lv_label_create(row);
        lv_obj_set_style_text_font(lbl, &lv_font_smart_home_16, 0);
        lv_obj_set_width(lbl, 100);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_label_set_text(lbl, "客厅空调");

        lv_obj_t * sw = lv_switch_create(row);
        lv_obj_set_size(sw, 48, 26);
        if(g_home_state.ac_power) lv_obj_add_state(sw, LV_STATE_CHECKED);
        lv_obj_add_event_cb(sw, on_ac_power, LV_EVENT_VALUE_CHANGED, NULL);

        ac_power_label = lv_label_create(row);
        lv_obj_set_style_text_font(ac_power_label,
            &lv_font_smart_home_16, 0);
        lv_obj_set_style_text_color(ac_power_label,
            lv_color_hex(g_home_state.ac_power ? COLOR_ON : COLOR_OFF), 0);
        lv_label_set_text(ac_power_label,
            g_home_state.ac_power ? "已开启" : "已关闭");
    }

    /* 模式选择行 */
    {
        lv_obj_t * row = create_inline_row(ac_section, 750, 44);

        lv_obj_t * lbl = lv_label_create(row);
        lv_obj_set_style_text_font(lbl, &lv_font_smart_home_16, 0);
        lv_obj_set_width(lbl, 60);
        lv_label_set_text(lbl, "模式:");

        lv_obj_t * dd = lv_dropdown_create(row);
        lv_dropdown_set_options(dd, "制冷\n制热\n通风\n除湿");
        lv_dropdown_set_selected(dd, g_home_state.ac_mode);
        lv_dropdown_set_symbol(dd, LV_SYMBOL_DOWN);
        lv_obj_set_style_text_font(dd, &lv_font_smart_home_16, 0);
        lv_obj_set_style_text_font(dd, &lv_font_montserrat_16, LV_PART_INDICATOR);
        lv_obj_set_style_text_font(lv_dropdown_get_list(dd),
            &lv_font_smart_home_16, 0);
        lv_obj_add_event_cb(dd, on_ac_mode, LV_EVENT_VALUE_CHANGED, NULL);
    }

    /* 温度行 */
    {
        lv_obj_t * row = create_inline_row(ac_section, 750, 50);

        lv_obj_t * lbl = lv_label_create(row);
        lv_obj_set_style_text_font(lbl, &lv_font_smart_home_16, 0);
        lv_obj_set_width(lbl, 60);
        lv_label_set_text(lbl, "温度:");

        lv_obj_t * btn_dec = lv_button_create(row);
        lv_obj_set_size(btn_dec, 36, 36);
        lv_obj_add_style(btn_dec, &style_temp_btn, 0);
        lv_obj_t * lbl_dec = lv_label_create(btn_dec);
        lv_label_set_text(lbl_dec, "-");
        lv_obj_add_event_cb(btn_dec, on_ac_temp, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)0);

        char buf[8];
        snprintf(buf, sizeof(buf), "%d度", g_home_state.ac_temp);
        ac_temp_label = lv_label_create(row);
        lv_obj_set_style_text_font(ac_temp_label,
            &lv_font_smart_home_16, 0);
        lv_obj_set_style_text_color(ac_temp_label, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_set_width(ac_temp_label, 60);
        lv_obj_set_style_text_align(ac_temp_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(ac_temp_label, buf);

        lv_obj_t * btn_inc = lv_button_create(row);
        lv_obj_set_size(btn_inc, 36, 36);
        lv_obj_add_style(btn_inc, &style_temp_btn, 0);
        lv_obj_t * lbl_inc = lv_label_create(btn_inc);
        lv_label_set_text(lbl_inc, "+");
        lv_obj_add_event_cb(btn_inc, on_ac_temp, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)1);
    }

    /* 风速行 */
    {
        lv_obj_t * row = create_inline_row(ac_section, 750, 44);

        lv_obj_t * lbl = lv_label_create(row);
        lv_obj_set_style_text_font(lbl, &lv_font_smart_home_16, 0);
        lv_obj_set_width(lbl, 60);
        lv_label_set_text(lbl, "风速:");

        lv_obj_t * dd_fan = lv_dropdown_create(row);
        lv_dropdown_set_options(dd_fan, "自动\n低风\n中风\n高风");
        lv_dropdown_set_selected(dd_fan, g_home_state.ac_fan);
        lv_dropdown_set_symbol(dd_fan, LV_SYMBOL_DOWN);
        lv_obj_set_style_text_font(dd_fan, &lv_font_smart_home_16, 0);
        lv_obj_set_style_text_font(dd_fan, &lv_font_montserrat_16, LV_PART_INDICATOR);
        lv_obj_set_style_text_font(lv_dropdown_get_list(dd_fan),
            &lv_font_smart_home_16, 0);
        lv_obj_add_event_cb(dd_fan, on_ac_fan, LV_EVENT_VALUE_CHANGED, NULL);
    }

    /* ===== 4. 其他设备区域 ===== */
    create_section_label(parent, LV_SYMBOL_POWER, "其他设备");
    lv_obj_t * other_section = create_section_container(parent);

    create_switch_row(other_section, "客厅插座",
        g_home_state.outlet, on_outlet_switch);
    create_switch_row(other_section, "热水器",
        g_home_state.water_heater, on_heater_switch);
}

/* ==================== 回调函数 ==================== */

static void on_light_switch(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    bool state = lv_obj_has_state(sw, LV_STATE_CHECKED);
    LV_UNUSED(state);
    LV_LOG_USER("灯光开关切换");
}

static void on_light_brightness(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int val = (int)lv_slider_get_value(slider);
    /* 更新亮度标签（简化处理，直接更新light1_brightness_label） */
    char buf[16];
    snprintf(buf, sizeof(buf), "亮度:%d%%", val);
    if(light1_brightness_label) lv_label_set_text(light1_brightness_label, buf);
    if(light2_brightness_label) lv_label_set_text(light2_brightness_label, buf);
}

static void on_curtain_btn(lv_event_t * e)
{
    uintptr_t action = (uintptr_t)lv_event_get_user_data(e);
    const char * act_name[] = { "关闭", "打开", "暂停" };
    LV_LOG_USER("窗帘操作: %s (id=%lu)", act_name[action % 3],
                (unsigned long)action);
}

static void on_curtain_slider(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int val = (int)lv_slider_get_value(slider);
    char buf[16];
    snprintf(buf, sizeof(buf), "位置:%d%%", val);
    if(curtain1_pos_label) lv_label_set_text(curtain1_pos_label, buf);
}

static void on_ac_power(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    bool state = lv_obj_has_state(sw, LV_STATE_CHECKED);
    g_home_state.ac_power = state;
    if(ac_power_label) {
        lv_obj_set_style_text_color(ac_power_label,
            lv_color_hex(state ? COLOR_ON : COLOR_OFF), 0);
        lv_label_set_text(ac_power_label, state ? "已开启" : "已关闭");
    }
}

static void on_ac_mode(lv_event_t * e)
{
    lv_obj_t * dd = lv_event_get_target(e);
    int mode = (int)lv_dropdown_get_selected(dd);
    g_home_state.ac_mode = mode;
    LV_LOG_USER("空调模式: %s", g_ac_mode_names[mode]);
}

static void on_ac_temp(lv_event_t * e)
{
    uintptr_t dir = (uintptr_t)lv_event_get_user_data(e);
    if(dir == 1) {
        if(g_home_state.ac_temp < 30) g_home_state.ac_temp++;
    } else {
        if(g_home_state.ac_temp > 16) g_home_state.ac_temp--;
    }
    char buf[8];
    snprintf(buf, sizeof(buf), "%d度", g_home_state.ac_temp);
    if(ac_temp_label) lv_label_set_text(ac_temp_label, buf);
    LV_LOG_USER("空调温度: %d度", g_home_state.ac_temp);
}

static void on_ac_fan(lv_event_t * e)
{
    lv_obj_t * dd = lv_event_get_target(e);
    int fan = (int)lv_dropdown_get_selected(dd);
    g_home_state.ac_fan = fan;
    LV_LOG_USER("空调风速: %s", g_ac_fan_names[fan]);
}

static void on_outlet_switch(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    g_home_state.outlet = lv_obj_has_state(sw, LV_STATE_CHECKED);
}

static void on_heater_switch(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    g_home_state.water_heater = lv_obj_has_state(sw, LV_STATE_CHECKED);
}
