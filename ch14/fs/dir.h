#ifndef __FS_DIR_H
#define __FS_DIR_H
#include "stdint.h"
#include "inode.h"
#include "fs.h"

/* 最大文件名长度 */
#define MAX_FILE_NAME_LEN  16




/* 目录结构 */
typedef struct dir
{
    inode i_node;

    /* 记录在目录内的偏移 */
    uint32_t dir_pos;

    /* 目录的数据缓存 */
    uint8_t dir_buf[512];
}dir;

/* 目录项结构 */
typedef struct dir_entry
{
    char filename[MAX_FILE_NAME_LEN];

    /* 普通文件或目录对应的inode编号 */
    uint32_t i_no;

    /* 文件类型 */
    file_types f_type;
}dir_entry;







#endif
