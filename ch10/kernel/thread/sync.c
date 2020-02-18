#include "sync.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"

/***************************
 * 函数:lock_init()
 * plock:lock指针
 * 功能:初始化锁
 * 返回值:无
 */
void lock_init(lock* plock)
{
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;

    /* 信号量初值设为1，用来实现锁 */
    sema_init(&plock->semaphore_binary, 1);
} 


/*******************************************
 * 函数名:lock_acquire()
 * plock lock指针
 * 功能:加锁
 * 返回值:无
 */ 
void lock_acquire(lock* plock)
{
    /* 当前线程不是锁的拥有者 */
    if(plock->holder != get_running_thread_pcb())
    {
        /* 请求获得锁, 原子操作 */
        sema_down(&plock->semaphore_binary);

        /* 已经获得，修改所有者 */
        plock->holder = get_running_thread_pcb();
        
        ASSERT(plock->holder_repeat_nr == 0);

        plock->holder_repeat_nr = 1;
    }
    else
    {
       // put_str("\n123\n");
        /* 请求次数加1 */
        plock->holder_repeat_nr++;
    }
}

/********************************************
 * 函数名:lock_release()
 * plock:lock指针
 * 功能:释放锁，唤醒阻塞线程
 * 返回值:无
 */ 
void lock_release(lock* plock)
{
    ASSERT(plock->holder == get_running_thread_pcb());

    if(plock->holder_repeat_nr > 1)
    {
        plock->holder_repeat_nr--;
        return;
    }

    /***********************************
     * 该函数并非原子操作，所以要先把holder
     * 置NULL,然后再释放信号量,先释放信号量的话，
     * 可能会立即被其他线程拿到
     */ 
    plock->holder = NULL;
    plock->holder_repeat_nr = 0;

    /* 执行v操作，是原子操作 */
    sema_up(&plock->semaphore_binary);
}
