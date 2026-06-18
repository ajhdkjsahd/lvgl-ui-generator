/**
 * @file main.c
 * @brief LVGL UI Preview — SDL 最小入口
 *
 * 用户只需:
 *   1. 把 UI 页面放到 src/ui/pages/*.c
 *   2. 修改下方的 #include 和 create 函数调用
 *   3. cmake --build build && ./build/main
 */

#include <stdlib.h>
#include <unistd.h>
#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"

/* =============================================
 *   引入你的 UI 页面
 * ============================================= */
/* TODO: 改为你自己的页面头文件 */
/* #include "src/ui/pages/your_page.h" */
/* #include "src/ui/fonts/app_fonts.h" */

/* =============================================
 *   分辨率 (可改为你的目标屏幕尺寸)
 * ============================================= */
#define SCR_W  800
#define SCR_H  480

int main(int argc, char **argv)
{
    (void)argc; (void)argv;

    lv_init();

    /* ---- SDL 显示 + 输入 ---- */
    lv_display_t * disp = lv_sdl_window_create(SCR_W, SCR_H);
    lv_indev_t * mouse = lv_sdl_mouse_create();
    lv_indev_set_display(mouse, disp);
    lv_indev_t * mw = lv_sdl_mousewheel_create();
    lv_indev_set_display(mw, disp);
    lv_indev_t * kb = lv_sdl_keyboard_create();
    lv_indev_set_display(kb, disp);

    /* ---- 可选: 主题 ---- */
    /* lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE),
                            lv_palette_main(LV_PALETTE_GREY), true,
                            &lv_font_montserrat_16); */

    /* ---- 加载你的 UI ---- */
    /* TODO: 改为你自己的页面 */
    /* lv_obj_t * scr = your_page_create(); */
    /* lv_screen_load(scr); */

    /* 临时: 显示 LVGL 官方 demo */
    lv_demo_widgets();

    /* ---- 主循环 ---- */
    while (1) {
        uint32_t ms = lv_timer_handler();
        if (ms == LV_NO_TIMER_READY) ms = LV_DEF_REFR_PERIOD;
        usleep(ms * 1000);
    }
    return 0;
}
