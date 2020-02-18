#include "memory.h"
#include "print.h"
#include "debug.h"



/* 页目录表索引(高10位) */
#define PDE_IDX(vaddr) ((vaddr & 0xffc00000) >> 22)

/* 页表索引(中间10位) */
#define PTE_IDX(vaddr) ((vaddr >> 12) & 0x3ff)

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

/****************************************************
 * 函数名:vaddr_get()
 * pf:内核内存池或用户内存池
 * pg_cnt:页个数
 * 功能:在pf内存池中申请pg_cnt个页面
 * 返回值:返回虚拟页的起始地址
 */
static void* vaddr_get(const pool_flags pf, const uint32_t pg_cnt)
{
    int vaddr_start = 0;
    
    /* 表示自此位开始有连续pg_cnt个空闲位 */
    int bit_idx_start = -1;

    if(pf == PF_KERNEL)
    {
        /* 扫描得到连续pg_cnt个空闲页 */
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
        

        if(bit_idx_start == -1)
        {
            put_str("\nscan error\n");
            return NULL;
        }
        /* 对这pg_cnt个位赋值1，表示已经分配，避免重复分配 */
        for(uint32_t i = 0; i < pg_cnt; i++)
        {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + i, 1);
        }

        /* 申请到的虚拟地址的起始地址 */
        vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE;
    }
    else
    {
        // 实现用户进程分配
    }
    return (void*)vaddr_start;
}

/****************************************************************
 * 函数名:get_pde_ptr()
 * vaddr:虚拟地址
 * 功能:得到vaddr对应的页目录表的中指向对应项的指针
 * 返回值:页目录表中每一项占4字节，返回指向这4字节的指针
 */
uint32_t* get_pde_ptr(const uint32_t vaddr)
{
    uint32_t pde_idx = PDE_IDX(vaddr);

    /* 0xfffff会访问到页目录所在页面，pde_idx * 4是页内偏移地址 */
    /* 原因是页目录表中的第1023项指向自身 */
    uint32_t* pde = (uint32_t*)(0xfffff000 + pde_idx * 4);

    return pde;
}

/*********************************************************
 * 函数名:get_pte_ptr()
 * vaddr:虚拟地址
 * 功能:获取指向页表项的指针
 * 返回:指向对应页表项的指针
 */
uint32_t* get_pte_ptr(const uint32_t vaddr)
{
    uint32_t pte_idx = PTE_IDX(vaddr);

    /* pde 中的内容可能为空，所以用这种方式 */
    uint32_t* pte = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 
                                10) + pte_idx * 4);
    return pte;
}

/****************************************************************
 * 函数名:phy_alloc()
 * m_pool:指向物理内存池的指针
 * 功能:在m_pool中分配一个物理页框
 * 返回值:成功返回物理地址，失败返回NULL
 */
static void* phy_alloc(pool* m_pool)
{
    /* 扫描位图 */
    int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1);

    if(bit_idx == -1)
    {
        return NULL;
    }
    /* 分配 */
    bitmap_set(&m_pool->pool_bitmap, bit_idx, 1);
    

    /* 返回地址 */
    return (void*)(m_pool->phy_addr_start + bit_idx * PAGE_SIZE);
}

/*****************************************************
 * 功能:page_table_add()
 * _vaddr:虚拟地址
 * _page_phy_addr:物理地址
 * 功能:在页表中添加虚拟地址到物理地址映射的函数
 * 返回值:无
 */
static void page_table_add(void* _vaddr, void* _page_phyaddr)
{
    uint32_t vaddr = (uint32_t)_vaddr;
    uint32_t page_phyaddr = (uint32_t)_page_phyaddr;

    uint32_t* pde = get_pde_ptr(vaddr);
    uint32_t* pte = get_pte_ptr(vaddr);
   
   // ASSERT(1 == 2);

    /*************** 注意 *******************************
     * 执行 *pte,可能会访问到*pde,所以要确保pde创建后才能
     * 访问*pte,否则会引发page_fault.页目录表所在的内存最初
     * 已经全部请0
     ***************************************************/
    /* 判断页目录表中的p位是否为1 */
    if(*pde & 0x00000001)
    {
        ASSERT(!(*pte & 0x00000001));

        if(!(*pte & 0x00000001))
        {
            /* 只创建页表 US = 1, RW = 1, P + 1*/
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
        else
        {
            PANIC("page repeat!!!\n");
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
    }
    else
    {
        /* 页目录不存在，先创建页目录表项再创建页表项 */

        /* 在内核空间新键一个一级页表 */
        uint32_t pte_phyaddr = (uint32_t)phy_alloc(&kerrnel_pool);
        *pde = (pte_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

        /* 新分配的用于存放页表项的内存全部清0 */
        memset((void*)((uint32_t)pte & 0xfffff000), 0, PAGE_SIZE);

        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
    

}

/********************************************************************
 * 函数名:malloc_page()
 * pf:为内核分配还是为
 * pg_cnt:页框个数
 * 功能:分配 pg_cnt 个页空间，成功则返回起始虚拟地址，失败时返回NULL
 * 返回值:成功返回起始虚拟地址，失败返回NULL
 */
void* malloc_page(const pool_flags pf, const uint32_t pg_cnt)
{
    /* 内核空间共16MB */
    ASSERT(pg_cnt > 0 && pg_cnt < 3840);

    /******* malloc_page原理是三个动作的合成 *****************
     * 通过vaddr_get在虚拟内存池中申请虚拟地址
     * 通过phy_malloc在物理内存池中申请物理页
     * 通过page_table_add 把以上得到的虚拟地址和物理地址在页表中完成映射
     */
    void* vaddr_start = vaddr_get(pf, pg_cnt);
    if(vaddr_start == NULL)
    {
        ASSERT(vaddr_start != NULL);
         return NULL;
    }
    
    uint32_t vaddr = (uint32_t)vaddr_start;
    pool* mem_pool = pf == PF_KERNEL? &kerrnel_pool: &user_pool;

    /* 由于虚拟地址连续，物理地址不连续，所以分配物理地址时候逐个做 */
    for(uint32_t i = 0; i < pg_cnt; i++)
    {
        void* page_phyaddr = phy_alloc(mem_pool);
        if(page_phyaddr == NULL)
        {
            ASSERT(page_phyaddr != NULL);
            /* 此处需要回收掉已经分配的*/
            return NULL;
        }
        page_table_add((void*)vaddr, page_phyaddr);
        vaddr += PAGE_SIZE;
    }
    return vaddr_start;
}

/**********************************************
 * 函数名:get_kernel_pages()
 * pg_cnt:要分配的页面数
 * 功能:在内核的内存池中分配页面
 * 返回:成功返回起始虚拟地址，失败返回NULL
 */
void* get_kernel_pages(const uint32_t pg_cnt)
{
    void* vaddr = malloc_page(PF_KERNEL, pg_cnt);
    if(vaddr != NULL)
    {
        memset(vaddr, 0, pg_cnt * PAGE_SIZE);
    }
    return vaddr;
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
