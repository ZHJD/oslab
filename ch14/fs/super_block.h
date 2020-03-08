#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H
#include "stdint.h"

/* 超级块 */
struct super_block
{
    /* 用于识别文件系统类型 */
    uint32_t magic;

    /* 本分区总共的扇区数 */
    uint32_t sec_cnt;

    /* 本分区中inode的数量 */
    uint32_t inode_cnt;

    /* 本分区的起始lba地址 */
    uint32_t part_lba_base;

    /* 块位图本身起始扇区地址 */
    uint32_t block_bitmap_lba;

    /* 块位图占用的扇区数量 */
    uint32_t block_bitmap_sects;

    /* i结点位图起始扇区lba地址 */
    uint32_t inode_bitmap_lba;

    /* i结点位图占用的扇区数量 */
    uint32_t inode_bitmap_sects;

    /* i结点位图起始扇区lba地址 */
    uint32_t inode_table_lba;

    /* i结点表占用的扇区数量 */
    uint32_t inode_table_sects;

    /* 数据区开始的第一个扇区 */
    uint32_t data_start_lba;

    /* 根目录所在的i结点 */
    uint32_t root_inode_no;

    /* 目录项大小 */
    uint32_t dir_entry_size;

    /* 加上460字节，凑够512字节一个扇区大小 */
    uint8_t pad[460];

}__attribute__((packed));

typedef struct super_block super_block;








#endif
