#ifndef __FS_INODE_H
#define __FS_INODE_H
#include "stdint.h"
#include "list.h"

/* inode结构 */
typedef struct inode
{
    /* inode 编号 */
    uint32_t i_no;

    /* 当此inode是文件时，i_size是指文件大小，
    若此inode是目录，i_size是指该目录下所有目录项大小之和
     */
    uint32_t i_size;

    /* 记录此文件被打开次数 */
    uint32_t i_open_cnts;

    /* 写文件不能并行，进程写文件前检查这个标识 */
    bool write_deny;

    /* i_sectors[0-11]是直接块,i_sectors[12]用来存储一级间接指针 */
    uint32_t i_sectors[13];

    /* 用于加入已打开的inode列表 */
    list_elem inode_tag;
}inode;

#endif
