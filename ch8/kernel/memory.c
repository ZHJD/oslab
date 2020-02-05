#include "memory.h"
#include "print.h"

/* 一个页大小为 4096 KB */
#define PAGE_SIZE 4096

/************* 位图地址 *****************
 * 0xc009f000是内核主线程栈顶,0xc009e000是main线程pcb起始地址
 * 4KB的位图可以表示 128MB 的空闲内存，位图安排在地址0xc009a000
 * 本系统最大支持管理 512 MB的内存
 */
#define MEM_BITMAP_BASE 0xc009a000

/****************************************
 * 0xc0000000 是内核从虚拟地址3G起
 * 跨国低端 1MB 内存
 * 堆用于内核在运行过程中动态申请空间
 */
#define K_HeAP_START    0xc0100000

/* 内存池结构，生成两个实列用于管理内核内存池和用户内存池 */
typedef struct pool
{
     /* 本内存池用到的位图结构，用于管理物理内存  */
     bitmap pool_bitmap;

     /* 以此为起始分配物理内存 */
     uint32_t phy_addr_start;

     /* 本内存池字节容量 */
     uint32_t pool_size;
}pool;
 
/* 内核内存池 */
pool kerrnel_pool;

/* 用户内存池 */
pool user_pool;

/* 此结构给内核分配虚拟地址  */
virtual_addr kernel_vaddr;

/*********************************************
 * 函数名:mem_pool_init()
 * 功能:根据all_mem初始化物理内存池的相关结构
 * all_mem:内存总大小
 * 返回值:无
 */
static void mem_pool_init(const uint32_t all_mem)
{
    put_str(" mem_pool_init start! \n");

    /**********************************************
    * 内核中建立的页表和页目录表总共占的空间，
    * 页目录项占 4KB, 页目录项中第 768-1022 项
    * 指向 255 个页表
    */
    uint32_t page_table_size = 256 * PAGE_SIZE;

    /* 0x100000表示低端1MB大小 */
    uint32_t used_mem = 0x100000 + page_table_size;

    uint32_t free_mem = all_mem - used_mem;

    /* 按页划分内存 */
    uint16_t all_free_pages = free_mem / PAGE_SIZE;

    /* 分配给内核的空闲页 */
    uint16_t kernel_free_page = all_free_pages / 2;

    uint16_t user_free_pages = all_free_pages - kernel_free_page;

    /* 表示内核空闲页的位图中bit的长度，一位表示一页, 位图以字节为单位 */
    uint32_t kbm_length = kernel_free_page / 8;

    uint32_t ubm_length = user_free_pages / 8;

    /* 内核空闲页起始地址 */
    uint32_t kp_start = used_mem;

    uint32_t up_start = kp_start + kernel_free_page * PAGE_SIZE;

    kerrnel_pool.phy_addr_start = kp_start;
    kerrnel_pool.pool_size = kernel_free_page * PAGE_SIZE;
    kerrnel_pool.pool_bitmap.btmp_bytes_len = kbm_length;

    user_pool.phy_addr_start = up_start;
    user_pool.pool_size = user_free_pages * PAGE_SIZE;
    user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

    /* 在MEM_BITMAP_BASE 处生成位图 */
    kerrnel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;

    /* 用户位图在内核位图之后 */
    user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

    /********************** 输出内存池信息 ******************************/
    put_str("  kernel_pool_bitmap_start: 0x");
    put_int((int)kerrnel_pool.pool_bitmap.bits);
    put_char('\n');

    put_str("kernel_pool_phy_addr_start: 0x");
    put_int((int)kerrnel_pool.phy_addr_start);
    put_char('\n');

    put_str("  user_pool_bitmap_start: 0x");
    put_int((int)user_pool.pool_bitmap.bits);
    put_char('\n');

    put_str("user_pool_phy_addr_start: 0x");
    put_int((int)user_pool.phy_addr_start);
    put_char('\n');

    /* 位图所在内存请0 */
    bitmap_init(&kerrnel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    /* 初始化内核虚拟地址的位图，按照实际物理内存大小生成数组 */
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;

    /* 内核虚拟地址的位图放在物理内存位图之后 */
    kernel_vaddr.vaddr_bitmap.bits = (void*)(MEM_BITMAP_BASE + 
                                     kbm_length +
                                     ubm_length);
    /* 内核空闲虚拟地址开始处 */
    kernel_vaddr.vaddr_start = K_HeAP_START;

    bitmap_init(&kernel_vaddr.vaddr_bitmap);

    put_str("mem init done \n");
}

/*****************************************************
 * 函数名:mem_init()
 * 功能:空闲内存管理系统入口
 * 返回值:无
 */
void mem_init()
{
    put_str("mem_init start\n");

    /* total_mem_bytes 保存在0xb00处，占4个字节 */
    uint32_t total_mem_bytes = *((uint32_t*)(0xb00));

    mem_pool_init(total_mem_bytes);

    put_str("mem init done \n");
    
}
