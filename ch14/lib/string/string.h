#ifndef __STRING_H
#define __STRING_H
#include "stdint.h"


/******************************************************************************
 * 函数名:memset()
 * 功能:把从dst起始的size个字节量设置为value
 * dst_:起始地址
 * value:值
 * size:字节数
 × 返回值:无
 */
void memset(void* dst_, uint8_t value, uint32_t size);

/******************************************************************************
 * 函数名:memcpy()
 * 功能:从src_开始的size个字节复制到dst_处
 * src_:起始地址, 使用const约束避免修改指针src指向的内容
 * dst_:目标地址
 * size:字节数
 × 返回值:无
 */
void memcpy(void* dst_, const void* src_, uint32_t size);


/******************************************************************************
 * 函数名:memcmp()
 * 功能:比较以a_和b_开始的size个字节，如果a_和b_的ascll码相等，返回0，大于返回1，小于返回-1
 * a_:
 * b_:
 * size:字节数
 × 返回值:1  大于
 *        0  等于
 *       -1  小于
 */
int memcmp(const void* a_, const void* b_, uint32_t size);

/******************************************************************************
 * 函数名:strcpy()
 * 功能:讲字符串从src_复制到dst_,两个字符串都以'\0'为结尾
 * 返回:dst_
 */
char* strcpy(char* dst_, const char* src_);

/******************************************************************************
 * 函数名:strlen()
 * 功能:获取字符串长度,不包括'\0'
 * str: 指向字符串的指针
 * 返回:字符串长度
 */
uint32_t strlen(const char* str);

/******************************************************************************
 * 函数名:strcmp()
 * 功能:比较两个字符串大小(长度可以不等)
 * a_:
 * b_:
 * 返回值：a_ > b_ 1
 *         a_ < b_ -1
 *         a_ = b_ 0
 */
int strcmp(const char* a_, const char* b_);

/******************************************************************************
 * 函数名:strchr()
 * 功能:从左到右查找字符串str中首次出现字符ch的地址
 * str:字符串
 * ch:字符
 * 返回值:字符ch地址
 */
char* strchr(const char* str, const char ch);

/*****************************************************************************
 * 函数名:strrchr()
 * 功能：从右往左查找首次出现的ch的地址
 * str: 字符串首地址
 * ch:  待查找字符
 * 返回值：首次出现ch的地址
 */
char* strrchr(const char* str, const char ch);

/****************************************************************************
 * 函数名:strcat()
 * 功能：字符串src_连接到字符串dst_之后
 * dst_:目标字符串
 * src_:源字符串
 * 返回值:连接后字符串的首地址
 */
char* strcat(char* dst_, const char* src_);

/*************************************************************************
 * 函数名:strchrs()
 * 功能:在字符串str中查找ch出现的次数
 * str:字符串
 * ch:字符
 * 返回值:出现次数
 */
uint32_t strchrs(const char* str, char ch);

#endif
