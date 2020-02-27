#include "thread.h"
#include "string.h"
#include "interrupt.h"
#include "debug.h"
#include "process.h"
#include "sync.h"

/* idle线程,系统空闲的时候运行 */
task_struct* idle_thread;

/* 主线程pcb */
task_struct* main_thread;

/* 就绪队列 */
list thread_ready_list;

/* 所有任务队列 */
list thread_all_list;

/* 用于保存队列中的线程结点 */
list_elem* thread_tag;

/* pid锁 */
lock pid_lock;

extern void switch_to(task_struct* cur, task_struct* next);

/* 就绪队列为空时执行该进程 */
static void idle(void* arg)
{
    while(true)
    {
        /* 被创建后立即处于阻塞状态，如果就绪队列为空，唤醒该进程 */
        thread_block(TASK_BLOCKED);

        /* 执行hit指令前先开中断,hit指令执行后cpu利用率为%0 */
        asm volatile ("sti; hlt": : : "memory");
    }
}


/* 分配pid  */
static pid_t allocate_pid(void)
{
    static pid_t next_pid = -1;
    lock_acquire(&pid_lock);
    next_pid++;
    lock_release(&pid_lock);
    return next_pid;
}

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
        
    uint32_t intr_stack_size = sizeof(intr_stack);
    /* 为中断栈留出空间 */
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread->self_kstack - intr_stack_size);

    /* 得到线程栈栈底 */
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread->self_kstack - sizeof(thread_stack));

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
    pthread->pid = allocate_pid();
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
    
    pthread->pgdir_vaddr = NULL;
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
    init_thread(main_thread, "main", 31);

    /* main函数是当前正在运行的线程，不在thread_all_list中 */
    ASSERT(!elem_find(&thread_all_list, &main_thread->all_list_tag));
    list_push_back(&thread_all_list, &main_thread->all_list_tag);
}

/* 主动让出cpu */
void thread_yield(void)
{
    task_struct* cur = get_running_thread_pcb();
    intr_status old_status = intr_disable();

    /* 添加到就绪队列当中 */
    list_push_back(&thread_ready_list, &cur->general_tag);
    cur->status = TASK_READY;
    schedule();
    set_intr_status(old_status);
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
       // put_str("\nblock\n");
    }
        
    /* 如果就绪队列为空，唤醒idle_thread */
    if(list_empty(&thread_ready_list))
    {
        thread_unblock(idle_thread);
    }

    /* 用于保存队列中的线程结点 */
    list_elem* thread_tag = list_pop_head(&thread_ready_list);

    task_struct* next = (task_struct*)((uint32_t)(thread_tag) & 0xfffff000);
    
    next->status = TASK_RUNNING;

    /* 切换任务页表等 */
    process_activate(next);

    switch_to(cur, next);
}

/**********************************************************
 * 函数名:thread_block()
 * stat:线程状态
 * 功能:修改当前线程为阻塞态，换下处理器，当苏醒后恢复阻塞前的中断状态
 * 返回值:无
 */ 
void thread_block(const task_status stat)
{
    /* 只有当线程的取值为TASK_BLOCKED TASK_WAITING,TASK_HANGING时才不会被调度 */
    ASSERT(stat == TASK_BLOCKED || 
           stat == TASK_WAITING ||
           stat == TASK_HANGING
    );
    intr_status old_status = intr_disable();
    task_struct* cur_thread = get_running_thread_pcb();

    /* 只有正在运行的线程才能进入该函数，阻塞是一种主动行为 */
    ASSERT(cur_thread->status == TASK_RUNNING);
    cur_thread->status = stat;

    /* 把当前线程换下处理器 */
    schedule();

    /* 当该线程苏醒后恢复中断状态  */
    set_intr_status(old_status);
}

/*************************************************************
 * 函数名:thread_unblock()
 * pthread:被唤醒的线程，由当前线程调用
 * 功能:把pthread唤醒，加入就绪队列，状态设置为ready
 * 返回值:无
 */ 
void thread_unblock(task_struct* pthread)
{
    intr_status old_status = intr_disable();
    /* 只有当线程的取值为TASK_BLOCKED TASK_WAITING,TASK_HANGING时才能被唤醒 */
    ASSERT(pthread->status == TASK_BLOCKED || 
           pthread->status == TASK_WAITING ||
           pthread->status == TASK_HANGING
    );

    /* 被唤醒的线程一定不是就绪态也一定不在就绪队列中 */
    ASSERT(pthread->status != TASK_READY);
    ASSERT(!elem_find(&thread_ready_list, &pthread->general_tag));

    /* 添加到就绪队列中并改为就绪态 */
    list_push_head(&thread_ready_list, &pthread->general_tag);
    pthread->status = TASK_READY;

    set_intr_status(old_status);
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
    
    /* 初始化锁 */
    lock_init(&pid_lock);
    /* 把当前main创建为线程 */
    make_main_thread();
    
    /* 创建idle线程  */
    idle_thread = thread_start("idle", 10, idle, NULL);
    
    put_str("thread init end\n");
}
