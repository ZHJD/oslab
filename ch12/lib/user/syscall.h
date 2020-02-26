#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "stdint.h"

enum SYSCALL_NR
{
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE
};

/* 获取进程或线程pid */
uint32_t getpid(void);

/* 打印字符串，返回字符串长度 */
uint32_t write(char* str);

/* 分配内存 */
void* malloc(uint32_t size);

/* 释放内存 */
void free(void* ptr);

#endif
