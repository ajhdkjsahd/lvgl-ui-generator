#include "smart_home.h"

/* ==================== 样式 ==================== */
static lv_style_t style_scene_card;
static lv_style_t style_scene_active;

/* 场景对应的图标和描述 */
static const char * scene_symbols[] = {
    LV_SYMBOL_HOME,       /* 回家 */
    LV_SYMBOL_POWER,      /* 离家 */
    LV_SYMBOL_MUTE,       /* 睡眠（月亮≈静音） */
    LV_SYMBOL_PLAY,       /* 观影 */
    LV_SYMBOL_FILE,       /* 阅读 */
    LV_SYMBOL_OK,         /* 用餐 */
    LV_SYMBOL_UP,         /* 起床 */
    LV_SYMBOL_CHARGE      /* 节能 */
};

static const char * scene_descs[] = {
    "所有设备恢复默认",
    "关闭所有设备\n启动安防",
    "关闭灯光窗帘\n调低温度",
    "关闭窗帘\n调暗灯光",
    "打开阅读灯\n关闭娱乐设备",
    "打开餐厅灯\n关闭客厅设备",
    "打开窗帘\n调亮灯光",
    "关闭高功耗设备\n节能模式"
};

/* 当前激活场景（-1 = 无） */
static int active_scene = -1;
static lv_obj_t * scene_buttons[SCENE_COUNT];

/* ==================== 回调 ==================== */
static void on_scene_activate(lv_event_t * e);

