/**
 * @file page_stepper.c
 * @brief 步进电机控制页
 */

#include "motor_control.h"
#include "page_stepper.h"
#include <stdio.h>

/* ============== 共享数据 ============== */

static const int subdiv_vals[6] = { 1, 2, 4, 8, 16, 32 };
static const char *subdiv_txt[6] = { "1", "2", "4", "8", "16", "32" };

/* ============== 回调 ============== */

static void on_speed_change(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(slider);
    g_motor.stepper.speed_hz = (uint16_t)v;
    char buf[32];
    snprintf(buf, sizeof(buf), "%ld Hz", (long)v);
    lv_label_set_text(g_motor.sp_speed_lbl, buf);
}

static void on_steps_change(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t v = lv_slider_get_value(slider);
    g_motor.stepper.target_steps = (uint32_t)v;
    char buf[32];
    snprintf(buf, sizeof(buf), "%ld 步", (long)v);
    lv_label_set_text(g_motor.sp_steps_lbl, buf);
}

static void on_subdiv_chip_click(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    int val = subdiv_vals[idx];
    g_motor.stepper.subdiv = (stepper_subdiv_t)val;
    LV_LOG_USER("细分切换: 1/%d", val);
    for (int j = 0; j < 6; j++) {
        lv_obj_t *c = g_motor.sp_subdiv_chips[j];
        bool sel = (subdiv_vals[j] == val);
        lv_obj_set_style_bg_color(c, sel ? COL_AMBER : COL_CARD_HI, 0);
        lv_obj_set_style_border_color(c, sel ? COL_AMBER : COL_BORDER_HI, 0);
        lv_obj_t *lbl = lv_obj_get_child(c, 0);
        lv_obj_set_style_text_color(lbl, sel ? COL_BG : COL_TEXT_DIM, 0);
    }
}

