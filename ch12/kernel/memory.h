#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H 

#include "global.h"
#include "bitmap.h"
#include "list.h"

/* 内存块，存储在每个空闲内存块起始地址处 */
typedef struct mem_block
{
    /* 链接空闲内存块,通过free_elem反向推导出内存块地址 */
    list_elem free_elem;
}mem_block;

/* 内存块描述符 */
typedef struct mem_block_desc
{
    /* 内存块大小 */
    uint32_t block_size;

    /* 每个arena包含多少mem_block */
    uint32_t blocks_pre_arena;
    
    /* 目前可用的mem_block链接 */  
    struct list free_list;
}mem_block_desc;

/* 定义7个内存块描述符 */
#define DESC_CNT 7



/* 虚拟地址池，用于虚拟地址管理, 虚拟内存没有设置大小 */
typedef struct virtual_addr
{
    /* 虚拟地址使用到的位图结构 */
    bitmap vaddr_bitmap;

    /* 以此为起始地址分配虚拟地址 */
    uint32_t vaddr_start;

}virtual_addr;

/* 内存池标记，用于判断属于哪个内存池 */
typedef enum pool_flags
{
    /* 表示内核内存池 */
    PF_KERNEL = 1,

    /* 表示用户内存池 */
    PF_USER   = 2
}pool_flags;

/**************** 页表属性项 ********************
 * | 2  | 1  | 0 |
 * | us | RW | P |
 * 下面的定义也是按照这种次序定义的，便于合成
 */

/* 页表或页目录项不在内存中 */
#define PG_P_0   0

/* 页表或页目录项在内存中 */
#define PG_P_1   1

/* 可读 */
#define PG_RW_R  0

/* 可读可写 */
#define PG_RW_W  2

/* 系统级 */
#define PG_US_S  0

/* 用户级 */
#define PG_US_U  4
//////////////////////////////////////////////////////////



/* 内核内存池 */
//extern pool kernel_pool;

/* 用户内存池 */
//extern pool user_pool;

/**********************************************
 * 函数名:get_kernel_pages()
 * pg_cnt:要分配的页面数
 * 功能:在内核的内存池中分配页面
 * 返回:成功返回起始虚拟地址，失败返回NULL
 */
void* get_kernel_pages(const uint32_t pg_cnt);

/*****************************************************
 * 函数名:mem_init()
 * 功能:空闲内存管理系统入口
 * 返回值:无
 */
void mem_init();

/*********************************************
 * 函数名:get_user_pages()
 * pg_cnt:要申请的页面数
 * 功能:在用户虚拟地址空间和物理内存上申请pg_cnt个页面，全部清零
 * 返回值:返回虚拟地址
 */
void* get_user_pages(uint32_t pg_cnt);

/*****************************************************
 * 函数名:get_a_page()
 * pf:内核空间还是用户空间
 * vaddr:要申请的虚拟地址
 * 功能:申请一页内存，并把vaddr映射到该页内存上
 * 返回值:成功返回vaddr,失败返回null
 */ 
void* get_a_page(pool_flags pf, uint32_t vaddr);

/* 初始化空闲内存块，共7种不同的规格 */
void block_desc_init(mem_block_desc* desc_array);

/* 在堆中申请size字节内存 */
void* sys_malloc(uint32_t size);

/* 回收内存 */
void sys_free(void* ptr);
#endif
