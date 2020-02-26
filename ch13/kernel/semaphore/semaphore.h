#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H
#include "list.h"
#include "thread.h"


/* 信号量结构 */
typedef struct  semaphore
{
    /* 资源值 */
    int value;

    /* 等待该资源的队列 */
    list waiters;
}semaphore;

/**************************
 * 函数名:sema_init()
 * psema:信号量指针
 * value:信号量初值
 * 功能:初始化信号量值和等待队列
 * 返回值:无
 */ 
void sema_init(semaphore* psema, int value);

/**************************************
 * 函数名:sema_down()
 * psema:semaphore指针
 * 功能:p操作
 * 返回值:无
 */ 
void sema_down(semaphore* psema);

/********************************************
 * 函数名:sema_up()
 * psema:semaphore
 * 功能:唤醒被阻塞的线程
 * 返回值:无
 */ 
void sema_up(semaphore* psema);



#endif
