#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "stdint.h"

enum SYSCALL_NR {
    SYS_GETPID,
    SYS_WRITE
};

/* 获取进程或线程pid */
uint32_t getpid(void);

/* 打印字符串，返回字符串长度 */
uint32_t write(char* str);

#endif
