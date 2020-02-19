#ifndef __USERPROG_PROCESS_H
#define __USERPROG_PROCESS_H
#include "thread.h"

/* 申请内存时使用的时当前页面的最低地址值 */
#define USER_STACK3_VADDR (0xc0000000 - 0x1000)

/********************************************************************
 * 函数名:process_activate()
 * pthread:pcb指针
 * 功能:激活进程或者线程的页目录表，更新tss中的esp0位进程的特权级0的z栈
 * 返回值:无 
 */ 
void process_activate(task_struct* pthread);



#endif
