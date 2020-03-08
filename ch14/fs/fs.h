#ifndef __FS_FS_H
#define __FS_FS_H

/* 每个分区所支持的最大创建的文件数 */
#define MAX_FILES_PER_PART   4096

/* 扇区字节大小 */
#define SECTOR_SIZE           512

/* 每扇区的位数 */
#define BITS_PER_SECTOR    SECTOR_SIZE * 8

#define BLOCK_SIZE    SECTOR_SIZE

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


void filesys_init();


#endif
