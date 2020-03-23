#ifndef __FS_FS_H
#define __FS_FS_H
#include "ide.h"
#include "stdint.h"



/* 系统可打开的最大文件数 */
#define MAX_FILE_OPEN 32

/* 每个分区所支持的最大创建的文件数 */
#define MAX_FILES_PER_PART   4096

/* 扇区字节大小 */
#define SECTOR_SIZE           512

/* 每扇区的位数 */
#define BITS_PER_SECTOR    SECTOR_SIZE * 8

#define BLOCK_SIZE    SECTOR_SIZE

#define MAX_PATH_LEN 512


extern struct partition* cur_part;

/* 文件类型 */
typedef enum file_types
{
    /* 不支持的文件类型 */
    FT_UNKNOWN,

    /* 普通文件 */
    FT_REGULAR,

    /* 目录 */
    FT_DIRECTORY
}file_types;

/* 打开文件的选项 */
typedef enum oflags
{
    O_RDONLY,
    O_WRONLY,
    O_RDWR,
    O_CREAT = 4
}oflags;

typedef struct path_search_record
{
    /* 查找过程中的父路径 */
    char searched_path[MAX_PATH_LEN];

    /* 文件或目录所在的直接父目录 */
    struct dir* parent_dir;

    /* 找到的是谱图文件，还是目录,找不到将为未知类型(FT_UNKNOWN) */
    file_types file_type;
}path_search_record;

void filesys_init();

/* 打开或者创建文件成功后，返回文件描述符，否则返回-1 */
int32_t sys_open(const char* pathname, uint8_t flags);

/* 关闭文件描述符fd指向的文件，成功返回0,失败返回-1 */
int32_t sys_close(int32_t fd);

int32_t sys_write(int32_t fd, const void* buf, uint32_t count);

#endif
