#include "stdio_kernel.h"
#include "stdio.h"
#include "console.h"



/* 供内核使用的模式化输出函数 */
void printk(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buf[1024] = {'\0'};
    vsprintf(buf, format, args);
    va_end(args);

    /* 打印字符串  */
    console_put_str(buf);
}

