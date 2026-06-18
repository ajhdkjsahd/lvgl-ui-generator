/**
 * @file page_dc.c
 * @brief 直流电机控制页
 */

#include "motor_control.h"
#include "page_dc.h"
#include <stdio.h>

/* ============== 回调 ============== */

static void on_pwm_change(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(slider);
    g_motor.dc.pwm_pct = (uint8_t)v;
    float pwm_f = (float)v / 100.0f;
    g_motor.dc.current_a = 0.45f * pwm_f;
    g_motor.dc.voltage_v = 12.3f - 0.5f * pwm_f;

    char buf[32];
    snprintf(buf, sizeof(buf), "%ld %%", (long)v);
    lv_label_set_text(g_motor.dc_pwm_lbl, buf);
    snprintf(buf, sizeof(buf), "%.1fV", (double)g_motor.dc.voltage_v);
    lv_label_set_text(g_motor.dc_v_lbl, buf);
    snprintf(buf, sizeof(buf), "%.2fA", (double)g_motor.dc.current_a);
    lv_label_set_text(g_motor.dc_i_lbl, buf);
    snprintf(buf, sizeof(buf), "%.1fW",
             (double)(g_motor.dc.voltage_v * g_motor.dc.current_a));
    lv_label_set_text(g_motor.dc_p_lbl, buf);
}

static void on_prot_chip_click(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    static const char *names[3] = { "过流", "堵转", "制动" };
    bool *flags[3] = { &g_motor.dc.prot_overcurrent,
                       &g_motor.dc.prot_stall,
                       &g_motor.dc.brake };
    *flags[idx] = !*flags[idx];
    LV_LOG_USER("保护 %s: %s", names[idx], *flags[idx] ? "开" : "关");
    lv_obj_t *c = g_motor.dc_prot_chips[idx];
    lv_color_t bg = *flags[idx] ? COL_AMBER : COL_CARD_HI;
    lv_color_t bd = *flags[idx] ? COL_AMBER : COL_BORDER_HI;
    lv_color_t fg = *flags[idx] ? COL_BG : COL_TEXT_DIM;
    lv_obj_set_style_bg_color(c, bg, 0);
    lv_obj_set_style_border_color(c, bd, 0);
    lv_obj_t *lbl = lv_obj_get_child(c, 0);
    lv_obj_set_style_text_color(lbl, fg, 0);
    char buf[16];
    snprintf(buf, sizeof(buf), "%s:%s", names[idx], *flags[idx] ? "开" : "关");
    lv_label_set_text(lbl, buf);
}

static void on_dc_dir_btn(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    if (idx == 3) {
        g_motor.dc.enable = !g_motor.dc.enable;
        LV_LOG_USER("直流使能: %s", g_motor.dc.enable ? "ON" : "OFF");
        lv_obj_set_style_bg_color(g_motor.dc_enable_btn,
            g_motor.dc.enable ? COL_GREEN : COL_CARD_HI, 0);
        lv_obj_set_style_border_color(g_motor.dc_enable_btn,
            g_motor.dc.enable ? COL_GREEN : COL_BORDER, 0);
        lv_obj_t *lbl = lv_obj_get_child(g_motor.dc_enable_btn, 0);
        lv_obj_set_style_text_color(lbl,
            g_motor.dc.enable ? COL_BG : COL_BORDER, 0);
        if (g_motor.dc_dot) {
            lv_obj_set_style_bg_color(g_motor.dc_dot,
                g_motor.dc.enable ? COL_CYAN : COL_STOP, 0);
        }
        return;
    }
    motor_dir_t new_dir = (idx == 0) ? MOTOR_CCW : (idx == 1) ? MOTOR_STOP : MOTOR_CW;
    g_motor.dc.dir = new_dir;
    LV_LOG_USER("直流方向: %s",
        new_dir == MOTOR_STOP ? "停止" : (new_dir == MOTOR_CW ? "正转" : "反转"));
}

static void on_brake_save_click(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    LV_LOG_USER("直流按钮: %s", idx == 0 ? "归零" : "保存参数");
}

/* ============== 标题卡 ============== */

