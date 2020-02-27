#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include "global.h"



/* 定义字符指针 */
typedef char* va_list;

/* 把ap指向第一个固定参数v的地址 */
#define va_start(ap, v) ap = (va_list)&v

/* ap指向下一个参数并返回值 */
#define va_arg(ap, t) *((t*)(ap += 4))

/* 清除ap */
#define va_end(ap) ap = NULL


/* 将参数 ap 转换成 format 输出到字符串 str, 并返回替换后 str 长度 */
uint32_t vsprintf(char* str, const char* format, va_list ap);

/* 格式化输出字符串format */
uint32_t printf(const char* format, ...);

/* 各种类型转换为char*  */
void sprintf(char* str, const char* format, ...);

#endif
