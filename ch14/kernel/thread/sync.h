#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "list.h"
#include "stdint.h"
#include "thread.h"
#include "semaphore.h"

/* 锁结构 */
typedef struct lock
{
    /* 锁的持有者 */
    task_struct* holder;

    /* 使用二元信号量实现锁 */
    semaphore semaphore_binary;

    /* 锁的持有者重复申请锁的次数 */
    uint32_t holder_repeat_nr;
}lock;

/***************************
 * 函数:lock_init()
 * plock:lock指针
 * 功能:初始化锁
 * 返回值:无
 */
void lock_init(lock* plock);

/*******************************************
 * 函数名:lock_acquire()
 * plock lock指针
 * 功能:加锁
 * 返回值:无
 */ 
void lock_acquire(lock* plock);

/********************************************
 * 函数名:lock_release()
 * plock:lock指针
 * 功能:释放锁，唤醒阻塞线程
 * 返回值:无
 */ 
void lock_release(lock* plock);

#endif
