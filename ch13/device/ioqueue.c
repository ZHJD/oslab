#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

/**********************************************
 * 函数名: ioqueue_init()
 * ioq:循环队列
 * 功能:初始化ioq
 * 返回值:无
 */
void ioqueue_init(ioqueue* ioq)
{
    /* 初始化io队列的锁 */
    lock_init(&ioq->ioq_lock);

    sema_init(&ioq->full, 0);

    sema_init(&ioq->empty, bufsize);

    /* 队列的首尾指针指向缓冲区数组第0个位置 */
    ioq->head = ioq->tail = 0;
}

/************************************************
 * 函数名:next_pos()
 * 参数:pos 位置
 * 功能: 返回pos在缓冲区中的下一个位置值
 */ 
static inline int32_t next_pos(int32_t pos)
{
    return (pos + 1) % bufsize;
}

/************************************************
 * 函数名:ioq_getchar()
 * ioq:ioqueue指针
 * 功能:从缓冲区中读出一个字符
 * 返回值:读取的字符
 */ 
char ioq_getchar(ioqueue* ioq)
{

    /* P 操作 */
    sema_down(&ioq->full);
    lock_acquire(&ioq->ioq_lock);
    char byte = ioq->buf[ioq->tail];
    ioq->tail = next_pos(ioq->tail);
    lock_release(&ioq->ioq_lock);

    /* V 操作 */
    sema_up(&ioq->empty);
    return byte;
}

/**********************************************
 * 函数名:ioq_putchar()
 * ioq:ioqueue指针
 * byte:向缓冲区中放入的一个字节
 * 功能值:向缓冲区中放入一个字符
 * 返回值:无
 */ 
void ioq_putchar(ioqueue* ioq, const char byte)
{
    sema_down(&ioq->empty);
    lock_acquire(&ioq->ioq_lock);
    ioq->buf[ioq->head] = byte;
    ioq->head = next_pos(ioq->head);
    lock_release(&ioq->ioq_lock);
    sema_up(&ioq->full);
 }


