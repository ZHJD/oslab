#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H
#include "stdio.h"
#include "bitmap.h"
#include "list.h"
#include "sync.h"
#include "semaphore.h"

typedef struct partition
{
    /* 起始扇区 */
    uint32_t start_lba;

    /* 扇区数 */
    uint32_t sec_cnt;

    /* 分区所属的磁盘 */
    struct disk* my_disk;

    /* 用于队列的标记  */
    list_elem  part_tag;

    /* 分区名称 */
    char name[8];

    /* 本区内的超级快 */
//    super_block* sb;

    /* 块位图 */
    bitmap block_bitmap;

    /* i结点位图 */
    bitmap inode_bitmap;

    /* 本分区打开的i结点队列 */
    list open_inode;
}partition;

typedef struct disk
{
    /* 本硬盘名称 */
    char name[8];

    /* 此块硬盘归属的ide通道 */
    struct ide_channel* my_channel;

    /* 本硬盘是主0,还是从1 */
    uint8_t dev_no;

    /* 主分区最多四个 */
    partition prim_parts[4];

    /* 逻辑分区最多8个 */
    partition logic_parts[8];
}disk;

typedef struct ide_channel
{
    /* 本ata通道名称 */
    char name[16];
    
    /* 本通道的起始端口号 */
    uint16_t port_base;

    /* 本通道所使用的中断号 */
    uint8_t irq_no;

    /* 通道锁 */
    lock channel_lock;

    /* 等待硬盘的中断 */
    bool expecting_intr;

    /* 用于阻塞、唤醒驱动程序 */
    semaphore disk_done;

    /* 每个通道链接两个硬盘 */
    disk devices[2];
}ide_channel;


/* 从硬盘读取sec_cnt个扇区到buf中 */
void ide_read(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);

/* 将 buf 中 sec_cnt 扇区数据写入硬盘 */
void ide_write(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);

/* 硬盘数据结构初始化 */
void ide_init();


#endif
