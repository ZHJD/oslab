#include "thread.h"
#include "string.h"
#include "memory.h"
#include "interrupt.h"
#include "debug.h"

/* 主线程pcb */
task_struct* main_thread;

/* 就绪队列 */
list thread_ready_list;

/* 所有任务队列 */
list thread_all_list;

/* 用于保存队列中的线程结点 */
list_elem* thread_tag;

extern void switch_to(task_struct* cur, task_struct* next);


/************************************************
 * 函数名:get_running_thread_pcb()
 * 功能:获取正在运行线程的pcb
 * 返回值:指向该线程pcb的指针 
 */
task_struct* get_running_thread_pcb()
{
    uint32_t esp = 0;
    asm volatile("movl %%esp, %k0":"=g"(esp));

    /* pcb位于线程所在页的起始地址处 */
    return (task_struct*)(esp & 0xfffff000);
}

/*************************************************
 * 函数名:kernel_thread
 * function:要执行的函数
 * func_arg:参数
 * 功能:执行该函数
 */
static void kernel_thread(thread_func* function, void* func_arg)
{
    /* 避免中断关闭影响调度，开启中断 */
    intr_enable();

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

    if(pthread == main_thread)
    {
        /* main函数被封装为一个线程，且一直在运行，故设置其状态为RUNNING */
        pthread->status = TASK_RUNNING;
    }
    else
    {
       pthread->status = TASK_READY;
    }
    
    pthread->priority = prio;

    /* 该线程执行的时间篇 */
    pthread->ticks    = prio;

    /* 该线程尚未运行过 */
    pthread->elapsed_ticks = 0;
    
    pthread->pgvaddr = NULL;
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

    /***********************************************************
     *  由于线程刚刚创建，所以不应该出现在线程队列中。
     */

    /* 确保之前不在就绪队列中 */
    ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));

    /* 加入就绪队列 */
    list_push_back(&thread_ready_list, &thread->general_tag);

    /* 确保不在所有线程的队列中 */
    ASSERT(!elem_find(&thread_all_list, &thread->all_list_tag));

    /* 加入就绪队列 */
    list_push_back(&thread_all_list, &thread->all_list_tag);

    return thread;
}

/*************************************************************
 * 函数名:make_main_thread()
 * 功能:将kernel中的main函数完善为主线程
 * 返回值:无
 */
static void make_main_thread(void)
{
    /*****************************************
     * 因为main函数已经运行，且留了空间
     * 0x9e000 - 0x9f000,所以不需要通过
     * get_kernel_page另分配一页
     */
    main_thread = get_running_thread_pcb();
    init_thread(main_thread, "main", 1);

    /* main函数是当前正在运行的线程，不在thread_all_list中 */
    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_push_back(&thread_all_list, &main_thread->all_list_tag);
}

/***************************************************
 * 函数名:schedule()
 * 功能:实现任务调度
 * 返回值:无
 */
void schedule(void)
{
    ASSERT(get_intr_status() == INTR_OFF);
    
    task_struct* cur = get_running_thread_pcb();

    if(cur->status == TASK_RUNNING)
    {
        /* 此线程只是cpu时间片到了,触发了线程调度函数 */

        /* 由于当前线程处于运行态，不应该在就绪队列中 */
        ASSERT(!elem_find(&thread_ready_list, &cur->general_tag));
        
        /* 加入就绪队列末尾 */
        list_push_back(&thread_ready_list, &cur->general_tag);
        
        cur->ticks = cur->priority;
        cur->status = TASK_READY;

    }
    else
    {
        /* 此线程是由于其它原因下处理器的 */
    }
        
    /* 就绪队列不空 */
    ASSERT(!list_empty(&thread_ready_list));
    /* 用于保存队列中的线程结点 */
    list_elem* thread_tag = list_pop_head(&thread_ready_list);

    task_struct* next = (task_struct*)((uint32_t)(thread_tag) & 0xfffff000);
    
    next->status = TASK_RUNNING;

    switch_to(cur, next);
}

/*********************************************
 * 函数名:thread_init()
 * 功能:初始化线程
 * 返回值:无
 */
void thread_init(void)
{
    put_str("thread init start \n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    /* 把当前main创建为线程 */
    make_main_thread();
    put_str("thread init end\n");
}
