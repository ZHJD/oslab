#include "syscall_init.h"
#include "syscall.h"
#include "thread.h"
#include "console.h"
#include "string.h"
#include "fs.h"


/* 中断个数 */
#define syscall_nr 32

typedef void* syscall;

/* 系统调用表 */
syscall syscall_table[syscall_nr];

/* 获取线程或进程id */
uint32_t sys_getpid(void)
{
    return get_running_thread_pcb()->pid;
}

/* 初始化系统调用表 */
void syscall_init(void)
{
    put_str("syscall_init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_WRITE]  = sys_write;
    syscall_table[SYS_MALLOC] = sys_malloc;
    syscall_table[SYS_FREE]   = sys_free;
    put_str("syscall_init done\n");
}
