/**
 * @file motor_control.h
 * @brief 电机控制中心 - 240x320 竖屏 LVGL v9 UI
 *
 * 结构:
 *   屏幕 240x320
 *     ├─ 顶栏     240x30   (y=0..30)
 *     ├─ 内容区   240x254  (y=30..284)
 *     │    ├─ page_overview   概览页
 *     │    ├─ page_stepper    步进电机页
 *     │    └─ page_dc         直流电机页
 *     └─ 底栏     240x36   (y=284..320)  Tab 切换
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "lvgl/lvgl.h"

/* ============== 色板 (工业深色金属风) ============== */
#define COL_BG          lv_color_hex(0x0F1419)  /* 主背景近黑     */
#define COL_CARD        lv_color_hex(0x1A1F2A)  /* 卡片底色       */
#define COL_CARD_HI     lv_color_hex(0x232938)  /* 卡片高亮/按下  */
#define COL_BORDER      lv_color_hex(0x2A3340)  /* 卡片描边       */
#define COL_BORDER_HI   lv_color_hex(0x3A4555)  /* 描边高亮       */
#define COL_TEXT        lv_color_hex(0xE1E7F0)  /* 主文字         */
#define COL_TEXT_DIM    lv_color_hex(0x8A94A6)  /* 次文字         */
#define COL_TEXT_MUTED  lv_color_hex(0x556070)  /* 辅助文字       */
#define COL_CYAN        lv_color_hex(0x00E5FF)  /* 强调青(直流)   */
#define COL_AMBER       lv_color_hex(0xFFB300)  /* 强调橙黄(步进) */
#define COL_GREEN       lv_color_hex(0x00E676)  /* 运行中         */
#define COL_RED         lv_color_hex(0xFF5252)  /* 故障/急停      */
#define COL_STOP        lv_color_hex(0x455A64)  /* 停止灰         */

/* ============== 字体 (在 src/ui/fonts/app_fonts.h 声明) ============== */
extern const lv_font_t lv_font_app_14;
extern const lv_font_t lv_font_app_20;

/* ============== 类型 ============== */
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_CW   = 1,   /* 正转 / 顺时针 */
    MOTOR_CCW  = 2,   /* 反转 / 逆时针 */
} motor_dir_t;

typedef enum {
    STEPPER_SUBDIV_1  = 1,
    STEPPER_SUBDIV_2  = 2,
    STEPPER_SUBDIV_4  = 4,
    STEPPER_SUBDIV_8  = 8,
    STEPPER_SUBDIV_16 = 16,
    STEPPER_SUBDIV_32 = 32,
} stepper_subdiv_t;

/* 步进电机状态 */
typedef struct {
    motor_dir_t       dir;            /* 当前方向        */
    bool              enable;         /* 使能            */
    uint16_t          speed_hz;       /* 脉冲频率 0..10000 */
    uint32_t          target_steps;   /* 目标步数 0..10000  */
    int32_t           position;       /* 当前位置        */
    stepper_subdiv_t  subdiv;         /* 细分 1/2/4/8/16/32 */
    bool              running;        /* 是否在跑        */
} stepper_state_t;

/* 直流电机状态 */
typedef struct {
    motor_dir_t dir;            /* 当前方向    */
    bool        enable;         /* 使能        */
    uint8_t     pwm_pct;        /* PWM 0..100  */
    float       voltage_v;      /* 电压 V      */
    float       current_a;      /* 电流 A      */
    bool        prot_overcurrent;/* 过流保护   */
    bool        prot_stall;      /* 堵转保护   */
    bool        brake;           /* 制动       */
} dc_state_t;

/* 全局 UI 状态 */
typedef struct {
    /* 容器 */
    lv_obj_t *scr;             /* 主屏幕         */
    lv_obj_t *top_bar;         /* 顶栏           */
    lv_obj_t *title_label;     /* 顶栏标题(动态) */
    lv_obj_t *content;         /* 内容区         */
    lv_obj_t *tab_bar;         /* 底栏           */
    lv_obj_t *tab_btns[3];     /* 3 个 Tab 按钮  */
    lv_obj_t *pages[3];        /* 3 个 page 容器 */
    uint8_t   current_tab;     /* 当前 tab 索引  */

    /* 业务状态 */
    stepper_state_t stepper;
    dc_state_t      dc;

    /* 概览页引用 (用于跨页更新) */
    lv_obj_t *ov_stepper_dot;
    lv_obj_t *ov_stepper_pct;
    lv_obj_t *ov_stepper_bar;
    lv_obj_t *ov_dc_dot;
    lv_obj_t *ov_dc_pct;
    lv_obj_t *ov_dc_bar;
    lv_obj_t *ov_runtime_lbl;

    /* 步进页引用 */
    lv_obj_t *sp_dot;
    lv_obj_t *sp_speed_lbl;
    lv_obj_t *sp_speed_slider;
    lv_obj_t *sp_steps_lbl;
    lv_obj_t *sp_steps_slider;
    lv_obj_t *sp_pos_lbl;
    lv_obj_t *sp_subdiv_chips[6];
    lv_obj_t *sp_enable_btn;

    /* 直流页引用 */
    lv_obj_t *dc_dot;
    lv_obj_t *dc_pwm_lbl;
    lv_obj_t *dc_pwm_slider;
    lv_obj_t *dc_v_lbl;
    lv_obj_t *dc_i_lbl;
    lv_obj_t *dc_p_lbl;
    lv_obj_t *dc_prot_chips[3];
    lv_obj_t *dc_enable_btn;
} motor_ctrl_t;

extern motor_ctrl_t g_motor;

/* ============== 工厂函数 ============== */

/* 创建无 padding 横向 row, 默认禁滚动 + 不拦截点击 */
lv_obj_t *create_inline_row(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);

/* 创建工业风卡片 (深灰底 + 描边 + 圆角 4) */
lv_obj_t *create_card(lv_obj_t *parent, lv_coord_t w, lv_coord_t h, lv_color_t border);

/* 创建固定宽度的 label, 默认 LONG_CLIP 避免溢出 */
lv_obj_t *create_label(lv_obj_t *parent, const char *txt,
                       const lv_font_t *font, lv_color_t color, lv_coord_t w);

/* 创建 chip (细分/保护选项) */
lv_obj_t *create_chip(lv_obj_t *parent, const char *txt,
                      lv_coord_t w, lv_coord_t h);

/* 工业风按钮: 透明底 + 描边 + accent 色 */
void style_button_industrial(lv_obj_t *btn, lv_color_t accent);

/* ============== 主入口 ============== */

/* 创建并返回主屏幕对象, 调用方再 lv_screen_load() */
lv_obj_t *motor_control_create(void);

/* ============== Tab 切换 (带 200ms 淡入淡出) ============== */
void switch_to_tab(uint8_t tab_idx);

/* ============== 动画 ============== */

/* 状态点呼吸: alpha 在 0.3..1.0 间循环, 周期 800ms */
void anim_breathing_start(lv_obj_t *obj);

/* 进度条弹性动画: 从当前值动画到 target_pct(0..100), 500ms */
void anim_bar_to(lv_obj_t *bar, int32_t target_pct);

#endif /* MOTOR_CONTROL_H */
