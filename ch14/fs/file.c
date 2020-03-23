#include "file.h"
#include "thread.h"
#include "stdio_kernel.h"
#include "ide.h"
#include "fs.h"
#include "string.h"
#include "debug.h"

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

/* 关闭文件 */
int32_t file_close(struct file* file)
{
    if(file == NULL)
    {
        return -1;
    }
    /* 始文件结构可用 */
    file->fd_inode->write_deny = false;
    inode_close(file->fd_inode);
    file->fd_inode = NULL;

    return 0;
}

/* 把buf中的count个字节写入file，成功则返回写入的字节数，失败返回-1 */
int32_t file_write(struct file* file, const void* buf, uint32_t count)
{
    if(file->fd_inode->i_size + count > (BLOCK_SIZE * 140)) 
    {
        printk("exceed max file size, write file failed \n");
        return -1;
    }

    uint8_t* io_buf = (uint8_t*)sys_malloc(BLOCK_SIZE);
    if(io_buf == NULL)
    {
        printk("file write failed!, sys_malloc for io_buf failed!\n");
        return -1;
    }
    uint32_t* all_blocks = (uint32_t*)sys_malloc(BLOCK_SIZE + 48);
    if(all_blocks == NULL)
    {
        printk("file write: sys_malloc for all_blocks failed\n");
        return -1;
    }

    /* 使用src指向buf */
    const uint8_t* src = buf;

    /* 记录已经写入的数据 */
    uint32_t bytes_written = 0;
    uint32_t size_left = count;

    /* 块地址 */
    uint32_t block_lba = -1;
    
    /* 用来记录block对应于block_bitmap中的索引，作为参数传递给bitmap_sync */
    uint32_t block_bitmap_idx = 0;

    uint32_t sec_idx; // 用来索引扇区
    uint32_t sec_lba; // 扇区地址
    uint32_t sec_off_bytes; // 扇区内字节偏移量
    uint32_t sec_left_bytes; // 扇区内剩余字节量
    uint32_t chunk_size; // 每次写入硬盘的数据块大小
    int32_t indirect_block_table; // 一级间接表地址
    uint32_t block_idx;  // 块索引

    /* 判断文件是否是第一次写入 */
    if(file->fd_inode->i_sectors[0] == 0)
    {
        block_lba = block_bitmap_alloc(cur_part);
        if(block_lba == -1)
        {
            printk("file write: block_bitmap_alloc failed!\n");
            return -1;
        }
        file->fd_inode->i_sectors[0] = block_lba;

        /* 每分配一个块就把位图同步到硬盘 */
        block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
        ASSERT(block_bitmap_idx >= 0);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
    }

    /* 文件已经占用的块数 */
    uint32_t file_has_used_blocks = file->fd_inode->i_size / BLOCK_SIZE + 1;

    /* 存储count个字节后该文件占用的块数 */
    uint32_t file_will_use_blocks = (file->fd_inode->i_size + count) / BLOCK_SIZE + 1;

    ASSERT(file_will_use_blocks <= 140);

    /* 如果为0则，不需要再分配扇区 */
    uint32_t add_blocks = file_will_use_blocks - file_has_used_blocks;

    /* 不需要分配扇区 */
    if(add_blocks == 0)
    {
        /* 不需要使用间接块 */
        if(file_will_use_blocks <= 12)
        {
            block_idx = file_has_used_blocks - 1;
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        }
        else
        {
            /* 已经占用了一级链接 */
            ASSERT(file->fd_inode->i_sectors[12] != 0);
            indirect_block_table = file->fd_inode->i_sectors[12];
            /* 把一级间接从磁盘读出写入all_blocks */
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);
        }
    }
    else // 要分配扇区
    {
        /* 12个块足够 */
        if(file_will_use_blocks <= 12)
        {
            /* 先把有剩余空间的可以使用的扇区地址写入all_blocks写入 */
            block_idx = file_has_used_blocks - 1;
            /* 初次写入未分配的情况已经处理 */
            ASSERT(file->fd_inode->i_sectors[block_idx] != 0);
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            /* 未来要用的扇区写入all_blocks */
            block_idx = file_has_used_blocks;
            while(block_idx < file_will_use_blocks)
            {
                block_lba = block_bitmap_alloc(cur_part);
                if(block_lba == -1)
                {
                    printk("file write: block_bitmap_alloc for situation 1 failed!\n");
                    return -1;
                }
                /* block_idx对应的必须的尚未分配 */
                ASSERT(file->fd_inode->i_sectors[block_idx] == 0);
                file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] =
                    block_lba;
                /* 每分配一个块就把位图同步到硬盘 */
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx >= 0);
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

                /* 下一个分配的新扇区 */              
                block_idx++;
            }
        }
        else if(file_has_used_blocks <= 12 && file_will_use_blocks > 12)
        {
            /* 先把有剩余空间的可以使用的扇区地址写入all_blocks写入 */
            block_idx = file_has_used_blocks - 1;
            /* 初次写入未分配的情况已经处理 */
            ASSERT(file->fd_inode->i_sectors[block_idx] != 0);
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];

            /* 创建一级间接块表 */
            block_lba = block_bitmap_alloc(cur_part);
            if(block_lba == -1)
            {
                printk("file write: block_bitmap_alloc for situation 2  \
                        failed!\n");
                return -1;
            }

            /* 一级间接为空 */
            ASSERT(file->fd_inode->i_sectors[12] == 0);

            /* 分配一级间接表 */
            indirect_block_table = file->fd_inode->i_sectors[12] = block_lba;
            block_idx = file_has_used_blocks;
            while(block_idx < file_will_use_blocks)
            {
                block_lba = block_bitmap_alloc(cur_part);
                if(block_lba == -1)
                {
                    printk("file write: block_bitmap_alloc for situation 2 \
                        failed!\n");
                    return -1;
                }
                /* 若新键的是 0 - 11 块，直接存入all_blocks数组 */
                if(block_idx < 12)
                {
                    /* block_idx对应的必须的尚未分配 */
                    ASSERT(file->fd_inode->i_sectors[block_idx] == 0);
                    file->fd_inode->i_sectors[block_idx] = all_blocks[block_idx] =
                        block_lba;
                }
                else
                {
                    all_blocks[block_idx] = block_lba;
                }
                /* 每分配一个块就把位图同步到硬盘 */
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx >= 0);
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                /* 下一个分配的新扇区 */              
                block_idx++;
            }
            /* 同步间接块 */
            ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);
        }
        else if(file_has_used_blocks > 12)
        {
             /* 一级间接为空 */
            ASSERT(file->fd_inode->i_sectors[12] != 0);

            /* 获取一级间接表 */
            indirect_block_table = file->fd_inode->i_sectors[12];       

            /* 第一个未使用完的地址已经读入 */
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);
            block_idx = file_has_used_blocks;
            while(block_idx < file_will_use_blocks)
            {
                block_lba = block_bitmap_alloc(cur_part);
                if(block_lba == -1)
                {
                    printk("file write: block_bitmap_alloc for situation 2 \
                        failed!\n");
                    return -1;
                }
                all_blocks[block_idx] = block_lba;
                /* 每分配一个块就把位图同步到硬盘 */
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx >= 0);
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                /* 下一个分配的新扇区 */              
                block_idx++;
            }
            /* 同步间接块 */
            ide_write(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);    
        }
    }
    
    /* 用到的块地址已经完全收录,开始写数据 */
    bool first_write_block = true; // 第一次写的扇区有剩余空间

    /* 文件指针位置设置为文件大小 - 1 */
    file->fd_pos = file->fd_inode->i_size - 1;
    while(bytes_written < count)
    {
        memset(io_buf, 0, sizeof(io_buf));

        /* 文件内的偏移扇区 */
        sec_idx = file->fd_inode->i_size / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_inode->i_size % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;

        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;

        if(first_write_block)
        {
            ide_read(cur_part->my_disk, sec_lba, io_buf, 1);
            first_write_block = false;
        }
        memcpy(io_buf + sec_off_bytes, src, chunk_size);
        ide_write(cur_part->my_disk, sec_lba, io_buf, 1);
        src += chunk_size;

        /* 更新文件大小 */
        file->fd_inode->i_size += chunk_size;
        file->fd_pos += chunk_size;
        bytes_written += chunk_size;
        size_left += chunk_size;
    }

    inode_sync(cur_part, file->fd_inode, io_buf);
    sys_free(all_blocks);
    sys_free(io_buf);
    return bytes_written;
}

