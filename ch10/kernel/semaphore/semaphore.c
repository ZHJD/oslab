#include "semaphore.h"

/**************************
 * 函数名:sema_init()
 * psema:信号量指针
 * value:信号量初值
 * 功能:初始化信号量值和等待队列
 * 返回值:无
 */ 
void sema_init(semaphore* psema, int value)
{
    /* 初始化值 */
    psema->value = value;

    /* 初始化信号量等待队列 */
    list_init(&psema->waiters);
}

