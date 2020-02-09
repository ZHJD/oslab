#include "thread.h"
#include "string.h"
#include "memory.h"


/*************************************************
 * 函数名:kernel_thread
 * function:要执行的函数
 * func_arg:参数
 * 功能:执行该函数
 */
static void kernel_thread(thread_func* function, void* func_arg)
{
    function(func_arg);
}

/***************************************************
 * 函数名:thread_create
 * pthread:pcb
 * function:要执行的函数
 * func_arg:function的参数
 * 功能:初始化线程栈thread_stack，把待执行的函数和参数放到
 * thread_stack中的相应位置
 */
void thread_create(task_struct* pthread, thread_func* function, void* func_arg)
{
    /* 为中断栈留出空间 */
    pthread->self_kstack -= sizeof(intr_stack);

    /* 得到线程栈栈底 */
    pthread->self_kstack -= sizeof(thread_stack);
    thread_stack* kthread_stack = (thread_stack*)pthread->self_kstack;

    /* 通过ret指令进入kernel_thread */
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;

    /* 要保护的寄存器 */
    kthread_stack->ebp = 0;
    kthread_stack->ebx = 0;
    kthread_stack->esi = 0;
    kthread_stack->edi = 0;
}

/*********************************************************
 * 函数名:init_thread()
 * pthread:线程pcb
 * name:线程名字
 * prio:线程优先级
 * 功能:初始化线程pcb
 * 返回:无
 */
void init_thread(task_struct* pthread, char* name, int prio)
{
    /* 清空线程运行环境所在的页 */
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);

    /* 设置线程状态 */
    pthread->status = TASK_BLOCKED;
    pthread->priority = prio;

    /* 线程栈顶在pcb所在页的最高地址处 */
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PAGE_SIZE);

    /* 魔数，用于检测栈顶指针是否进入pcb结构所在内存 */
    pthread->stack_magic = 0x19870916;
}

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
                          void* func_arg)
{
    /* 所有进程的pcb都在内核空间 */
    task_struct* thread = get_kernel_pages(1);

    init_thread(thread, function, func_arg);

    thread_create(thread, function, func_arg);

    /**************************************************************
     * ret指令执行后进入kernel_thread函数，此时栈顶是return address
     * esp + 4是第一个参数
     */
    asm volatile("movl %0, %%esp; \
                  pop %%ebp;     \
                  pop %%ebx;     \
                  pop %%edi;     \
                  pop %%esi;     \
                  ret": :        \
                  "g"(thread->self_kstack):   \
                  "memory");   \
    return thread;
}
