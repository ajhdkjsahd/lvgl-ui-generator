#include "smart_home.h"
#include <stdio.h>

/* ==================== 样式 ==================== */
static lv_style_t style_card;
static lv_style_t style_sensor_value;
static lv_style_t style_sensor_label;
static lv_style_t style_status_dot_on;
static lv_style_t style_status_dot_off;
static lv_style_t style_scene_btn;

/* ==================== 回调 ==================== */
static void on_scene_clicked(lv_event_t * e);
static void on_quick_toggle(lv_event_t * e);

/* ==================== 辅助：创建传感器卡片 ==================== */
static lv_obj_t * create_sensor_card(lv_obj_t * parent,
    const char * emoji, const char * value, const char * label)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_size(card, 140, 100);
    lv_obj_add_style(card, &style_card, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 图标 */
    lv_obj_t * icon = lv_label_create(card);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_20, 0);
    lv_label_set_text(icon, emoji);

    /* 数值 */
    lv_obj_t * val = lv_label_create(card);
    lv_obj_add_style(val, &style_sensor_value, 0);
    lv_label_set_long_mode(val, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(val, LV_PCT(100));
    lv_obj_set_style_text_align(val, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(val, value);

    /* 标签 */
    lv_obj_t * lbl = lv_label_create(card);
    lv_obj_add_style(lbl, &style_sensor_label, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl, LV_PCT(100));
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(lbl, label);

    return card;
}

/* ==================== 辅助：创建设备状态卡片 ==================== */
static lv_obj_t * create_status_card(lv_obj_t * parent,
    const char * icon, const char * title, const char * status_text, bool is_on)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_set_size(card, 370, 72);
    lv_obj_add_style(card, &style_card, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 左侧：图标 + 名称 + 状态 */
    lv_obj_t * left = lv_obj_create(card);
    lv_obj_set_width(left, 290);
    lv_obj_set_height(left, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(left, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left, 0, 0);
    lv_obj_set_style_pad_all(left, 0, 0);
    lv_obj_set_scrollbar_mode(left, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(left, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题行：图标(montserrat) + 文字(中文字体) 分开 */
    lv_obj_t * title_row = create_inline_row(left, 290, 22);
    lv_obj_set_flex_align(title_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * icon_lbl = lv_label_create(title_row);
    lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(COLOR_PRIMARY), 0);
    lv_label_set_text(icon_lbl, icon);

    lv_obj_t * title_lbl = lv_label_create(title_row);
    lv_obj_set_style_text_font(title_lbl, &lv_font_smart_home_16, 0);
    lv_obj_set_style_pad_left(title_lbl, 4, 0);
    lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(title_lbl, 270);
    lv_label_set_text(title_lbl, title);

    /* 状态文本 */
    lv_obj_t * status_lbl = lv_label_create(left);
    lv_obj_set_style_text_font(status_lbl, &lv_font_smart_home_16, 0);
    lv_obj_set_style_text_color(status_lbl,
        lv_color_hex(is_on ? COLOR_ON : COLOR_OFF), 0);
    lv_label_set_long_mode(status_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(status_lbl, LV_PCT(100));
    lv_label_set_text(status_lbl, status_text);

    /* 右侧：状态指示灯 */
    lv_obj_t * dot = lv_obj_create(card);
    lv_obj_set_size(dot, 18, 18);
    lv_obj_add_style(dot, is_on ? &style_status_dot_on : &style_status_dot_off, 0);

    return card;
}

/* ==================== 首页创建 ==================== */
void home_page_create(lv_obj_t * parent)
{
    /* ---- 初始化样式 ---- */
    lv_style_init(&style_card);
    lv_style_set_bg_color(&style_card, lv_color_hex(COLOR_CARD_BG));
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, 10);
    lv_style_set_pad_all(&style_card, 10);
    lv_style_set_shadow_width(&style_card, 3);
    lv_style_set_shadow_color(&style_card, lv_color_hex(0xCCCCCC));
    lv_style_set_shadow_ofs_y(&style_card, 2);

    lv_style_init(&style_sensor_value);
    lv_style_set_text_font(&style_sensor_value, &lv_font_smart_home_16);
    lv_style_set_text_color(&style_sensor_value, lv_color_hex(COLOR_TEXT));

    lv_style_init(&style_sensor_label);
    lv_style_set_text_font(&style_sensor_label, &lv_font_smart_home_16);
    lv_style_set_text_color(&style_sensor_label, lv_color_hex(COLOR_TEXT_SEC));

    lv_style_init(&style_status_dot_on);
    lv_style_set_bg_color(&style_status_dot_on, lv_color_hex(COLOR_ON));
    lv_style_set_bg_opa(&style_status_dot_on, LV_OPA_COVER);
    lv_style_set_radius(&style_status_dot_on, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_status_dot_on, 0);

    lv_style_init(&style_status_dot_off);
    lv_style_set_bg_color(&style_status_dot_off, lv_color_hex(COLOR_OFF));
    lv_style_set_bg_opa(&style_status_dot_off, LV_OPA_COVER);
    lv_style_set_radius(&style_status_dot_off, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_status_dot_off, 0);

    lv_style_init(&style_scene_btn);
    lv_style_set_bg_color(&style_scene_btn, lv_color_hex(COLOR_SCENE_BTN));
    lv_style_set_bg_opa(&style_scene_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_scene_btn, 10);
    lv_style_set_text_font(&style_scene_btn, &lv_font_smart_home_16);
    lv_style_set_text_color(&style_scene_btn, lv_color_white());
    lv_style_set_pad_all(&style_scene_btn, 8);

    /* ---- 父容器设置：垂直Flex布局 ---- */
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    /* ===== 1. 标题栏：图标 + 中文分开 ===== */
    lv_obj_t * title_row = create_inline_row(parent, 776, 36);
    lv_obj_set_style_pad_top(title_row, 4, 0);
    lv_obj_set_style_pad_bottom(title_row, 4, 0);
    lv_obj_set_flex_align(title_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * title_icon = lv_label_create(title_row);
    lv_obj_set_style_text_font(title_icon, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title_icon, lv_color_hex(COLOR_PRIMARY), 0);
    lv_label_set_text(title_icon, LV_SYMBOL_HOME);

    lv_obj_t * title_text = lv_label_create(title_row);
    lv_obj_set_style_text_font(title_text, &lv_font_smart_home_24, 0);
    lv_obj_set_style_text_color(title_text, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_pad_left(title_text, 8, 0);
    lv_label_set_text(title_text, "智能家居控制中心");

    /* ===== 2. 传感器数据行 ===== */
    lv_obj_t * sensor_row = lv_obj_create(parent);
    lv_obj_set_size(sensor_row, 784, 108);
    lv_obj_set_flex_flow(sensor_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sensor_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(sensor_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sensor_row, 0, 0);
    lv_obj_set_style_pad_all(sensor_row, 0, 0);

    /* 动态生成传感器数值文本 */
    char val_buf[32];

    snprintf(val_buf, sizeof(val_buf), "%.1f度", (double)g_sensor_data.temperature);
    create_sensor_card(sensor_row, LV_SYMBOL_TINT, val_buf, "温度");

    snprintf(val_buf, sizeof(val_buf), "%d%%RH", g_sensor_data.humidity);
    create_sensor_card(sensor_row, LV_SYMBOL_TINT, val_buf, "湿度");

    snprintf(val_buf, sizeof(val_buf), "%d lux", g_sensor_data.light_lux);
    create_sensor_card(sensor_row, LV_SYMBOL_IMAGE, val_buf, "光照");

    snprintf(val_buf, sizeof(val_buf), "%d ug/m3", g_sensor_data.pm25);
    create_sensor_card(sensor_row, LV_SYMBOL_REFRESH, val_buf, "PM2.5");

    snprintf(val_buf, sizeof(val_buf), "%d%%", g_sensor_data.battery);
    create_sensor_card(sensor_row, LV_SYMBOL_CHARGE, val_buf, "电量");

    /* ===== 3. 设备快速状态区域 ===== */
    create_section_label(parent, LV_SYMBOL_LIST, "设备状态");

    /* 第一行：灯光 + 窗帘 */
    lv_obj_t * dev_row1 = lv_obj_create(parent);
    lv_obj_set_size(dev_row1, 784, 80);
    lv_obj_set_flex_flow(dev_row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dev_row1, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(dev_row1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(dev_row1, 0, 0);
    lv_obj_set_style_pad_all(dev_row1, 0, 0);

    snprintf(val_buf, sizeof(val_buf), "%s 亮度 %d%%",
        g_home_state.living_light ? "已开启" : "已关闭",
        g_home_state.living_light_brightness);
    create_status_card(dev_row1, LV_SYMBOL_IMAGE, "客厅灯光", val_buf,
        g_home_state.living_light);

    snprintf(val_buf, sizeof(val_buf), "%s %d%%",
        g_home_state.living_curtain ? "已打开" : "已关闭",
        g_home_state.living_curtain_pos);
    create_status_card(dev_row1, LV_SYMBOL_LEFT, "客厅窗帘", val_buf,
        g_home_state.living_curtain);

    /* 第二行：空调 + 插座/热水器 */
    lv_obj_t * dev_row2 = lv_obj_create(parent);
    lv_obj_set_size(dev_row2, 784, 80);
    lv_obj_set_flex_flow(dev_row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dev_row2, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(dev_row2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(dev_row2, 0, 0);
    lv_obj_set_style_pad_all(dev_row2, 0, 0);

    snprintf(val_buf, sizeof(val_buf), "%s %d度 %s",
        g_home_state.ac_power ? "运行中" : "已关闭",
        g_home_state.ac_temp,
        g_ac_mode_names[g_home_state.ac_mode]);
    create_status_card(dev_row2, LV_SYMBOL_CHARGE, "客厅空调", val_buf,
        g_home_state.ac_power);

    snprintf(val_buf, sizeof(val_buf), "插座%s  热水器%s",
        g_home_state.outlet ? "已开启" : "已关闭",
        g_home_state.water_heater ? "已开启" : "已关闭");
    create_status_card(dev_row2, LV_SYMBOL_POWER, "其他设备", val_buf,
        g_home_state.outlet || g_home_state.water_heater);

    /* ===== 4. 快捷场景按钮 ===== */
    create_section_label(parent, LV_SYMBOL_PLAY, "快捷场景");

    lv_obj_t * scene_row = lv_obj_create(parent);
    lv_obj_set_size(scene_row, 784, 60);
    lv_obj_set_flex_flow(scene_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(scene_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(scene_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scene_row, 0, 0);
    lv_obj_set_style_pad_all(scene_row, 0, 0);

    /* 只显示前4个常用场景 */
    const char * scene_btn_symbols[] = {
        LV_SYMBOL_HOME, LV_SYMBOL_POWER, LV_SYMBOL_MUTE, LV_SYMBOL_PLAY
    };
    for(int i = 0; i < 4; i++) {
        lv_obj_t * btn = lv_button_create(scene_row);
        lv_obj_set_size(btn, 175, 48);
        lv_obj_add_style(btn, &style_scene_btn, 0);

        /* 按钮内水平排列图标和文字 */
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t * icon_lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_16, 0);
        lv_label_set_text(icon_lbl, scene_btn_symbols[i]);

        lv_obj_t * text_lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(text_lbl, &lv_font_smart_home_16, 0);
        lv_label_set_text(text_lbl, g_scene_names[i]);

        lv_obj_add_event_cb(btn, on_scene_clicked, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)i);
    }
}

/* ==================== 回调函数 ==================== */

static void on_scene_clicked(lv_event_t * e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    LV_LOG_USER("场景 [%s] 已激活", g_scene_names[idx]);
}

static void on_quick_toggle(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    bool state = lv_obj_has_state(sw, LV_STATE_CHECKED);
    LV_LOG_USER("快速开关切换: %s", state ? "ON" : "OFF");
    (void)state;
}
