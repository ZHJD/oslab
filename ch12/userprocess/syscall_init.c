#include "syscall.h"
#include "thread.h"
#include "console.h"
#include "string.h"

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

/* 打印字符串str(未实现文件系统) */
uint32_t sys_write(char* str)
{
    console_put_str(str);
    return strlen(str);
}

/* 初始化系统调用表 */
void syscall_init(void)
{
    put_str("syscall_init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_WRITE]  = sys_write;
    put_str("syscall_init done\n");
}
