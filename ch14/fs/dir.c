#include "dir.h"
#include "ide.h"
#include "file.h"
#include "debug.h"
#include "string.h"
#include "bitmap.h"
#include "stdio_kernel.h"
#include "file.h"

/* 根目录 */
struct dir root_dir;

/* 打开根目录 */
void open_root_dir(partition* part)
{
    root_dir.inode = inode_open(part, part->sb->root_inode_no);
    root_dir.dir_pos = 0; 
}

 /* 在分区part打开i结点为inode_no的目录并返回目录指针 */
 struct dir* dir_open(partition* part, uint32_t inode_no)
 {
     struct dir* pdir = (struct dir*)sys_malloc(sizeof(struct dir));
     pdir->inode = inode_open(part, inode_no);
     pdir->dir_pos = 0;
     return pdir;
 }

 /* 在part分区内的pdir目录内寻找名字为name的文件或目录,找到后返回
    true并将其目录项存入dir_e,否则返回false
*/
 bool search_dir_entry(partition* part, struct dir* pdir,
                      const char* name, struct dir_entry* dir_e)
 {
     /* 12个直接块，128个一级间接块 = 140块 */
     uint32_t block_cnt = 140;

    /* 12 * 4 + 512 共这么多的扇区号 */
    uint32_t* all_blocks = (uint32_t*)sys_malloc(48 + 512);
    if(all_blocks == NULL)
    {
        printk("search_dir_entry: sys_malloc for all_blocks failed");
        return false;
    }
    uint32_t block_idx = 0;
    while(block_idx < 12)
    {
        all_blocks[block_idx] = pdir->inode->i_sectors[block_idx];
    }

    /* 若含有一级间接块表 */
    if(pdir->inode->i_sectors[12] != 0)
    {
        /* all_blocks是指针,加上12相当于12 * 4 */
        ide_read(part->my_disk, pdir->inode->i_sectors[12], all_blocks + 12, 1);
    }

    uint8_t* buf = (uint8_t*)sys_malloc(SECTOR_SIZE);

    /* p_de为指向目录项的指针 */
    struct dir_entry* p_de = (struct dir_entry*)buf;
    
    uint32_t dir_entry_size = part->sb->dir_entry_size;

    /* 一个扇区内容纳的目录项个数 */
    uint32_t dir_entry_cnt = SECTOR_SIZE / dir_entry_size;
    
    /* 开始在所有块中查找目录项 */
    while(block_idx < block_cnt)
    {
        /* 块地址为0表示该块没有数据，继续在其它目录中找 */
        if(all_blocks[block_idx] == 0)
        {
            block_idx++;
            continue;
        }

        /* 读入一个扇区的目录项或者文件名 */
        ide_read(part->my_disk, all_blocks[block_idx], buf, 1);
        uint32_t dir_entry_idx = 0;

        /* 遍历扇区中所有的目录项 */
        while(dir_entry_idx < dir_entry_cnt)
        {
            /* 若找到了，就复制整个目录项 */
            if(!strcmp(p_de->filename, name))
            {
                memcpy(dir_e, p_de, dir_entry_size);
                sys_free(buf);
                sys_free(all_blocks);
                return true;
            }
            dir_entry_idx++;
            p_de++;
        }
        block_idx++;
        p_de = (struct dir_entry*)buf;
        memset(buf, 0, SECTOR_SIZE);
    }
    sys_free(buf);
    sys_free(all_blocks);
    return false;
 }


