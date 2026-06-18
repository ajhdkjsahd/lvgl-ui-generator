/**
 * @file page_overview.c
 * @brief 概览页: 系统状态 + 两电机卡片 + 底部信息
 */

#include "motor_control.h"
#include "page_overview.h"
#include "page_stepper.h"
#include "page_dc.h"
#include <stdio.h>

/* ============== 回调 ============== */

static void on_view_btn_click(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    switch_to_tab((uint8_t)(idx + 1));
}

/* ============== 系统状态条 ============== */

static lv_obj_t *build_status_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 232, 24);
    lv_obj_set_style_bg_color(bar, COL_CARD, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 3, 0);
    lv_obj_set_style_pad_left(bar, 8, 0);
    lv_obj_set_style_pad_right(bar, 8, 0);
    lv_obj_set_style_pad_top(bar, 0, 0);
    lv_obj_set_style_pad_bottom(bar, 0, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    static const char *items[3] = { "CAN已连", "系统就绪", "急停正常" };
    static lv_color_t dot_colors[3];
    dot_colors[0] = COL_GREEN; dot_colors[1] = COL_GREEN; dot_colors[2] = COL_AMBER;

    for (int i = 0; i < 3; i++) {
        lv_obj_t *item = create_inline_row(bar, 70, 24);
        lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *dot = lv_obj_create(item);
        lv_obj_set_size(dot, 6, 6);
        lv_obj_set_style_bg_color(dot, dot_colors[i], 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(dot, 3, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
        lv_obj_set_scrollbar_mode(dot, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *lbl = lv_label_create(item);
        lv_obj_set_style_text_font(lbl, &lv_font_app_14, 0);
        lv_obj_set_style_text_color(lbl, COL_TEXT_DIM, 0);
        lv_label_set_text(lbl, items[i]);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(lbl, 56);
        lv_obj_set_style_pad_left(lbl, 4, 0);
        lv_obj_set_scrollbar_mode(lbl, LV_SCROLLBAR_MODE_OFF);
    }
    return bar;
}

/* ============== 电机概览卡 ============== */

static lv_obj_t *build_motor_card(lv_obj_t *parent,
                                  const char *title, lv_color_t accent,
                                  const char *status_text, lv_color_t status_col,
                                  uint8_t pct, bool animated_dot,
                                  uint8_t card_idx)
{
    lv_obj_t *card = create_card(parent, 232, 88, accent);

    /* 标题行 */
    lv_obj_t *title_row = create_inline_row(card, 216, 20);
    lv_obj_set_width(title_row, 216);
    lv_obj_set_flex_align(title_row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *dot = lv_obj_create(title_row);
    lv_obj_set_size(dot, 8, 8);
    lv_obj_set_style_bg_color(dot, status_col, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(dot, 4, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_scrollbar_mode(dot, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    if (card_idx == 0) g_motor.ov_stepper_dot = dot;
    else                g_motor.ov_dc_dot = dot;

    lv_obj_t *ttl = lv_label_create(title_row);
    lv_obj_set_style_text_font(ttl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(ttl, COL_TEXT, 0);
    lv_label_set_text(ttl, title);
    lv_label_set_long_mode(ttl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(ttl, 150);
    lv_obj_set_scrollbar_mode(ttl, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *stlbl = lv_label_create(title_row);
    lv_obj_set_style_text_font(stlbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(stlbl, status_col, 0);
    lv_label_set_text(stlbl, status_text);
    lv_label_set_long_mode(stlbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(stlbl, 50);
    lv_obj_set_scrollbar_mode(stlbl, LV_SCROLLBAR_MODE_OFF);

    /* 进度条 */
    lv_obj_t *bar = lv_bar_create(card);
    lv_obj_set_size(bar, 216, 8);
    lv_obj_set_style_bg_color(bar, COL_BG, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(bar, 4, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, accent, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR);
    lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);
    if (card_idx == 0) g_motor.ov_stepper_bar = bar;
    else                g_motor.ov_dc_bar = bar;

    /* 百分比 + 详情按钮 行 */
    lv_obj_t *bottom = create_inline_row(card, 216, 22);
    lv_obj_set_width(bottom, 216);
    lv_obj_set_flex_align(bottom, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    char pct_buf[16];
    snprintf(pct_buf, sizeof(pct_buf), "%d%%", pct);
    lv_obj_t *pct_lbl = lv_label_create(bottom);
    lv_obj_set_style_text_font(pct_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(pct_lbl, COL_TEXT, 0);
    lv_label_set_text(pct_lbl, pct_buf);
    lv_label_set_long_mode(pct_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(pct_lbl, 40);
    lv_obj_set_scrollbar_mode(pct_lbl, LV_SCROLLBAR_MODE_OFF);
    if (card_idx == 0) g_motor.ov_stepper_pct = pct_lbl;
    else                g_motor.ov_dc_pct = pct_lbl;

    lv_obj_t *view_btn = lv_button_create(bottom);
    lv_obj_set_size(view_btn, 80, 22);
    style_button_industrial(view_btn, accent);
    lv_obj_add_event_cb(view_btn, on_view_btn_click,
                        LV_EVENT_CLICKED, (void *)(uintptr_t)card_idx);

    lv_obj_t *vtxt = lv_label_create(view_btn);
    lv_obj_set_style_text_font(vtxt, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(vtxt, accent, 0);
    lv_label_set_text(vtxt, "查看详情");
    lv_label_set_long_mode(vtxt, LV_LABEL_LONG_CLIP);
    lv_obj_set_scrollbar_mode(vtxt, LV_SCROLLBAR_MODE_OFF);

    return card;
}

/* ============== 底部信息 ============== */

static lv_obj_t *build_footer(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 232, 28);
    lv_obj_set_style_bg_color(bar, COL_CARD, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 3, 0);
    lv_obj_set_style_pad_left(bar, 8, 0);
    lv_obj_set_style_pad_right(bar, 8, 0);
    lv_obj_set_style_pad_top(bar, 0, 0);
    lv_obj_set_style_pad_bottom(bar, 0, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *mode_lbl = lv_label_create(bar);
    lv_obj_set_style_text_font(mode_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(mode_lbl, COL_TEXT_DIM, 0);
    lv_label_set_text(mode_lbl, "模式: 自动");
    lv_label_set_long_mode(mode_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(mode_lbl, 80);
    lv_obj_set_scrollbar_mode(mode_lbl, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *rt_lbl = lv_label_create(bar);
    lv_obj_set_style_text_font(rt_lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(rt_lbl, COL_TEXT, 0);
    lv_label_set_text(rt_lbl, "运行时长 02:34:12");
    lv_label_set_long_mode(rt_lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(rt_lbl, 140);
    lv_obj_set_scrollbar_mode(rt_lbl, LV_SCROLLBAR_MODE_OFF);
    g_motor.ov_runtime_lbl = rt_lbl;

    return bar;
}

/* ============== 主构建 ============== */

void page_overview_build(lv_obj_t *parent)
{
    build_status_bar(parent);
    build_motor_card(parent, "步进 STEPPER #1", COL_AMBER,
                     "运行中", COL_GREEN, 24, true, 0);
    build_motor_card(parent, "直流 DC MOTOR #2", COL_CYAN,
                     "停止", COL_STOP, 0, false, 1);
    build_footer(parent);

    if (g_motor.ov_stepper_bar) anim_bar_to(g_motor.ov_stepper_bar, 24);
    if (g_motor.ov_dc_bar)      anim_bar_to(g_motor.ov_dc_bar, 0);
    if (g_motor.ov_stepper_dot) anim_breathing_start(g_motor.ov_stepper_dot);
}
