#include "process.h"
#include "interrupt.h"
#include "global.h"
#include "console.h"
#include "list.h"
#include "debug.h"

extern list  thread_ready_list;
extern list  thread_all_list;


/********************************
 * 函数名:intr_exit()
 * 功能:退出中断
 * 返回值:无
 */ 
extern void intr_exit(void);


/***************************************
 * 函数名:start_process()
 * filename_:创建的程序名字
 * 功能:初始化进程的上下文
 * 返回值:无
 */
void start_process(void* filename_)
{
    /* 制造中断现场假象，使用iretd指令进入用户态 */
    void* function = filename_;
    task_struct* cur = get_running_thread_pcb();
    cur->self_kstack = (uint32_t*)((uint32_t)cur->self_kstack + sizeof(thread_stack));
    intr_stack* proc_stack = (intr_stack*)cur->self_kstack;
    
    proc_stack->edi = 0;
    proc_stack->esi = 0;
    proc_stack->ebp = 0;
    proc_stack->esp_dummy = 0;
    
    proc_stack->ebx = 0;
    proc_stack->edx = 0;
    proc_stack->ecx = 0;
    proc_stack->eax = 0;
    
    /* 显存段，在用户态下用不到 */
    proc_stack->gs = 0;

    proc_stack->ds = SELECTOR_U_DATA;
    proc_stack->es = SELECTOR_U_DATA;
    proc_stack->fs = SELECTOR_U_DATA;
    proc_stack->eip = function;
    proc_stack->cs = SELECTOR_U_CODE;
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);

    /* 用户态下栈顶地址是0xc0000000 */
    proc_stack->esp = (void*)((uint32_t)get_a_page(PF_USER, USER_STACK3_VADDR) +
                                    PAGE_SIZE);
    

    proc_stack->ss = SELECTOR_U_DATA;
    asm volatile ("movl %0, %%esp; jmp intr_exit"::
                "g"(proc_stack): "memory");
}

/*****************************************************************
 * 功能表:page_dir_activate()
 * pthread:进程或者线程
 * 功能:安装页目录表
 * 返回值:无
 */ 
static void page_dir_activate(task_struct* pthread)
{
    /************************************************************
     * 调用这个函数的可能是用户进程也可能是内核线程，由于调度的原因
     * 都需要重新加载页目录表
     */ 

    /* 内核页目录表起始地址 */
    uint32_t pagedir_phy_addr = 0x100000;

    if(pthread->pgdir_vaddr != NULL)
    {
        /* 若为用户进程 */
        pagedir_phy_addr = addr_v2p((uint32_t)pthread->pgdir_vaddr);
    }
    /* 把页目录表的物理起始地址写入cr3寄存器 */
    asm volatile ("movl %0, %%cr3": : "r"(pagedir_phy_addr): "memory");
}

/********************************************************************
 * 函数名:process_activate()
 * pthread:pcb指针
 * 功能:激活进程或者线程的页目录表，更新tss中的esp0位进程的特权级0的z栈
 * 返回值:无 
 */ 
void process_activate(task_struct* pthread)
{
    ASSERT(pthread != NULL);

    /* 激活页表 */
    page_dir_activate(pthread);

    /* 内核线程位于特权级0，处理器不会使用tss中的栈指针 */

    /* 更新用户进程的tss */
    if(pthread->pgdir_vaddr != NULL)
    {
        update_tss_esp0(pthread);
    }
}

/******************************************
 * 函数名:vreate_page_dir()
 * 功能:在内核空间创建用户进程的页目录表，
 * 第768 - 1022项指向内核空间
 * 返回值:页表虚拟地址
 */ 
uint32_t* create_page_dir(void)
{
    /* 用户的页表也放在内核空间 */
    uint32_t* page_dir_vaddr = get_kernel_pages(1);

    if(page_dir_vaddr == NULL)
    {
        console_put_str("create_page_dir: get kernel_page failed!\n");
        return NULL;
    }

    /**************************** 1 先复制页表 ***************************
     * page_dir_vaddr + 0x300 * 4 是内核页目录的第768项
     * (1024 - 768) * 4 = 1024字节
     */ 
    memcpy((uint32_t*)((uint32_t)page_dir_vaddr + 0x300 * 4),
            (uint32_t*)(0xfffff000 + 0x300 * 4),
            1024);


    /*********************** 2 更新页目录地址 **************************/
    uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t)page_dir_vaddr);

    /* 页目录表最后一项放自身的起始物理地址，偏于在内核中访问页表 */
    page_dir_vaddr[1023] = new_page_dir_phy_addr | 
                            PG_US_U |
                            PG_RW_W |
                            PG_P_1;
    
    return page_dir_vaddr;
}

/****************************************************************
 * 函数名:create_user_vaddr_bitmap()
 * user_prog:进程pcb
 * 功能:在内核空间为该进程创建虚拟地址位图，并赋值给user_prog
 * 的虚拟地址字段
 * 返回值:无
 */ 
void create_user_vaddr_bitmap(task_struct* user_prog)
{
    /* 用户进程内存的起始地址 */
    user_prog->userprog_vaddr.vaddr_start = USER_VADDR_START;

    /* 位图所占页面数，已经向上取整 */
    uint32_t bitmap_pg_cnt = DIV_ROUND_UP(
        (0xc0000000 - USER_VADDR_START) / PAGE_SIZE / 8, // 需要分配的字节数
        PAGE_SIZE
    );

    /* 位图在内核空间分配，位图起始地址 */
    user_prog->userprog_vaddr.vaddr_bitmap.bits = get_kernel_pages(bitmap_pg_cnt);

    /* 位图字节数 */
    user_prog->userprog_vaddr.vaddr_bitmap.btmp_bytes_len = 
         (0xc0000000 - USER_VADDR_START) / PAGE_SIZE / 8;
    
    /* 分配页面时已经清过0了 */
    bitmap_init(&user_prog->userprog_vaddr.vaddr_bitmap);
}

/*********************************************************
 * 函数名:process_execute()
 * filename: 用户程序文件名
 * name:线程名字
 * 功能:创建进程
 * 返回值:无
 */ 
void process_execute(void* filename, char* name)
{
    /* 在内核空间为用户进程创建pcb */
    task_struct* thread = get_kernel_pages(1);

    /* 初始化线程 */
    init_thread(thread, name, default_prio);

    /* 创建用户进程的虚拟地址管理位图 */
    create_user_vaddr_bitmap(thread);

    /* 准备好线程栈和执行函数，start_process是线程要执行的函数 */
    thread_create(thread, start_process, filename);

    /* 设置进程的页目录表虚拟地址 */
    thread->pgdir_vaddr = create_page_dir();

    intr_status old_status = intr_disable();
    list_push_back(&thread_ready_list, &thread->general_tag);
    list_push_back(&thread_all_list, &thread->all_list_tag);
    set_intr_status(old_status);
}
