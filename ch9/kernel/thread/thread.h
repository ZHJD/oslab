#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "global.h"

/* 定义通用函数类型，它将在很多线程函数中作为形参参数 */
typedef void thread_func(void*);

/* 进程或者线程的状态 */
typedef enum task_status
{
    /* 运行 */
    TASK_RUNNING,

    /* 就绪 */
    TASK_READY,

    /* 阻塞 */
    TASK_BLOCKED,

    TASK_WAITING,


    TASK_HANGING,

    /* 暂停 */
    TASK_DIED
}task_status;

/********** 中断栈 ******************
 * 此结构用于中断发生时保护程序的上下文环境进程或
 * 线程被外部或者软中断打断时，会按照此结构压入上下文
 * 寄存器，intr_exit中的出栈操作是此结构的逆操作，此
 * 栈在线程自己的内核栈中位置固定，位于所在页的最顶端
 */
typedef struct intr_stack
{
    /* 中断号 */
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    
    /* pushad把esp压入，但popad忽略 */
    uint32_t esp_dumpy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fa;
    uint32_t es;
    uint32_t ds;

    uint32_t err_code;
    void (*eip)(void);
    uint32_t cs;
    uint32_t eflags;

    /* 以下由cpu从低特权级进入高特权级时压入 */
    void* esp;  

    /* ss压入的是4个字节 */
    uint32_t ss;
}intr_stack;

/**************** 线程栈 *********************
 * 线程自己的栈，用于存储执行线程中的环境
 */
typedef struct thread_stack
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    /**************************************************
     * 线程第一次执行时，eip指向调用的函数kernel_thread
     * 其他时候，eip指向switch_to的返回地址
     * ret指令执行后进入eip指向的函数，栈中的eip被弹出
     */
    void (*eip)(thread_func* func, void* func_arg);

    /* 以下仅供第一次被调度上cpu时使用 */

    /* 为了模拟call指令，call执行后，进入被调用函数，栈顶元素是
    *  返回地址。
    */
    void (*unused_retaddr);

    /* 由kernel_thread所调用的函数名  */
    thread_func* function;

    /* 由kernel_thread所调用的函数所需的参数 */
    void* func_arg;
}thread_stack;

/* 进程或线程的pcb,程序控制块 */
typedef struct task_struct
{
    /* 线程内核栈的栈顶 */
    uint32_t* self_kstack;
    
    /* 线程状态 */
    task_status status;

    /* 线程优先级 */
    uint8_t priority;

    /* 线程名字 */
    char name[14];

    /* 栈的边界标记，用于检测栈的溢出*/
    uint32_t stack_magic;
}task_struct;




/***********************************************
 * 函数名:thread_start()
 * name:线程名字
 * prio:线程优先级
 * function:执行的函数
 * func_arg:function的参数
 * 功能:通过该函数创建线程并执行
 * 返回值:线程pcb
 */
task_struct* thread_start(char* name,
                          int prio,
                          thread_func* function,
                          void* func_arg);
#endif
