#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMPRY_H

#include "stdint.h"
#include "bitmap.h"

/* 虚拟地址池，用于虚拟地址管理, 虚拟内存没有设置大小 */
typedef struct virtual_addr
{
    /* 虚拟地址使用到的位图结构 */
    bitmap vaddr_bitmap;

    /* 以此为起始地址分配虚拟地址 */
    uint32_t vaddr_start;

}virtual_addr;

/* 内核内存池 */
extern kernel_pool;

/* 用户内存池 */
extern user_pool;

/*****************************************************
 * 函数名:mem_init()
 * 功能:空闲内存管理系统入口
 * 返回值:无
 */
void mem_init();

#endif
