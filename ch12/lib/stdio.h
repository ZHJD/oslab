#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include "global.h"

/* 定义字符指针 */
typedef char* va_list;

/* 将参数 ap 转换成 format 输出到字符串 str, 并返回替换后 str 长度 */
uint32_t vsprintf(char* str, const char* format, va_list ap);

/* 格式化输出字符串format */
uint32_t printf(const char* format, ...);

#endif
