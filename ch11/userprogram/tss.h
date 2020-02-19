#ifndef __USERPROGRAM__TSS_H
#define __USERPROGRAM__TSS_H
#include "thread.h"

/******************************************************
 * 函数名:update_tss_esp0()
 * pthread:线程pcb起始地址
 * 功能:设置esp0的值
 * 返回值:无
 */ 
void update_tss_esp0(task_struct* pthread);

/************************************************************
 * 函数名:tss_init()
 * 功能:添加用户层段描述符并初始化tss
 * 返回值:无
 */ 
void tss_init();

#endif
