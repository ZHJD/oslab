#include "inode.h"
#include "ide.h"
#include "interrupt.h"
#include "list.h"
#include "thread.h"
#include "debug.h"

/* 用来存储inode位置 */
typedef struct inode_position
{
    /* inode是否跨扇区 */
    bool two_sec;

    /* inode所在扇区号 */
    uint32_t sec_lba;

    /* inode在扇区内的字节偏移量 */
    uint32_t off_size;
}inode_position;

/* 查找inode位置 */
static void inode_locate(partition* part, uint32_t inode_no, inode_position* inode_pos)
{
    ASSERT(inode_no < 4096);
    uint32_t inode_table_lba = part->sb->inode_table_lba;
    uint32_t inode_size = sizeof(struct inode);
    uint32_t off_size = inode_no * inode_size;

    /* 起始扇区，一个inode结点大小小于一个扇区 */
    uint32_t off_sec = off_size / 512;

    /* 待查找的inode所在扇区的起始地址 */
    uint32_t off_size_in_sec = off_size % 512;

    uint32_t left_in_sec = 512 - off_size_in_sec;

    /* 如果当前扇区放不下 */
    if(left_in_sec < inode_size)
    {
        inode_pos->two_sec = true;
    }
    else
    {
        inode_pos->two_sec = false;
    }
    inode_pos->sec_lba = inode_table_lba + off_sec;

    /* 扇区内的起始字节 */
    inode_pos->off_size = off_size_in_sec;
}

/* 将inode写入到分区part */
void inode_sync(partition* part, struct inode* inode, void* io_buf)
{
    /* io_buf 是用于硬盘的io缓冲区 */
    uint8_t inode_no = inode->i_no;
    inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);

    struct inode pure_inode;
    memcpy(&pure_inode, inode, sizeof(inode));

    /* 一下三个属性用在内存中 */
    pure_inode.i_open_cnts = 0;
    pure_inode.write_deny = false;
    pure_inode.inode_tag.prev = pure_inode.inode_tag.next = NULL;

    char* inode_buf = (char*)io_buf;

    if(inode_pos.two_sec) // 如果跨扇区了
    {
        /* 若是跨了两个扇区，就要读出两个扇区再写入两个扇区 */
        /* 读写硬盘是以扇区为单位，若写入的数据小于一个扇区，
            要将原硬盘上的内容先读出来再和新数据拼成一扇区后再写入。
        */
       ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
       
       /* 修改当前inode对应部分的内容 */
       memcpy(inode_buf + inode_pos.off_size, &pure_inode, sizeof(inode));

       ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    }
    else
    {
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
        memcpy(inode_buf + inode_pos.off_size, &pure_inode, sizeof(inode));

        /* 读出来，拼接后重新写入硬盘 */
        ide_write(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
}

/* 根据i结点号返回相应的i结点 */
struct inode* inode_open(partition* part, uint32_t inode_no)
{
    /* 先在inode打开队列中查找 */
    list_elem* elem = part->open_inodes.head.next;
    struct inode* inode_found;
    while(elem != &part->open_inodes.tail)
    {
        inode_found = elem2entry(struct inode, inode_tag, elem);
        if(inode_found->i_no == inode_no)
        {
            inode_found->i_open_cnts++;
            return inode_found;
        }
        elem = elem->next;
    }

    /* 如果在相应的打开链表中找不到 */
    inode_position inode_pos;
    inode_locate(part, inode_no, &inode_pos);

    /****************************************************************
     * 为使通过sys_malloc创建的新inode被所有任务共享，需要把inode放在内核空间，故需要临时 
     * 把
     */
    task_struct* cur = get_running_thread_pcb();
    uint32_t* cur_pagedir_bak = cur->pgdir_vaddr;
    cur->pgdir_vaddr = NULL;

    /* 在内核空间分配inode */
    inode_found = (struct inode*)sys_malloc(sizeof(struct inode));

    /* 恢复page_dir */
    cur->pgdir_vaddr = cur_pagedir_bak;

    char* inode_buf;
    if(inode_pos.two_sec)
    {
        inode_buf = (char*)sys_malloc(1024);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 2);
    }
    else
    {
        inode_buf = (char*)sys_malloc(512);
        ide_read(part->my_disk, inode_pos.sec_lba, inode_buf, 1);
    }
    memcpy(inode_found, inode_buf + inode_pos.off_size, sizeof(struct inode));
    
    /* 加入打开队列 */
    list_push_back(&part->open_inodes, &inode_found->inode_tag);
    inode_found->i_open_cnts = 1;

    sys_free(inode_buf);
    return inode_found;
}

/* 关闭inode或者减少inode的打开数 */
void inode_close(struct inode* inode)
{
    /* 若没有进程打开此文件，将此inode去掉并释放空间 */
    intr_status old_status = intr_disable();
    if(--inode->i_open_cnts == 0)
    {
        list_remove(&inode->inode_tag);
        /* inode open时为inode分配了内核空间，释放的时候应该释放此空间 */
        task_struct* cur = get_running_thread_pcb();
        uint32_t* cur_pagedir_bak = cur->pgdir_vaddr;
        cur->pgdir_vaddr = NULL;
        sys_free(inode);
        cur->pgdir_vaddr = cur_pagedir_bak;
    }
    set_intr_status(old_status);
}

/* 初始化new_inode */
void inode_init(uint32_t inode_no, struct inode* new_inode)
{
    new_inode->i_no = inode_no;
    new_inode->i_size = 0;
    new_inode->i_open_cnts = 0;
    new_inode->write_deny = false;

    /* 初始化块索引数组i_sector */
    for(uint8_t sec_idx = 0; sec_idx < 13; sec_idx++)
    {
        new_inode->i_sectors[sec_idx] = 0;
    }
}

