#ifndef __DEVICE_TIMER_H
#define __DEVICE_TIMER_H
#include "stdint.h"

/* 以ms为单位的顺眠函数  */
void mtime_sleep(uint32_t m_seconds);


/************************************************************
 * 函数名：timer_init()
 * 功能：初始化PIT8253
 * 返回值：无
 */
void timer_init();

#endif
