/************************及其模式************************************
 *  b -- QImode,即寄存器中的最低8位[a-d]l
 *  w -- HImode,即寄存器中2字节部分[a-d]x
 *  HImode half of integer 表示一个两字节的整数
 *  QImode quarter integer 表示一个一字节的整数
 *******************************************************************/

#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

// 向端口port中写入一个字节
// static 该文件外不可见，并且static函数只能在定义它的那个文件中实现
// inline 避免函数调用，提升速度
static inline void outb(uint16_t port, uint8_t data)
{
  // 对端口指定N表示0-255，d表示用dx存储
  // %b0表示对应al，%w1表示dx
  asm volatile ("outb %b0, %w1" : : "a"(data), "d"(port));
}

// 将addr处起始的word_cnt个字节写入端口port
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt)
{
  // +表示既做输入又做输出，且限制仅能用于输出处
  // S 表示esi
  // c 表示ecx
  // d 表示edx
  // outsw表示把从ds:esi开始的连续word_cnt个16bit的内如写入端口port处
  // ds,es,ss内容相同
  asm volatile ("cld; rep movsw": "+S"(addr), "+c"(word_cnt): "d"(port));
}

// 从port处读一个字节
static inline uint8_t inb(uint16_t port)
{
  uint8_t data;
  // N 限制为0-255
  asm volatile("inb %w1, %b0": "=a"(data): "Nd"(port));
  return data;
}

// 从端口port读出的word_cnt个字写入addr
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt)
{
  // insw负责把从端口中读出的数据写入到es:edi指向的内存
  // ecx是循环计数器
  asm volatile ("cld; rep insw": "+D"(addr), "+c"(word_cnt): "d"(port): "memory");
}


#endif