static lv_obj_t *build_title_card(lv_obj_t *parent)
{
    lv_obj_t *row = create_inline_row(parent, 232, 16);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *dot = lv_obj_create(row);
    lv_obj_set_size(dot, 8, 8);
    lv_obj_set_style_bg_color(dot, COL_STOP, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(dot, 4, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_scrollbar_mode(dot, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    g_motor.dc_dot = dot;

    lv_obj_t *ttl = lv_label_create(row);
    lv_obj_set_style_text_font(ttl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(ttl, COL_CYAN, 0);
    lv_label_set_text(ttl, "直流电机 DC MOTOR #2");
    lv_label_set_long_mode(ttl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(ttl, 160);
    lv_obj_set_scrollbar_mode(ttl, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *st = lv_label_create(row);
    lv_obj_set_style_text_font(st, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(st, COL_TEXT_DIM, 0);
    lv_label_set_text(st, "停止");
    lv_label_set_long_mode(st, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(st, 40);
    lv_obj_set_scrollbar_mode(st, LV_SCROLLBAR_MODE_OFF);
    return row;
}

/* ============== PWM 滑块 ============== */

static lv_obj_t *build_pwm_row(lv_obj_t *parent)
{
    lv_obj_t *col = lv_obj_create(parent);
    lv_obj_set_size(col, 232, 40);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_set_style_pad_gap(col, 2, 0);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollbar_mode(col, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *top = create_inline_row(col, 232, 16);
    lv_obj_set_flex_align(top, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *name_lbl = lv_label_create(top);
    lv_obj_set_style_text_font(name_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(name_lbl, COL_TEXT_DIM, 0);
    lv_label_set_text(name_lbl, "PWM");
    lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(name_lbl, 50);
    lv_obj_set_scrollbar_mode(name_lbl, LV_SCROLLBAR_MODE_OFF);

    char buf[32];
    snprintf(buf, sizeof(buf), "%d %%", g_motor.dc.pwm_pct);
    lv_obj_t *val_lbl = lv_label_create(top);
    lv_obj_set_style_text_font(val_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(val_lbl, COL_CYAN, 0);
    lv_label_set_text(val_lbl, buf);
    lv_label_set_long_mode(val_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(val_lbl, 170);
    lv_obj_set_scrollbar_mode(val_lbl, LV_SCROLLBAR_MODE_OFF);
    g_motor.dc_pwm_lbl = val_lbl;

    lv_obj_t *slider = lv_slider_create(col);
    lv_obj_set_size(slider, 232, 16);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, g_motor.dc.pwm_pct, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, COL_CARD, 0);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(slider, 3, 0);
    lv_obj_set_style_pad_all(slider, 0, 0);
    lv_obj_set_style_bg_color(slider, COL_CYAN, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, 3, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, COL_TEXT, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_border_color(slider, COL_CYAN, LV_PART_KNOB);
    lv_obj_set_style_border_width(slider, 1, LV_PART_KNOB);
    lv_obj_set_style_radius(slider, 2, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 0, LV_PART_KNOB);
    lv_obj_set_style_width(slider, 8, LV_PART_KNOB);
    lv_obj_set_style_height(slider, 16, LV_PART_KNOB);
    lv_obj_set_scrollbar_mode(slider, LV_SCROLLBAR_MODE_OFF);
    g_motor.dc_pwm_slider = slider;
    return col;
}

/* ============== V/I/P 状态 ============== */

static lv_obj_t *build_vip_row(lv_obj_t *parent)
{
    lv_obj_t *row = create_inline_row(parent, 232, 20);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    char buf[16];
    snprintf(buf, sizeof(buf), "%.1fV", (double)g_motor.dc.voltage_v);
    lv_obj_t *v_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(v_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(v_lbl, COL_TEXT, 0);
    lv_label_set_text(v_lbl, buf);
    lv_label_set_long_mode(v_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(v_lbl, 70);
    lv_obj_set_scrollbar_mode(v_lbl, LV_SCROLLBAR_MODE_OFF);
    g_motor.dc_v_lbl = v_lbl;

    snprintf(buf, sizeof(buf), "%.2fA", (double)g_motor.dc.current_a);
    lv_obj_t *i_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(i_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(i_lbl, COL_TEXT, 0);
    lv_label_set_text(i_lbl, buf);
    lv_label_set_long_mode(i_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(i_lbl, 70);
    lv_obj_set_scrollbar_mode(i_lbl, LV_SCROLLBAR_MODE_OFF);
    g_motor.dc_i_lbl = i_lbl;

    snprintf(buf, sizeof(buf), "%.1fW",
             (double)(g_motor.dc.voltage_v * g_motor.dc.current_a));
    lv_obj_t *p_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(p_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(p_lbl, COL_AMBER, 0);
    lv_label_set_text(p_lbl, buf);
    lv_label_set_long_mode(p_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(p_lbl, 70);
    lv_obj_set_scrollbar_mode(p_lbl, LV_SCROLLBAR_MODE_OFF);
    g_motor.dc_p_lbl = p_lbl;

    return row;
}

/* ============== 保护 chips ============== */

static lv_obj_t *build_prot_row(lv_obj_t *parent)
{
    lv_obj_t *row = create_inline_row(parent, 232, 24);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *name_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(name_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(name_lbl, COL_TEXT_DIM, 0);
    lv_label_set_text(name_lbl, "保护");
    lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(name_lbl, 28);
    lv_obj_set_scrollbar_mode(name_lbl, LV_SCROLLBAR_MODE_OFF);

    static const char *prot_txt[3] = { "过流:关", "堵转:关", "制动:关" };
    for (int i = 0; i < 3; i++) {
        lv_obj_t *chip = create_chip(row, prot_txt[i], 56, 22);
        lv_obj_add_event_cb(chip, on_prot_chip_click,
                            LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        g_motor.dc_prot_chips[i] = chip;
    }
    return row;
}

/* ============== 控制按钮 ============== */

static lv_obj_t *build_control_btns(lv_obj_t *parent)
{
    lv_obj_t *row = create_inline_row(parent, 232, 32);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    static const char *labels[4] = { "反转", "停止", "正转", "使能" };
    static lv_color_t accents[4];
    accents[0] = COL_CYAN; accents[1] = COL_STOP;
    accents[2] = COL_CYAN; accents[3] = COL_BORDER;

    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_button_create(row);
        lv_obj_set_size(btn, 50, 28);
        style_button_industrial(btn, accents[i]);
        lv_obj_add_event_cb(btn, on_dc_dir_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)i);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, &lv_font_app_14, 0);
        lv_obj_set_style_text_color(lbl, accents[i], 0);
        lv_label_set_text(lbl, labels[i]);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_scrollbar_mode(lbl, LV_SCROLLBAR_MODE_OFF);

        if (i == 3) g_motor.dc_enable_btn = btn;
    }
    return row;
}

/* ============== 制动/保存 ============== */

static lv_obj_t *build_brake_save_row(lv_obj_t *parent)
{
    lv_obj_t *row = create_inline_row(parent, 232, 32);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    static const char *labels[2] = { "归零", "保存参数" };
    for (int i = 0; i < 2; i++) {
        lv_obj_t *btn = lv_button_create(row);
        lv_obj_set_size(btn, 110, 28);
        style_button_industrial(btn, COL_CYAN);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, &lv_font_app_14, 0);
        lv_obj_set_style_text_color(lbl, COL_CYAN, 0);
        lv_label_set_text(lbl, labels[i]);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_scrollbar_mode(lbl, LV_SCROLLBAR_MODE_OFF);

        lv_obj_add_event_cb(btn, on_brake_save_click, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)i);
    }
    return row;
}

/* ============== 统计 ============== */

static lv_obj_t *build_stats(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 232, 24);
    lv_obj_set_style_bg_color(bar, COL_CARD, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 3, 0);
    lv_obj_set_style_pad_left(bar, 8, 0);
    lv_obj_set_style_pad_right(bar, 8, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *l1 = lv_label_create(bar);
    lv_obj_set_style_text_font(l1, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(l1, COL_TEXT_DIM, 0);
    lv_label_set_text(l1, "时长 00:00:00");
    lv_label_set_long_mode(l1, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(l1, 105);
    lv_obj_set_scrollbar_mode(l1, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *l2 = lv_label_create(bar);
    lv_obj_set_style_text_font(l2, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(l2, COL_TEXT_DIM, 0);
    lv_label_set_text(l2, "启停 0 次");
    lv_label_set_long_mode(l2, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(l2, 100);
    lv_obj_set_scrollbar_mode(l2, LV_SCROLLBAR_MODE_OFF);
    return bar;
}

/* ============== 主构建 ============== */

void page_dc_build(lv_obj_t *parent)
{
    build_title_card(parent);
    build_pwm_row(parent);
    build_vip_row(parent);
    build_prot_row(parent);
    build_control_btns(parent);
    build_brake_save_row(parent);
    build_stats(parent);

    lv_obj_add_event_cb(g_motor.dc_pwm_slider, on_pwm_change,
                        LV_EVENT_VALUE_CHANGED, NULL);
}
