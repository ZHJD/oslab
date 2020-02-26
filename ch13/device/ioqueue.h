#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H

#include "stdint.h"
#include "thread.h"
#include "semaphore.h"
#include "sync.h"

#define bufsize 64

/* 环形队列 */
typedef struct ioqueue
{
    /* 缓冲区满 */
    semaphore full;

    /* 缓冲区空 */
    semaphore empty;

    /* 缓冲区互斥锁 */
    lock ioq_lock;

    char buf[bufsize];

    /* 队头进队 */
    uint32_t head; 

    /* 队尾出队 */
    uint32_t tail;
    
}ioqueue;


/**********************************************
 * 函数名: ioqueue_init()
 * ioq:循环队列
 * 功能:初始化ioq
 * 返回值:无
 */
void ioqueue_init(ioqueue* ioq);

/************************************************
 * 函数名:ioq_getchar()
 * ioq:ioqueue指针
 * 功能:从缓冲区中读出一个字符
 * 返回值:读取的字符
 */ 
char ioq_getchar(ioqueue* ioq);

/**********************************************
 * 函数名:ioq_putchar()
 * ioq:ioqueue指针
 * byte:向缓冲区中放入的一个字节
 * 功能值:向缓冲区中放入一个字符
 * 返回值:无
 */ 
void ioq_putchar(ioqueue* ioq, const char byte);

#endif