/* ==================== 场景页面创建 ==================== */
void scene_page_create(lv_obj_t * parent)
{
    /* ---- 初始化样式 ---- */
    lv_style_init(&style_scene_card);
    lv_style_set_bg_color(&style_scene_card, lv_color_hex(COLOR_CARD_BG));
    lv_style_set_bg_opa(&style_scene_card, LV_OPA_COVER);
    lv_style_set_radius(&style_scene_card, 12);
    lv_style_set_shadow_width(&style_scene_card, 3);
    lv_style_set_shadow_color(&style_scene_card, lv_color_hex(0xBBBBBB));
    lv_style_set_shadow_ofs_y(&style_scene_card, 2);
    lv_style_set_pad_all(&style_scene_card, 10);
    lv_style_set_pad_row(&style_scene_card, 4);
    lv_style_set_border_width(&style_scene_card, 2);
    lv_style_set_border_color(&style_scene_card, lv_color_hex(0xE0E0E0));

    lv_style_init(&style_scene_active);
    lv_style_set_border_color(&style_scene_active, lv_color_hex(COLOR_PRIMARY));
    lv_style_set_border_width(&style_scene_active, 3);
    lv_style_set_bg_color(&style_scene_active, lv_color_hex(0xE3F2FD));

    /* ---- 父容器布局：禁止滚动 ---- */
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(parent, 8, 0);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);

    /* 标题行：图标 + 中文分开，图标用 montserrat 字体 */
    lv_obj_t * title_row = create_inline_row(parent, 776, 36);
    lv_obj_set_style_pad_top(title_row, 4, 0);
    lv_obj_set_style_pad_bottom(title_row, 4, 0);
    lv_obj_set_flex_align(title_row, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * title_icon = lv_label_create(title_row);
    lv_obj_set_style_text_font(title_icon, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title_icon, lv_color_hex(COLOR_PRIMARY), 0);
    lv_label_set_text(title_icon, LV_SYMBOL_PLAY);

    lv_obj_t * title_text = lv_label_create(title_row);
    lv_obj_set_style_text_font(title_text, &lv_font_smart_home_24, 0);
    lv_obj_set_style_text_color(title_text, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_style_pad_left(title_text, 8, 0);
    lv_label_set_text(title_text, "选择场景 一键切换智能生活");

    /* ---- 场景网格 (2行×4列) ---- */
    lv_obj_t * grid = lv_obj_create(parent);
    lv_obj_set_size(grid, 776, 330);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 8, 0);
    lv_obj_set_scrollbar_mode(grid, LV_SCROLLBAR_MODE_OFF);

    /* Grid 布局：4列均分，2行等高 */
    static int32_t col_dsc[] = {
        LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };
    static int32_t row_dsc[] = {
        LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);

    for(int i = 0; i < SCENE_COUNT; i++) {
        lv_obj_t * card = lv_obj_create(grid);
        lv_obj_add_style(card, &style_scene_card, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);
        /* 关键：禁止 card 内部滚动 */
        lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

        /* Grid 定位 */
        int col = i % 4;
        int row = i / 4;
        lv_obj_set_grid_cell(card, LV_GRID_ALIGN_STRETCH, col, 1,
                                    LV_GRID_ALIGN_STRETCH, row, 1);

        /* 图标（LVGL 符号字体） */
        lv_obj_t * emoji = lv_label_create(card);
        lv_obj_set_style_text_font(emoji, &lv_font_montserrat_20, 0);
        lv_label_set_text(emoji, scene_symbols[i]);

        /* 场景名称 */
        lv_obj_t * name = lv_label_create(card);
        lv_obj_set_style_text_font(name, &lv_font_smart_home_16, 0);
        lv_obj_set_style_text_color(name, lv_color_hex(COLOR_TEXT), 0);
        lv_label_set_long_mode(name, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(name, LV_PCT(100));
        lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(name, g_scene_names[i]);

        /* 场景描述：用 SCROLL_CIRCULAR + 1 行高度限制 + 居中，
           文字超出宽度直接截断，绝不允许出现滚动条 */
        lv_obj_t * desc = lv_label_create(card);
        lv_obj_set_style_text_font(desc, &lv_font_smart_home_16, 0);
        lv_obj_set_style_text_color(desc, lv_color_hex(COLOR_TEXT_SEC), 0);
        lv_label_set_long_mode(desc, LV_LABEL_LONG_CLIP);  /* 截断而非换行 */
        lv_obj_set_width(desc, LV_PCT(100));
        lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(desc, scene_descs[i]);

        lv_obj_add_event_cb(card, on_scene_activate, LV_EVENT_CLICKED,
                            (void *)(uintptr_t)i);
        scene_buttons[i] = card;
    }
}

/* ==================== 回调函数 ==================== */

static void on_scene_activate(lv_event_t * e)
{
    uintptr_t idx = (uintptr_t)lv_event_get_user_data(e);

    /* 取消上一个激活场景的样式 */
    if(active_scene >= 0 && scene_buttons[active_scene]) {
        lv_obj_remove_style(scene_buttons[active_scene],
                            &style_scene_active, 0);
    }

    /* 设置新激活场景 */
    if((int)idx != active_scene) {
        active_scene = (int)idx;
        lv_obj_add_style(scene_buttons[active_scene],
                         &style_scene_active, 0);
        LV_LOG_USER("场景 [%s] 已激活", g_scene_names[idx]);

        /* 根据场景预设更新设备状态 */
        switch(idx) {
        case 0: /* 回家 */
            g_home_state.living_light = true;
            g_home_state.living_light_brightness = 80;
            g_home_state.living_curtain = true;
            g_home_state.living_curtain_pos = 100;
            g_home_state.ac_power = true;
            g_home_state.ac_temp = 26;
            break;
        case 1: /* 离家 */
            g_home_state.living_light = false;
            g_home_state.bedroom_light = false;
            g_home_state.kitchen_light = false;
            g_home_state.living_curtain = false;
            g_home_state.living_curtain_pos = 0;
            g_home_state.ac_power = false;
            g_home_state.outlet = false;
            g_home_state.water_heater = false;
            break;
        case 2: /* 睡眠 */
            g_home_state.living_light = false;
            g_home_state.bedroom_light = false;
            g_home_state.living_curtain = false;
            g_home_state.ac_temp = 24;
            g_home_state.ac_fan = 1;  /* 低风 */
            break;
        case 3: /* 观影 */
            g_home_state.living_light = true;
            g_home_state.living_light_brightness = 20;
            g_home_state.living_curtain = false;
            g_home_state.living_curtain_pos = 0;
            break;
        case 4: /* 阅读 */
            g_home_state.living_light = true;
            g_home_state.living_light_brightness = 90;
            break;
        case 5: /* 用餐 */
            g_home_state.living_light = true;
            g_home_state.living_light_brightness = 60;
            break;
        case 6: /* 起床 */
            g_home_state.living_curtain = true;
            g_home_state.living_curtain_pos = 100;
            g_home_state.living_light = true;
            g_home_state.living_light_brightness = 70;
            break;
        case 7: /* 节能 */
            g_home_state.ac_temp = 27;
            g_home_state.water_heater = false;
            g_home_state.living_light_brightness = 40;
            break;
        }
    } else {
        /* 再次点击取消激活 */
        active_scene = -1;
        LV_LOG_USER("场景已取消");
    }
}
