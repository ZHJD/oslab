#include "file.h"
#include "thread.h"
#include "stdio_kernel.h"
#include "ide.h"
#include "fs.h"
#include "string.h"

/* 文件表 */
struct file file_table[MAX_FILE_OPEN];

/* 从文件表file_table中获取一个空闲位，成功返回下标，失败返回-1 */
int32_t get_free_slot_in_global(void)
{
    uint32_t fd_idx = 3;
    while(fd_idx < MAX_FILE_OPEN)
    {
        if(file_table[fd_idx].fd_inode == NULL)
        {
            break;
        }
        fd_idx++;
    }
    if(fd_idx == MAX_FILE_OPEN)
    {
        printk("exceed max open files\n");
        return -1;
    }
    return fd_idx;
}

/* 将全局描述符下标安装到进程或线程自己的文件描述符数组fd_table中，成功返回下标，失败
    返回-1
 */
int32_t pcb_fd_install(int32_t global_fd_idx)
{
    task_struct* cur = get_running_thread_pcb();

    /* 跨过stdin, stdout, stderr */
    uint8_t local_fd_idx = 3;
    while(local_fd_idx < MAX_FILES_OPEN_PER_PROC)
    {
        if(cur->fd_table[local_fd_idx] == -1)
        {
            cur->fd_table[local_fd_idx] = global_fd_idx;
            break;
        }
        local_fd_idx++;
    }
    if(local_fd_idx == MAX_FILES_OPEN_PER_PROC)
    {
        printk("exceed max open files_per_proc\n");
        return -1;
    }
    return local_fd_idx;
}

/* 分配一个i结点，返回i结点号 */
int32_t inode_bitmap_alloc(partition* part)
{
    int32_t bit_idx = bitmap_scan(&part->inode_bitmap, 1);
    if(bit_idx == -1)
    {
        return -1;
    }
    bitmap_set(&part->inode_bitmap, bit_idx, 1);
    return bit_idx;
}

/* 分配1个扇区，返回其扇区地址 */
int32_t block_bitmap_alloc(partition* part)
{
    int32_t bit_idx = bitmap_scan(&part->block_bitmap, 1);
    if(bit_idx == -1)
    {
        return -1;
    }
    bitmap_set(&part->block_bitmap, bit_idx, 1);

    /* 和inode_bitmap_malloc不同，此处返回的不是位图索引，是具体的扇区地址 */
    return (part->sb->data_start_lba + bit_idx);
}

/* 将内存中bitmap第bit_idx位所在的512字节同步到硬盘 */
void bitmap_sync(partition* part, uint32_t bit_idx, uint8_t btmp)
{
    /* 512 * 8 = 4096 */
    uint32_t off_sec = bit_idx / 4096;
    uint32_t off_size = off_sec * BLOCK_SIZE;
    uint32_t sec_lba;
    uint8_t* bitmap_off;

    switch(btmp)
    {
    case INODE_BITMAP:
        sec_lba = part->sb->inode_bitmap_lba + off_sec;
        bitmap_off = part->inode_bitmap.bits + off_size;
        break;
    
    case BLOCK_BITMAP:
        sec_lba = part->sb->block_bitmap_lba + off_sec;
        bitmap_off = part->block_bitmap.bits + off_size;
        break;
    }
    ide_write(part->my_disk, sec_lba, bitmap_off, 1);
}


/* 创建文件，创建成功，返回文件描述符，失败则返回-1 */
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag)
{
    /* 后续操作的公共缓冲区 */
    void* io_buf = sys_malloc(1024);
    if(io_buf == NULL)
    {
        printk("in file_create: sys_malloc for io_buf failed\n");
        return -1;
    }
    /* 用于操作失败时回滚各资源状态 */
    uint8_t rollback_step = 0;

    /* 为新文件分配inode */
    int32_t inode_no = inode_bitmap_alloc(cur_part);
    if(inode_no == -1)
    {
        printk("in file create: allocate inode failed!\n");
        return -1;
    }

    /***************************************************************
     * inode需要在堆上分配，因为函数退出后不会释放
     */ 
    struct inode* new_file_inode = 
        (struct inode*)sys_malloc(sizeof(struct inode));
    
    if(new_file_inode == NULL)
    {
        printk("in file create: sys_malloc for inode failed\n");
        rollback_step = 1;
        goto rollback;
    }
    /* 初始化I结点 */
    inode_init(inode_no, new_file_inode);

    /* 返回file_table数组下标 */
    int fd_idx = get_free_slot_in_global();
    if(fd_idx == -1)
    {
        printk("exceed max open files \n");
        rollback_step = 2;
        goto rollback;
    }
    file_table[fd_idx].fd_inode = new_file_inode;
    file_table[fd_idx].fd_pos = 0;

    /* 文件操作类型 */
    file_table[fd_idx].fd_flag = flag;
    file_table[fd_idx].fd_inode->write_deny = false;

    struct dir_entry new_dir_entry;
    memset(&new_dir_entry, 0, sizeof(new_dir_entry));

    create_dir_entry(filename, inode_no, FT_REGULAR, &new_dir_entry);

    if(!sync_dir_entry(parent_dir, &new_dir_entry, io_buf))
    {
        printk("sync dir_entry to disk failed\n");
        rollback_step = 3;
        goto rollback;
    }

    memset(io_buf, 0, 1024);

    /* 添加目录项后的inode信息写入硬盘 */
    inode_sync(cur_part, parent_dir->inode, io_buf);

    memset(io_buf, 0, 1024);

    /* 把新文件的inode写入硬盘 */
    inode_sync(cur_part, new_file_inode, io_buf);

    /* inode_bitmap位图同步到硬盘 */
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    /* i结点加入到open_inode队列中 */
    list_push_back(&cur_part->open_inodes, &new_file_inode->inode_tag);
    new_file_inode->i_open_cnts = 1;

    sys_free(io_buf);

    rollback:
        switch(rollback_step)
        {
        case 3:
            memset(&file_table[fd_idx], 0, sizeof(struct file));
        case 2:
            sys_free(new_file_inode);
        case 1:
            bitmap_set(&cur_part->inode_bitmap, inode_no, 0);
            break;
        }
    /* 返回文件在pcb文件数组中的下标 */
    return pcb_fd_install(fd_idx);
}

/* 打开编号为inode_no的inode对应的文件,若成功则
返回文件描述符，否则返回-1 */
int32_t file_open(uint32_t inode_no, uint8_t flag)
{
    int fd_idx = get_free_slot_in_global();
    if(fd_idx == -1) 
    {
        printk("exceed max open files\n");
        return -1;
    }
    file_table[fd_idx].fd_inode = inode_open(cur_part, inode_no);

    /* 打开时文件内的指针指向开头 */
    file_table[fd_idx].fd_pos   = 0;
    file_table[fd_idx].fd_flag = flag;

    bool* write_deny = &file_table[fd_idx].fd_inode->write_deny;

    if(flag & O_WRONLY || flag & O_RDWR)
    {
        enum intr_status old_status = intr_disable();
        if(!(*write_deny))
        {
            *write_deny = true;
            set_intr_status(old_status);
        }
        else // 若其他进程在写，返回写失败
        {
            set_intr_status(old_status);
            printk("file can't be write now, try again later\n");
            return -1;
        }
    }
    /* 返回描述符 */
    return pcb_fd_install(fd_idx);
}





















































































