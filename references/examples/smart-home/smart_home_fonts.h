#ifndef SMART_HOME_FONTS_H
#define SMART_HOME_FONTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

/**
 * @brief 智能家居页面字体
 *
 * 生成方式：
 *   lv_font_conv --font SourceHanSansSC-Normal.otf
 *                --symbols "$(cat fonts/chars.txt)"
 *                --no-compress -r 0x20-0x7F
 *
 * 字符来源：fonts/chars.txt（从所有页面源码自动收集）
 */

/** 正文 16px（卡片正文、按钮、开关标签、场景描述等） */
extern const lv_font_t lv_font_smart_home_16;

/** 标题 24px（页面大标题、传感器数值强调等） */
extern const lv_font_t lv_font_smart_home_24;

#ifdef __cplusplus
}
#endif

#endif /* SMART_HOME_FONTS_H */