/* 从文件中读取count个字节写入buf中，返回读出的字节数， 
   若到文件结束，则返回-1
 */
int32_t file_read(struct file* file, void* buf, uint32_t count)
{
    uint8_t* buf_dst = (uint8_t*)buf;
    uint32_t size = count;
    uint32_t size_left = size;

    /* 若要读取的字节数超过了文件可读的剩余量, 能用剩余量作为待读取的字节数 */
    if((file->fd_pos + count) > file->fd_inode->i_size)
    {
        size = file->fd_inode->i_size - file->fd_pos;
        size_left = size;
        if(size == 0) // 若文件为空，则返回-1
        {
            return -1;
        }
    }
    uint8_t* io_buf = (uint8_t*)sys_malloc(BLOCK_SIZE);
    if(io_buf == NULL)
    {
        printk("file read: sys_malloc for io_buf failed\n");
    }
    uint32_t* all_blocks = (uint32_t*)sys_malloc(BLOCK_SIZE + 48);

    if(all_blocks == NULL)
    {
        printk("file_read: sys_malloc for all_blocks failed\n");
        return -1;
    }
    /* 起始 */
    uint32_t block_read_start_idx = file->fd_pos / BLOCK_SIZE;
    /* 终止 */
    uint32_t block_read_end_idx = (file->fd_pos + size) / BLOCK_SIZE;

    /* 若为0， 表示在同一扇区内 */
    uint32_t read_blocks = block_read_end_idx - block_read_start_idx;

    int32_t indirect_block_table;
    uint32_t block_idx = 0;

    if(read_blocks == 0)
    {
        /* 若在12扇区内 */
        if(block_read_end_idx < 12)
        {
            block_idx = block_read_end_idx;
            all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
        }
        else
        {
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);
        }        
    }
    else // 多个扇区
    {
        /* 若在12扇区内 */
        if(block_read_end_idx < 12)
        {
            block_idx = block_read_start_idx;
            while(block_idx <= block_read_end_idx)
            {
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
        }
        else if(block_read_start_idx < 12 && block_read_end_idx >= 12)
        {
            block_idx = block_read_start_idx;
            while(block_idx < 12)
            {
                all_blocks[block_idx] = file->fd_inode->i_sectors[block_idx];
                block_idx++;
            }
            ASSERT(file->fd_inode->i_sectors[12] != 0);     
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);      
        }
        else
        {
            ASSERT(file->fd_inode->i_sectors[12] != 0);     
            indirect_block_table = file->fd_inode->i_sectors[12];
            ide_read(cur_part->my_disk, indirect_block_table, all_blocks + 12, 1);   
        }
    }

    /* 要读取的块地址都已经准备好，开始读取 */
    uint32_t sec_idx;
    uint32_t sec_lba;
    uint32_t sec_off_bytes;
    uint32_t sec_left_bytes;
    uint32_t chunk_size;
    uint32_t bytes_read = 0;
    while(bytes_read < size)
    {
        sec_idx = file->fd_pos / BLOCK_SIZE;
        sec_lba = all_blocks[sec_idx];
        sec_off_bytes = file->fd_pos % BLOCK_SIZE;
        sec_left_bytes = BLOCK_SIZE - sec_off_bytes;
        chunk_size = size_left < sec_left_bytes ? size_left : sec_left_bytes;

        ide_read(cur_part->my_disk, sec_lba, io_buf, 1);
        memcpy(buf_dst, io_buf + sec_off_bytes, chunk_size);
        buf_dst += bytes_read;
        file->fd_pos += chunk_size;
        bytes_read += chunk_size;
        size_left -= chunk_size;
    }
    sys_free(all_blocks);
    sys_free(io_buf);
    
    return bytes_read;
}

















































































