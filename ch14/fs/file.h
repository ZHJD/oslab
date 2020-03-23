#ifndef __FS_FILE_H
#define __FS_FILE_H
#include "stdint.h"
#include "ide.h"
#include "inode.h"
#include "interrupt.h"
#include "dir.h"

/* 文件结构 */
struct file
{
    /* 记录当前文件操作的偏移地址，以0为起始，最大为文件大小-1 */
    uint32_t fd_pos;

    /* 文件操作标识 */
    uint32_t fd_flag;

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

extern struct file file_table[MAX_FILE_OPEN];


/* 从文件表file_table中获取一个空闲位，成功返回下标，失败返回-1 */
int32_t get_free_slot_in_global(void);

/* 将全局描述符下标安装到进程或线程自己的文件描述符数组fd_table中，成功返回下标，失败
    返回-1
 */
int32_t pcb_fd_install(int32_t global_fd_idx);

/* 分配一个i结点，返回i结点号 */
int32_t inode_bitmap_alloc(partition* part);

/* 分配1个扇区，返回其扇区地址 */
int32_t block_bitmap_alloc(partition* part);

/* 将内存中bitmap第bit_idx位所在的512字节同步到硬盘 */
void bitmap_sync(partition* part, uint32_t bit_idx, uint8_t btmp);

/* 创建文件，创建成功，返回文件描述符，失败则返回-1 */
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag);

/* 打开编号为inode_no的inode对应的文件,若成功则
返回文件描述符，否则返回-1 */
int32_t file_open(uint32_t inode_no, uint8_t flag);

/* 关闭文件 */
int32_t file_close(struct file* file);
#endif