/* 把目录项p_de写入父目录parent_dir中，io_buf由主调函数提供 */
bool sync_dir_entry(struct dir* parent_dir, struct dir_entry* p_de,
                    struct partition* part,
                    void* io_buf)
{
    struct inode* dir_inode = parent_dir->inode;
    uint32_t dir_size = dir_inode->i_size;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;

    /* dir_size应该是dir_entry_size的整数倍 */
    ASSERT(dir_size % dir_entry_size == 0);

    /* 每扇区最大的目录项数目 */
    uint32_t dir_entrys_per_sec = (SECTOR_SIZE / dir_entry_size);

    int32_t block_lba = -1;

    /* 将该目录的所有扇区地址(12个直接块, 128个间接块)存入all_blocks */
    uint8_t block_idx = 0;
    /* all_blocks保存目录所有的块 */
    uint32_t all_blocks[140] = {0};

    /* 将12个直接存入all_blocks */
    while(block_idx < 12)
    {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
        block_idx++;
    }

    /* dir_e用来在io_buf中遍历目录项 */
    struct dir_entry* dir_e = (struct dir_entry*)io_buf;

    int32_t block_bitmap_idx = -1;
    block_idx = 0;
    while(block_idx < 140)
    {
        block_bitmap_idx = -1;

        if(all_blocks[block_idx] == 0)
        {
            block_lba = block_bitmap_alloc(cur_part);
            if(block_lba == -1)
            {
                printk("alloc block bitmap for sync_dir_entry failed \n");
                return false;
            }
            /* 每分割一个块被同步一次block_bitmap */
            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
            ASSERT(block_bitmap_idx != -1);
            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

            block_bitmap_idx = -1;
            if(block_idx < 12) // 如果是直接块
            {
                dir_inode->i_sectors[block_idx] = 
                all_blocks[block_idx] = block_lba;
            }
            /*************************************************************
            * 若是尚未分配一级间接块表(block_idx等于12表示第0个间接块地址为0)
            */ 
            else if(block_idx == 12)
            {
                dir_inode->i_sectors[12] = block_lba;
                /* 分配的块是一级间接块表地址 */
                block_lba = -1;
                block_lba = block_bitmap_alloc(cur_part);
                /* 再分配一个块作为第0个间接块 */
                if(block_lba == -1)
                {
                    /* 回退已经分配的一个间接块 */

                    block_bitmap_idx = dir_inode->i_sectors[12] - 
                        cur_part->sb->data_start_lba;
                    bitmap_set(&cur_part->block_bitmap, block_bitmap_idx, 0);
                    dir_inode->i_sectors[12] = 0;
                    printk("alloc block bitmap for sync_dir_entry failed \n");
                    return false;
                }
                /* 每分配一个块就同步一次block_bitmap */
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx != -1);
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

                /* 存入的是真实数据地址 */
                all_blocks[12] = block_lba; 
                /* 把新分配的第0个间接块地址写入一级间接块表 */
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12],
                        all_blocks + 12, 1);
                
            }
            else // 若是间接块未分配
            {
                all_blocks[block_idx] = block_lba;

                /* 直接使用内存中all_blocks + 12处的内容进行覆盖 */
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1);
            }
            /* 再将新目录项p_de写入新分配的间接块 */
            memset(io_buf, 0, 512);
            memcpy(io_buf, p_de, dir_entry_size);
            ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
            dir_inode->i_size += dir_entry_size;
            return true;            
        }

        /* 若第block_idx已经存在，将其读进内存，然后在这块中查找空目录项 */
        ide_read(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
        /* 在扇区内查找空目录项 */
        for(uint8_t dir_entry_idx = 0; dir_entry_idx < dir_entrys_per_sec; dir_entry_idx++)
        {
            /* 如果该目录为空，则写入 */
            if((dir_e + dir_entry_idx)->f_type == FT_UNKNOWN)
            {
                memcpy(dir_e + dir_entry_idx, p_de, dir_entry_size);
                ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
                dir_inode->i_size += dir_entry_size;
                return true;
            }
        }
        block_idx++;
    }
    printk("directory is full\n");
    return false;
}   

/* 关闭目录 */
void dir_close(struct dir* dir)
{
/***********  根目录不能关闭   ***************
 * 1、根目录自打开后不应该关闭
 * 2、root_dir所在的内存是低端1MB内存，不是在堆内
 */
    if(dir == &root_dir)
    {
    /* 对于根目录，不做任何处理 */
        return;
    }
    inode_close(dir->inode);
    sys_free(dir);
}

/* 在内存中初始化目录项p_de */
void create_dir_entry(char* filename, uint32_t inode_no, uint8_t file_type,
    struct dir_entry* p_de)
{
    ASSERT(strlen(filename) <= MAX_FILE_NAME_LEN);

    /* 初始化目录项 */
    memcpy(p_de->filename, filename, strlen(filename));
    p_de->i_no = inode_no;
    p_de->f_type = file_type;
}


