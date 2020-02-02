#ifndef __STRING_H
#define __STRING_H

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







#endif
