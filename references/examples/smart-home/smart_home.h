#ifndef SMART_HOME_H
#define SMART_HOME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

/* ==================== 自定义中文字体（lv_font_conv 生成） ==================== */
#include "src/ui/fonts/smart_home_fonts.h"

/* ==================== 主题颜色 ==================== */
#define COLOR_BG            0xF0F4F8  /* 页面背景 */
#define COLOR_PRIMARY       0x2196F3  /* 主题蓝 */
#define COLOR_CARD_BG       0xFFFFFF  /* 卡片背景 */
#define COLOR_TEXT          0x333333  /* 主文字 */
#define COLOR_TEXT_SEC      0x888888  /* 次要文字 */
#define COLOR_TEMP          0xFF9800  /* 温度橙 */
#define COLOR_HUMIDITY      0x00BCD4  /* 湿度青 */
#define COLOR_LIGHT_SENSOR  0xFFC107  /* 光照黄 */
#define COLOR_PM25          0x9C27B0  /* PM2.5紫 */
#define COLOR_ON            0x4CAF50  /* 开启绿 */
#define COLOR_OFF           0xF44336  /* 关闭红 */
#define COLOR_SCENE_BTN     0x607D8B  /* 场景按钮 */

/* ==================== 设备状态 ==================== */
typedef struct {
    /* 灯光 */
    bool living_light;
    int  living_light_brightness;   /* 0-100 */
    bool bedroom_light;
    int  bedroom_light_brightness;  /* 0-100 */
    bool kitchen_light;

    /* 窗帘 */
    bool living_curtain;            /* true=打开 */
    int  living_curtain_pos;        /* 0-100, 0=全关 */
    bool bedroom_curtain;
    int  bedroom_curtain_pos;

    /* 空调 */
    bool ac_power;
    int  ac_temp;                   /* 16-30 */
    int  ac_mode;                   /* 0=制冷, 1=制热, 2=通风, 3=除湿 */
    int  ac_fan;                    /* 0=自动, 1=低, 2=中, 3=高 */

    /* 其他 */
    bool outlet;
    bool water_heater;
} smart_home_state_t;

/* ==================== 传感器数据 ==================== */
typedef struct {
    float temperature;      /* °C */
    int   humidity;         /* %RH */
    int   light_lux;        /* lux */
    int   pm25;             /* μg/m³ */
    int   battery;          /* % */
} sensor_data_t;

/* ==================== 全局状态（供各页面读写） ==================== */
extern smart_home_state_t g_home_state;
extern sensor_data_t       g_sensor_data;
extern const char *        g_ac_mode_names[];
extern const char *        g_ac_fan_names[];
extern const char *        g_scene_names[];

#define SCENE_COUNT 8

/* ==================== 页面创建函数 ==================== */

/**
 * @brief 创建智能家居主界面（含底部Tab导航）
 * @return 屏幕对象指针
 */
lv_obj_t * smart_home_create(void);

/**
 * @brief 创建首页Tab内容
 * @param parent 父容器（tab页面）
 */
void home_page_create(lv_obj_t * parent);

/**
 * @brief 创建设备控制Tab内容
 * @param parent 父容器（tab页面）
 */
void device_page_create(lv_obj_t * parent);

/**
 * @brief 创建场景Tab内容
 * @param parent 父容器（tab页面）
 */
void scene_page_create(lv_obj_t * parent);

/* ==================== 工具函数 ==================== */

/**
 * @brief 创建带图标的区域标题（图标+中文分开，分别使用 montserrat 和中文字体）
 * @param parent 父容器
 * @param icon   LV_SYMBOL_* 图标字符串
 * @param text   标题文本（中文）
 * @return 行容器对象
 */
lv_obj_t * create_section_label(lv_obj_t * parent, const char * icon, const char * text);

/**
 * @brief 创建带开关和标签的设备控制行
 * @param parent    父容器
 * @param label     设备名称
 * @param init_state 初始开关状态
 * @param cb        开关切换回调（可为NULL）
 * @return 开关对象
 */
lv_obj_t * create_switch_row(lv_obj_t * parent, const char * label,
                              bool init_state, lv_event_cb_t cb);

/**
 * @brief 创建带滑块的设备控制行
 * @param parent    父容器
 * @param label     设备名称
 * @param min       最小值
 * @param max       最大值
 * @param init_val  初始值
 * @param cb        滑块变化回调
 * @return 滑块对象
 */
lv_obj_t * create_slider_row(lv_obj_t * parent, const char * label,
                              int min, int max, int init_val,
                              lv_event_cb_t cb);

/**
 * @brief 创建一个透明的水平行容器（已禁用滚动、已设 Flex 布局）
 *
 * 适用于卡片内部的子行：开关/按钮/标签放在同一行。
 * 默认 align 为 START + CENTER，调用方可继续用 lv_obj_set_flex_align 调整。
 *
 * @param parent 父容器
 * @param w      宽度（像素或 LV_PCT）
 * @param h      高度（像素或 LV_PCT 或 LV_SIZE_CONTENT）
 * @return 行容器
 */
lv_obj_t * create_inline_row(lv_obj_t * parent, lv_coord_t w, lv_coord_t h);

#ifdef __cplusplus
}
#endif

#endif /* SMART_HOME_H */
