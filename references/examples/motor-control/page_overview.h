/**
 * @file page_overview.h
 * @brief 概览页: 系统状态 + 两电机卡片
 */

#ifndef PAGE_OVERVIEW_H
#define PAGE_OVERVIEW_H

#include "motor_control.h"

/* 在主屏幕调用, 创建概览页内容到 parent */
void page_overview_build(lv_obj_t *parent);

#endif
