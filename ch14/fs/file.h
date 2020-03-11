#ifndef __FS_FILE_H
#define __FS_FILE_H
#include "stdint.h"

/* 文件结构 */
struct file
{
    /* 记录当前文件操作的偏移地址，以0为起始，最大为文件大小-1 */
    uint32_t fd_pos;

    /* 文件操作标识 */
    uint32_t fs_flags;

    /* 指向inode队列中open_inodes中的inode */
    struct inode* fd_inode;
};

/* 标准输入输出描述符 */
typedef enum std_fd
{
    /* 标准输入 */
    stdin_no,

    /* 标准输出 */
    stdout_no,

    /* 标准错误 */
    stderr_no
}std_fd;

/* 位图类型 */
typedef enum bitmap_type
{
    /* inode位图 */
    INODE_BITMAP,

    /* 块位图 */
    BLOCK_BITMAP
}bitmap_type;

/* 系统可打开的最大文件数 */
#define MAX_FILE_OPEN 32

#endif
