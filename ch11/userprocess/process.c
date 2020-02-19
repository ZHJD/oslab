#include "process.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"
/***************************************
 * 函数名:start_process()
 * filename_:创建的程序名字
 * 功能:初始化进程的上下文
 * 返回值:无
 */
void start_process(void* filename_)
{
    void* function = filename_;
    task_struct* cur = get_running_thread_pcb();
    cur->self_kstack += sizeof(thread_stack);
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
    proc_stack->esp = get_a_page(PF_USER, USER_STACK3_VADDR + PAGE_SIZE);
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
    uint32_t pagedir_phy_addr = 0x10000;

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
