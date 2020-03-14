#ifndef __FS_DIR_H
#define __FS_DIR_H 
#include "stdint.h"
#include "inode.h"
#include "fs.h"

/* 最大文件名长度 */
#define MAX_FILE_NAME_LEN  16




/* 目录结构 */
struct dir
{
    struct inode* inode;

    /* 记录在目录内的偏移 */
    uint32_t dir_pos;

    /* 目录的数据缓存 */
    uint8_t dir_buf[512];
};

struct dir dir;

extern struct dir root_dir;

/* 目录项结构 */
typedef struct dir_entry
{
    char filename[MAX_FILE_NAME_LEN];

    /* 普通文件或目录对应的inode编号 */
    uint32_t i_no;

    /* 文件类型 */
    file_types f_type;
}dir_entry;

/* 把目录项p_de写入父目录parent_dir中，io_buf由主调函数提供 */
bool sync_dir_entry(struct dir* parent_dir, struct dir_entry* p_de, void* io_buf);


 /* 在part分区内的pdir目录内寻找名字为name的文件或目录,找到后返回
    true并将其目录项存入dir_e,否则返回false
*/
bool search_dir_entry(partition* part, struct dir* pdir,
                      const char* name, struct dir_entry* dir_e);


 /* 在分区part打开i结点为inode_no的目录并返回目录指针 */
struct dir* dir_open(partition* part, uint32_t inode_no);

/* 打开根目录 */
void open_root_dir(partition* part);

/* 关闭目录 */
void dir_close(struct dir* dir);

/* 在内存中初始化目录项p_de */
void create_dir_entry(char* file_name, uint32_t inode_no, uint8_t file_type,
    struct dir_entry* p_de);
#endif