static void on_dir_btn(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    if (idx == 3) {
        g_motor.stepper.enable = !g_motor.stepper.enable;
        LV_LOG_USER("步进使能: %s", g_motor.stepper.enable ? "ON" : "OFF");
        lv_obj_set_style_bg_color(g_motor.sp_enable_btn,
                                  g_motor.stepper.enable ? COL_GREEN : COL_CARD_HI, 0);
        lv_obj_set_style_border_color(g_motor.sp_enable_btn,
                                      g_motor.stepper.enable ? COL_GREEN : COL_BORDER, 0);
        lv_obj_t *lbl = lv_obj_get_child(g_motor.sp_enable_btn, 0);
        lv_obj_set_style_text_color(lbl,
                                    g_motor.stepper.enable ? COL_BG : COL_BORDER, 0);
        if (g_motor.sp_dot) {
            lv_obj_set_style_bg_color(g_motor.sp_dot,
                g_motor.stepper.enable ? COL_GREEN : COL_STOP, 0);
        }
        return;
    }
    motor_dir_t new_dir = (idx == 0) ? MOTOR_CCW : (idx == 1) ? MOTOR_STOP : MOTOR_CW;
    g_motor.stepper.dir = new_dir;
    g_motor.stepper.running = (new_dir != MOTOR_STOP) && g_motor.stepper.enable;
    LV_LOG_USER("步进方向: %s",
        new_dir == MOTOR_STOP ? "停止" : (new_dir == MOTOR_CW ? "正转" : "反转"));
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
    g_motor.sp_dot = dot;

    lv_obj_t *ttl = lv_label_create(row);
    lv_obj_set_style_text_font(ttl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(ttl, COL_AMBER, 0);
    lv_label_set_text(ttl, "步进电机 STEPPER #1");
    lv_label_set_long_mode(ttl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(ttl, 150);
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

/* ============== 滑块行 ============== */

static lv_obj_t *build_slider_row(lv_obj_t *parent, const char *name,
                                  const char *unit, int32_t min, int32_t max,
                                  int32_t init_val, lv_color_t indicator_col,
                                  lv_obj_t **out_slider, lv_obj_t **out_value_lbl)
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
    lv_label_set_text(name_lbl, name);
    lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(name_lbl, 60);
    lv_obj_set_scrollbar_mode(name_lbl, LV_SCROLLBAR_MODE_OFF);

    char val_buf[32];
    snprintf(val_buf, sizeof(val_buf), "%ld %s", (long)init_val, unit);
    lv_obj_t *val_lbl = lv_label_create(top);
    lv_obj_set_style_text_font(val_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(val_lbl, indicator_col, 0);
    lv_label_set_text(val_lbl, val_buf);
    lv_label_set_long_mode(val_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(val_lbl, 160);
    lv_obj_set_scrollbar_mode(val_lbl, LV_SCROLLBAR_MODE_OFF);
    if (out_value_lbl) *out_value_lbl = val_lbl;

    lv_obj_t *slider = lv_slider_create(col);
    lv_obj_set_size(slider, 232, 16);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, init_val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, COL_CARD, 0);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(slider, 3, 0);
    lv_obj_set_style_pad_all(slider, 0, 0);
    /* 指示器 */
    lv_obj_set_style_bg_color(slider, indicator_col, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(slider, 3, LV_PART_INDICATOR);
    /* 把手 (用 style_width/height 设置尺寸, 不是 set_size) */
    lv_obj_set_style_bg_color(slider, COL_TEXT, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
    lv_obj_set_style_border_color(slider, indicator_col, LV_PART_KNOB);
    lv_obj_set_style_border_width(slider, 1, LV_PART_KNOB);
    lv_obj_set_style_radius(slider, 2, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 0, LV_PART_KNOB);
    lv_obj_set_style_width(slider, 8, LV_PART_KNOB);
    lv_obj_set_style_height(slider, 16, LV_PART_KNOB);
    lv_obj_set_scrollbar_mode(slider, LV_SCROLLBAR_MODE_OFF);
    if (out_slider) *out_slider = slider;

    return col;
}

/* ============== 细分 chips ============== */

static lv_obj_t *build_subdiv_row(lv_obj_t *parent)
{
    lv_obj_t *row = create_inline_row(parent, 232, 24);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *name_lbl = lv_label_create(row);
    lv_obj_set_style_text_font(name_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(name_lbl, COL_TEXT_DIM, 0);
    lv_label_set_text(name_lbl, "细分");
    lv_label_set_long_mode(name_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(name_lbl, 28);
    lv_obj_set_scrollbar_mode(name_lbl, LV_SCROLLBAR_MODE_OFF);

    for (int i = 0; i < 6; i++) {
        lv_obj_t *chip = create_chip(row, subdiv_txt[i], 26, 22);
        bool selected = (subdiv_vals[i] == (int)g_motor.stepper.subdiv);
        if (selected) {
            lv_obj_set_style_bg_color(chip, COL_AMBER, 0);
            lv_obj_set_style_border_color(chip, COL_AMBER, 0);
            lv_obj_t *lbl = lv_obj_get_child(chip, 0);
            lv_obj_set_style_text_color(lbl, COL_BG, 0);
        }
        lv_obj_add_event_cb(chip, on_subdiv_chip_click,
                            LV_EVENT_CLICKED, (void *)(uintptr_t)i);
        g_motor.sp_subdiv_chips[i] = chip;
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
    accents[0] = COL_AMBER; accents[1] = COL_STOP;
    accents[2] = COL_AMBER; accents[3] = COL_BORDER;

    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_button_create(row);
        lv_obj_set_size(btn, 50, 28);
        style_button_industrial(btn, accents[i]);
        lv_obj_add_event_cb(btn, on_dir_btn, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)i);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, &lv_font_app_14, 0);
        lv_obj_set_style_text_color(lbl, accents[i], 0);
        lv_label_set_text(lbl, labels[i]);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_scrollbar_mode(lbl, LV_SCROLLBAR_MODE_OFF);

        if (i == 3) g_motor.sp_enable_btn = btn;
    }
    return row;
}

/* ============== 位置行 ============== */

static lv_obj_t *build_position_row(lv_obj_t *parent)
{
    lv_obj_t *row = create_inline_row(parent, 232, 20);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *lbl1 = lv_label_create(row);
    lv_obj_set_style_text_font(lbl1, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(lbl1, COL_TEXT_DIM, 0);
    lv_label_set_text(lbl1, "位置 0 步");
    lv_label_set_long_mode(lbl1, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl1, 110);
    lv_obj_set_scrollbar_mode(lbl1, LV_SCROLLBAR_MODE_OFF);
    g_motor.sp_pos_lbl = lbl1;

    lv_obj_t *lbl2 = lv_label_create(row);
    lv_obj_set_style_text_font(lbl2, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(lbl2, COL_TEXT_DIM, 0);
    lv_label_set_text(lbl2, "细分 1/8");
    lv_label_set_long_mode(lbl2, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(lbl2, 110);
    lv_obj_set_scrollbar_mode(lbl2, LV_SCROLLBAR_MODE_OFF);
    return row;
}

/* ============== 运行时长 ============== */

static lv_obj_t *build_runtime(lv_obj_t *parent)
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

    lv_obj_t *l = lv_label_create(bar);
    lv_obj_set_style_text_font(l, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(l, COL_TEXT_DIM, 0);
    lv_label_set_text(l, "运行时长 00:00:00");
    lv_label_set_long_mode(l, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(l, 200);
    lv_obj_set_scrollbar_mode(l, LV_SCROLLBAR_MODE_OFF);
    return bar;
}

/* ============== 主构建 ============== */

void page_stepper_build(lv_obj_t *parent)
{
    build_title_card(parent);

    build_slider_row(parent, "速度", "Hz", 0, 10000,
                     g_motor.stepper.speed_hz, COL_AMBER,
                     &g_motor.sp_speed_slider, &g_motor.sp_speed_lbl);
    lv_obj_add_event_cb(g_motor.sp_speed_slider, on_speed_change,
                        LV_EVENT_VALUE_CHANGED, NULL);

    build_slider_row(parent, "步数", "步", 0, 10000,
                     g_motor.stepper.target_steps, COL_AMBER,
                     &g_motor.sp_steps_slider, &g_motor.sp_steps_lbl);
    lv_obj_add_event_cb(g_motor.sp_steps_slider, on_steps_change,
                        LV_EVENT_VALUE_CHANGED, NULL);

    build_position_row(parent);
    build_subdiv_row(parent);
    build_control_btns(parent);
    build_runtime(parent);
}
