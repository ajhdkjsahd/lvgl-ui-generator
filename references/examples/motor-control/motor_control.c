/**
 * @file motor_control.c
 * @brief 主屏幕骨架: 顶栏 + 内容区(3 个 page) + 底栏 Tab 导航
 */

#include "motor_control.h"
#include "page_overview.h"
#include "page_stepper.h"
#include "page_dc.h"

/* ============== 全局状态 ============== */
motor_ctrl_t g_motor;

/* ============== 工厂函数实现 ============== */

lv_obj_t *create_inline_row(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, w, h);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_gap(row, 4, 0);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE);  /* 行容器不拦截点击, 让事件穿透到父对象 */
    return row;
}

lv_obj_t *create_card(lv_obj_t *parent, lv_coord_t w, lv_coord_t h, lv_color_t border)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, COL_CARD, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, border, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 4, 0);
    lv_obj_set_style_pad_all(card, 8, 0);
    lv_obj_set_style_pad_gap(card, 4, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

lv_obj_t *create_label(lv_obj_t *parent, const char *txt,
                       const lv_font_t *font, lv_color_t color, lv_coord_t w)
{
    lv_obj_t *lbl = lv_label_create(parent);
    if (font) lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    lv_label_set_text(lbl, txt);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    if (w > 0) lv_obj_set_width(lbl, w);
    lv_obj_set_scrollbar_mode(lbl, LV_SCROLLBAR_MODE_OFF);
    return lbl;
}

lv_obj_t *create_chip(lv_obj_t *parent, const char *txt, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *chip = lv_obj_create(parent);
    lv_obj_set_size(chip, w, h);
    lv_obj_set_style_bg_color(chip, COL_CARD_HI, 0);
    lv_obj_set_style_bg_opa(chip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(chip, COL_BORDER_HI, 0);
    lv_obj_set_style_border_width(chip, 1, 0);
    lv_obj_set_style_radius(chip, 3, 0);
    lv_obj_set_style_pad_all(chip, 0, 0);
    lv_obj_set_flex_flow(chip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chip, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(chip, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(chip, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(chip);
    lv_obj_set_style_text_font(lbl, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(lbl, COL_TEXT_DIM, 0);
    lv_label_set_text(lbl, txt);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    lv_obj_set_scrollbar_mode(lbl, LV_SCROLLBAR_MODE_OFF);
    return chip;
}

void style_button_industrial(lv_obj_t *btn, lv_color_t accent)
{
    lv_obj_set_style_bg_color(btn, COL_CARD_HI, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn, accent, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_radius(btn, 3, 0);
    lv_obj_set_style_pad_all(btn, 4, 0);
    lv_obj_set_style_pad_gap(btn, 2, 0);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_shadow_color(btn, lv_color_black(), 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    /* 按下态: 描边变亮 + 阴影内陷 */
    lv_obj_set_style_border_color(btn, COL_BORDER_HI, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn, COL_CARD, LV_STATE_PRESSED);
    lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_ADV_HITTEST);
}

/* ============== 动画: 状态点呼吸 ============== */

static void breathing_anim_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, v, 0);
}

void anim_breathing_start(lv_obj_t *obj)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, LV_OPA_30, LV_OPA_COVER);
    lv_anim_set_time(&a, 800);
    lv_anim_set_playback_time(&a, 800);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, breathing_anim_cb);
    lv_anim_start(&a);
}

/* ============== 动画: 进度条弹性 ============== */

static void bar_anim_cb(void *bar, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)bar, v, LV_ANIM_OFF);
}

void anim_bar_to(lv_obj_t *bar, int32_t target_pct)
{
    if (target_pct < 0) target_pct = 0;
    if (target_pct > 100) target_pct = 100;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bar);
    lv_anim_set_values(&a, lv_bar_get_value(bar), target_pct);
    lv_anim_set_time(&a, 500);
    lv_anim_set_exec_cb(&a, bar_anim_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

/* ============== Tab 切换 ============== */

static void on_tab_click(lv_event_t *e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);
    switch_to_tab((uint8_t)idx);
}

