#ifndef __USERPROG_PROCESS_H
#define __USERPROG_PROCESS_H

#include "thread.h"
/* 申请内存时使用的时当前页面的最低地址值 */
#define USER_STACK3_VADDR (0xc0000000 -  0x1000)

/* linux 用户程序入口地址 */
#define USER_VADDR_START 0x8048000

/* 用户进程默认优先级 */
#define default_prio     1

/********************************************************************
 * 函数名:process_activate()
 * pthread:pcb指针
 * 功能:激活进程或者线程的页目录表，更新tss中的esp0位进程的特权级0的z栈
 * 返回值:无 
 */ 
void process_activate(task_struct* pthread);

/******************************************
 * 函数名:vreate_page_dir()
 * 功能:在内核空间创建用户进程的页目录表，
 * 第768 - 1022项指向内核空间
 * 返回值:页表虚拟地址
 */ 
uint32_t* create_page_dir(void);

/*********************************************************
 * 函数名:process_execute()
 * filename: 用户程序文件名
 * name:线程名字
 * 功能:创建进程
 * 返回值:无
 */ 
void process_execute(void* filename, char* name);

#endif