void switch_to_tab(uint8_t tab_idx)
{
    if (tab_idx > 2) return;
    if (tab_idx == g_motor.current_tab) return;

    uint8_t old = g_motor.current_tab;

    /* 旧页面淡出 */
    lv_obj_fade_out(g_motor.pages[old], 200, 0);
    /* 新页面淡入 (从 0 开始) */
    lv_obj_set_style_opa(g_motor.pages[tab_idx], LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(g_motor.pages[tab_idx], LV_OBJ_FLAG_HIDDEN);
    lv_obj_fade_in(g_motor.pages[tab_idx], 200, 0);

    /* 旧按钮取消高亮, 新按钮高亮 */
    for (int i = 0; i < 3; i++) {
        lv_color_t col = (i == tab_idx) ? COL_CYAN : COL_TEXT_DIM;
        lv_obj_set_style_text_color(lv_obj_get_child(g_motor.tab_btns[i], 0), col, 0);
        lv_obj_set_style_border_color(g_motor.tab_btns[i],
                                      (i == tab_idx) ? COL_CYAN : COL_BORDER, 0);
        lv_obj_set_style_border_width(g_motor.tab_btns[i],
                                      (i == tab_idx) ? 1 : 0, 0);
    }

    /* 顶栏标题切换 */
    static const char *titles[3] = { "电机控制中心", "步进电机", "直流电机" };
    lv_label_set_text(g_motor.title_label, titles[tab_idx]);

    /* 隐藏旧 page 完成后清理 (200ms 后) */
    lv_obj_add_flag(g_motor.pages[old], LV_OBJ_FLAG_HIDDEN);

    g_motor.current_tab = tab_idx;
}

/* ============== 顶栏 ============== */

static lv_obj_t *build_top_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 240, 30);
    lv_obj_set_style_bg_color(bar, COL_CARD, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(bar, COL_BORDER, 0);
    lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(bar, 8, 0);
    lv_obj_set_style_pad_right(bar, 8, 0);
    lv_obj_set_style_pad_top(bar, 0, 0);
    lv_obj_set_style_pad_bottom(bar, 0, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    /* 左侧: 齿轮图标 + 标题 */
    lv_obj_t *left = create_inline_row(bar, 160, 30);
    lv_obj_t *icon = lv_label_create(left);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(icon, COL_CYAN, 0);
    lv_label_set_text(icon, LV_SYMBOL_SETTINGS);
    lv_obj_set_scrollbar_mode(icon, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *title = lv_label_create(left);
    lv_obj_set_style_text_font(title, &lv_font_app_14, 0);
    lv_obj_set_style_text_color(title, COL_TEXT, 0);
    lv_label_set_text(title, "电机控制中心");
    lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);
    lv_obj_set_width(title, 130);
    lv_obj_set_scrollbar_mode(title, LV_SCROLLBAR_MODE_OFF);
    g_motor.title_label = title;

    /* 右侧: 电量 + 时间 */
    lv_obj_t *right = create_inline_row(bar, 70, 30);
    lv_obj_t *bat = lv_label_create(right);
    lv_obj_set_style_text_font(bat, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(bat, COL_GREEN, 0);
    lv_label_set_text(bat, LV_SYMBOL_CHARGE);
    lv_obj_set_scrollbar_mode(bat, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *time = lv_label_create(right);
    lv_obj_set_style_text_font(time, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(time, COL_TEXT_DIM, 0);
    lv_label_set_text(time, "09:42");
    lv_obj_set_scrollbar_mode(time, LV_SCROLLBAR_MODE_OFF);

    return bar;
}

/* ============== 底栏 Tab ============== */

static lv_obj_t *build_tab_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 240, 36);
    lv_obj_set_style_bg_color(bar, COL_CARD, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(bar, COL_BORDER, 0);
    lv_obj_set_style_pad_all(bar, 4, 0);
    lv_obj_set_style_pad_gap(bar, 4, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    static const char *icons[3]  = { LV_SYMBOL_LIST, LV_SYMBOL_LOOP, LV_SYMBOL_POWER };
    static const char *labels[3] = { "概览", "步进", "直流" };

    for (int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_button_create(bar);
        lv_obj_set_size(btn, 70, 28);
        style_button_industrial(btn, COL_BORDER);
        lv_obj_set_style_border_width(btn, (i == 0) ? 1 : 0, 0);
        lv_obj_set_style_border_color(btn, (i == 0) ? COL_CYAN : COL_BORDER, 0);
        lv_obj_add_event_cb(btn, on_tab_click, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        lv_obj_t *inner = create_inline_row(btn, 64, 20);
        lv_obj_t *ic = lv_label_create(inner);
        lv_obj_set_style_text_font(ic, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(ic, (i == 0) ? COL_CYAN : COL_TEXT_DIM, 0);
        lv_label_set_text(ic, icons[i]);
        lv_obj_set_scrollbar_mode(ic, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t *tx = lv_label_create(inner);
        lv_obj_set_style_text_font(tx, &lv_font_app_14, 0);
        lv_obj_set_style_text_color(tx, (i == 0) ? COL_CYAN : COL_TEXT_DIM, 0);
        lv_label_set_text(tx, labels[i]);
        lv_label_set_long_mode(tx, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(tx, 36);
        lv_obj_set_scrollbar_mode(tx, LV_SCROLLBAR_MODE_OFF);

        g_motor.tab_btns[i] = btn;
    }
    return bar;
}

/* ============== 主入口 ============== */

lv_obj_t *motor_control_create(void)
{
    /* 初始化业务状态默认值 */
    g_motor.stepper.dir = MOTOR_STOP;
    g_motor.stepper.enable = false;
    g_motor.stepper.speed_hz = 2000;
    g_motor.stepper.target_steps = 500;
    g_motor.stepper.position = 0;
    g_motor.stepper.subdiv = STEPPER_SUBDIV_8;
    g_motor.stepper.running = false;

    g_motor.dc.dir = MOTOR_STOP;
    g_motor.dc.enable = false;
    g_motor.dc.pwm_pct = 50;
    g_motor.dc.voltage_v = 12.3f;
    g_motor.dc.current_a = 0.0f;
    g_motor.dc.prot_overcurrent = false;
    g_motor.dc.prot_stall = false;
    g_motor.dc.brake = false;

    g_motor.current_tab = 0;

    /* 主屏幕: 不使用 flex, 子对象手动绝对定位, 避免 lvgl flex 行为差异 */
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, 240, 320);
    lv_obj_set_style_bg_color(scr, COL_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_style_border_width(scr, 0, 0);
    /* 关键: 不设 flex_flow, 用 absolute 定位 */
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    g_motor.scr = scr;

    /* 顶栏 y=0 h=30 */
    g_motor.top_bar = build_top_bar(scr);
    lv_obj_set_pos(g_motor.top_bar, 0, 0);

    /* 内容区 y=30 h=254 */
    g_motor.content = lv_obj_create(scr);
    lv_obj_set_size(g_motor.content, 240, 254);
    lv_obj_set_pos(g_motor.content, 0, 30);
    lv_obj_set_style_bg_opa(g_motor.content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_motor.content, 0, 0);
    lv_obj_set_style_pad_all(g_motor.content, 0, 0);
    lv_obj_set_scrollbar_mode(g_motor.content, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(g_motor.content, LV_OBJ_FLAG_SCROLLABLE);

    /* 三个 page 容器 (absolute 定位叠放, 通过 HIDDEN 切换) */
    for (int i = 0; i < 3; i++) {
        lv_obj_t *page = lv_obj_create(g_motor.content);
        lv_obj_set_size(page, 240, 254);
        lv_obj_set_pos(page, 0, 0);
        lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(page, 0, 0);
        lv_obj_set_style_pad_all(page, 4, 0);
        lv_obj_set_style_pad_gap(page, 6, 0);
        lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(page, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
        if (i != 0) lv_obj_add_flag(page, LV_OBJ_FLAG_HIDDEN);
        g_motor.pages[i] = page;
    }

    /* 填充三个页面内容 */
    page_overview_build(g_motor.pages[0]);
    page_stepper_build(g_motor.pages[1]);
    page_dc_build(g_motor.pages[2]);

    /* 底栏 y=284 h=36 */
    g_motor.tab_bar = build_tab_bar(scr);
    lv_obj_set_pos(g_motor.tab_bar, 0, 284);

    /* 启动状态点呼吸动画 */
    if (g_motor.ov_stepper_dot) anim_breathing_start(g_motor.ov_stepper_dot);

    return scr;
}
