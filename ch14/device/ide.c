#include "ide.h"
#include "stdio_kernel.h"
#include "debug.h"
#include "io.h"
#include "timer.h"
#include "interrupt.h"

/* 定义硬盘各寄存器的端口号 */
#define reg_data(channel)       (channel->port_base + 0)
#define reg_error(channel)      (channel->port_base + 1)
#define reg_sect_cnt(channel)   (channel->port_base + 2)
#define reg_lba_l(channel)      (channel->port_base + 3)
#define reg_lba_m(channel)      (channel->port_base + 4)
#define reg_lba_h(channel)      (channel->port_base + 5)
#define reg_dev(channel)        (channel->port_base + 6)
#define reg_status(channel)     (channel->port_base + 7)
#define reg_cmd(channel)        reg_status(channel)
#define reg_alt_status(channel) (channel->port_base + 0x206)

/* 同一个寄存器读出和写入时的功能不同 */
#define reg_ctl(channel)        ret_alt_status(channel)


/* reg_alt_status 寄存器的一些关键位 */
#define BIT_ALT_STAT_BUSY        0x80    // 硬盘忙
#define BIT_ALT_STAT_DRDY        0x40    // 驱动器准备好
#define BIT_ALT_STAT_DRQ         0x8     // 数据传输准备好

/* device寄存器的一些关键位 */
#define BIT_DEV_MBS              0xa0    // 第七位和第五位固定为1
#define BIT_DEV_LBA              0x40    // 
#define BIT_DEV_DEV              0x10

/* 一些硬盘操作的指令 */
#define CMD_IDENtIFY            0xec     // identify 指令
#define CMD_READ_SECTOR         0x20     // 读扇区指令
#define CMD_WRItE_SECTOR        0x30     // 写扇区指令

/* 定义可以读写的最大扇区数，调试使用 */
#define max_lba ((80 * 1024 * 1024 / 512) - 1)

/* 按硬盘数计算通道数 */
uint8_t channel_cnt;

/* 两个ide通道 */
ide_channel channels[2];

/* 用于记录总扩展分区的起始lba, 初始为0，partition_scan 时以此为标记 */
static int32_t ext_lba_base = 0;

/* 记录硬盘主分区和逻辑分区的下标 */
static uint8_t p_no = 0;

static uint8_t l_no = 0;

/* 分区队列 */
list partition_list;

struct partition_table_entry
{
    /* 是否可引导 */
    uint8_t bootable;

    /* 起始磁头号 */
    uint8_t start_head;

    /* 起始扇区号 */
    uint8_t start_sec;

    /* 起始柱面号 */
    uint8_t start_chs;

    /* 分区类型 */
    uint8_t fs_type;

    /* 结束磁头号 */
    uint8_t end_head;

    /* 结束扇区号 */
    uint8_t end_sec;

    /* 结束柱面号 */
    uint8_t end_chs;

    /* 本分区起始扇区的lba地址 */
    uint32_t start_lba;

    /* 本分区的扇区数目 */
    uint32_t sec_cnt;
}__attribute__((packed));  // 保证此结构16字节大小

typedef struct partition_table_entry partition_table_entry;

struct boot_sector
{
    /* 引导代码 */
    uint8_t other[446];
    partition_table_entry partition_table[4];

    /* 启动扇区的结束标志是0x55 0xaa */
    uint16_t signature;
}__attribute__((packed));

typedef struct boot_sector boot_sector;

/* 将dst中len个相邻字节交换位置后存入buf */
static void swap_pairs_bytes(const char* dst, char* buf, uint32_t len)
{
    uint8_t idx = 0;
    for(idx = 0; idx < len; idx++)
    {
        /* buf中存储dst */
        buf[idx + 1] = *dst++;
        buf[idx] = *dst++;
    }
    buf[idx] = '\0';
}

/* 选择读写的硬盘 */
static void select_disk(disk* hd)
{
    /* 启用lba */
    uint8_t reg_device = BIT_DEV_MBS | BIT_DEV_LBA;
    /* 从盘dev位置1 */
    if(hd->dev_no == 1)
    {
        reg_device |= BIT_DEV_DEV;
    }
    /* 通道分主从，区别在端口号不同，硬盘也分主从,区别在device寄存器 */
    outb(reg_dev(hd->my_channel), reg_device);
}

/* 向控制器写入要读取和写入的起始扇区地址以及要读写的扇区数 */
static void select_sector(disk* hd, uint32_t lba, uint8_t sec_cnt)
{
    ASSERT(lba <= max_lba);
    ide_channel* channel = hd->my_channel;

    /* 写入要读写的扇区数 */
    outb(reg_sect_cnt(hd->my_channel), sec_cnt);

    /* 写入lba地址 */
    outb(reg_lba_l(hd->my_channel), lba);
    outb(reg_lba_m(hd->my_channel), lba >> 8);
    outb(reg_lba_h(hd->my_channel), lba >> 16);
    
    uint8_t reg_device = BIT_DEV_MBS | BIT_DEV_LBA |
            (hd->dev_no == 1 ? BIT_DEV_DEV : 0) |
            lba >> 24;
    
    /* lba的第23-d7位位于device的第0-3位 */
    outb(reg_dev(hd->my_channel), reg_device);
}

/* 向channel发命令 */
static void cmd_out(ide_channel* channel, uint8_t cmd)
{
    /* 只要发出命令就置为true */
    channel->expecting_intr = true;
    outb(reg_cmd(channel), cmd);
}

/* 磁盘读入sec_cnt个扇区的数据到buf中 */
static read_from_sector(disk* hd, void* buf, uint8_t sec_cnt)
{
    uint32_t size_in_byte;

    /* 为0表示写入256个扇区 */
    if(sec_cnt == 0)
    {
        size_in_byte = 256 * 512;
    }
    else
    {
        size_in_byte = sec_cnt * 512;
    }
    /* 写入的单位是字 */
    insw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

/* 将 buf 中 sec_cnt 扇区的数据写入硬盘 */
static void write2sector(disk* hd, void* buf, uint8_t sec_cnt)
{
    uint32_t size_in_byte;
    if(sec_cnt == 0)
    {
        size_in_byte = 256 * 512;
    }
    else
    {
        size_in_byte = sec_cnt * 512;
    }
    outsw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

/* 等待30s */
static bool busy_wait(disk* hd)
{
//    printk(" busy_wait ");
    ide_channel* channel = hd->my_channel;
    uint16_t time_limit = 30 * 1000;
    for(; time_limit >= 0; time_limit -= 10)
    {
        if(!(inb(reg_status(hd->my_channel)) & BIT_ALT_STAT_BUSY))
        {
            return inb(reg_status(hd->my_channel)) & BIT_ALT_STAT_DRQ;
        }
        else
        {
            /* 睡眠10毫秒*/
            mtime_sleep(10);
        }
    }
//    printk("disk not ready!\n");
    return false;
}

/* 获得硬盘参数信息 */
static void identify_disk(disk* hd)
{
    char id_info[512];
    select_disk(hd);

    /* 获取硬盘参数 */
    cmd_out(hd->my_channel, CMD_IDENtIFY);

    /* 发出命令后阻塞自己，等到硬盘工作完毕唤醒自己 */
    sema_down(&hd->my_channel->disk_done);

    if(!busy_wait(hd))
    {
        char error[64];
        sprintf(error, "%s identify failed||||||\n", hd->name);
        PANIC(error);
    }

    printk("sema pu\n");

    /* 返回identify命令返回的参数信息 */
    read_from_sector(hd, id_info, 1);
    

    char buf[64];

    /* 硬盘序列号，长度为20的字符串，偏移字节为20 */
    uint8_t sn_start = 10 * 2;

    /* 长度为20 */
    uint8_t sn_len = 20;

    /* 硬盘型号 */
    uint8_t md_start = 27 * 2;

    /* 字节数 */
    uint8_t md_len = 40;

    swap_pairs_bytes(&id_info[sn_start], buf, sn_len);

    printk("  disk %s info;\n  SN: %s\n", hd->name, buf);
    memset(buf, 0, sizeof(buf));
    swap_pairs_bytes(&id_info[md_start], buf, md_len);
    printk(" MODULE: %s \n", buf);

    /* 可供用户使用的扇区数 */
    uint32_t sectors = *(uint32_t*)&id_info[60 * 2];
    printk(" SECTORS: %d\n", sectors);
    printk(" CAPACITY: %dMB\n", sectors * 512 / 1024 / 1024);
}

/* 扫描分区 */
static void partition_scan(disk* hd, uint32_t ext_lba)
{
    /* 避免栈溢出 */
    boot_sector* bs = sys_malloc(sizeof(boot_sector));
    
    ASSERT(bs != 0);

    ide_read(hd, ext_lba, bs, 1);
    partition_table_entry* p = bs->partition_table;

    /* 遍历分区表4个分区表项 */
    for(uint8_t part_idx = 0; part_idx < 4; part_idx++)
    {   
        if(p->fs_type == 0x5) // 若为扩展分区
        {
            if(ext_lba_base != 0) // 
            {
                /* 子扩展分区的lba地址相对于总扩展分区的lba地址 */
                partition_scan(hd, p->start_lba + ext_lba_base);
            }
            else
            {
                /* 赋值为总扩展分区地址 */
                ext_lba_base = p->start_lba;
                partition_scan(hd, p->start_lba);
            }
        }
        else if(p->fs_type != 0) // 若是有效的分区类型
        {
            if(ext_lba == 0) // 此时是主分区
            {
                hd->prim_parts[p_no].start_lba = ext_lba + p->start_lba;
                hd->prim_parts[p_no].sec_cnt = p->sec_cnt;
                hd->prim_parts[p_no].my_disk = hd;
                list_push_back(&partition_list, &hd->prim_parts[p_no].part_tag);
                sprintf(hd->prim_parts[p_no].name, "%s%d", hd->name, p_no + 1);
                p_no++;
                ASSERT(p_no < 4);  // 0, 1, 2, 3
            }
            else
            {
                hd->logic_parts[l_no].start_lba = ext_lba + p->start_lba;
                hd->logic_parts[l_no].sec_cnt = p->sec_cnt;
                hd->logic_parts[l_no].my_disk = hd;
                list_push_back(&partition_list, &hd->logic_parts[l_no].part_tag);
                sprintf(hd->logic_parts[l_no].name, "%s%d", hd->name, l_no + 5);
                l_no++;
                if(l_no >= 5)
                {
                    return;
                }
            }
        }
        p++;
    }
    sys_free(bs);
}

/* 打印分区信息，作为回调函数使用 */
static partition_info(list_elem* pelem, int arg UNUSED)
{
    partition* part = elem2entry(partition, part_tag, pelem);
    printk(" %s start_lba; 0x%x, sec_cnt: 0x%x\n", part->name, part->start_lba, part->sec_cnt);

    return false;
}

/* 从硬盘读取sec_cnt个扇区到buf中 */
void ide_read(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt)
{
    lock_acquire(&hd->my_channel->channel_lock);

    /* 选择硬盘 */
    select_disk(hd);

    /* 每次操作的扇区数 */
    uint32_t secs_op;

    /* 已完成的扇区数 */
    uint32_t secs_done = 0;

    /* 每次最多写入256个扇区 */
    while(secs_done < sec_cnt)
    {
        if(secs_done + 256 <= sec_cnt)
        {
            secs_op = 256;
        }
        else
        {
            secs_op = sec_cnt - secs_done;
        }

        /* 写入待写入的扇区数和起始扇区号 */
        select_sector(hd, lba + secs_done, secs_op);

        /* 执行的命令写入reg_cmd寄存器 */
        cmd_out(hd->my_channel, CMD_READ_SECTOR);

        /************* 阻塞自己 ***********************
         * 硬盘已经开始工作，阻塞自己，等待硬盘工作完成，发出
         * 中断，唤醒线程
         */
        sema_down(&hd->my_channel->disk_done);

        /* 检测硬盘状态是否可以读 */
        if(!busy_wait(hd))
        {
            char error[64];
            sprintf(error, "%s read sector %d failed|||\n", hd->name, lba);
            PANIC(error);
        } 

        /* 把数据从硬盘缓冲区读出 */
        read_from_sector(hd, (void*)((uint32_t)buf + secs_done * 512),
                        secs_op);
        
        secs_done += secs_op;
    }
    lock_release(&hd->my_channel->channel_lock);
}

/* 将 buf 中 sec_cnt 扇区数据写入硬盘 */
void ide_write(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt)
{
    lock_acquire(&hd->my_channel->channel_lock);

    /* 选择磁盘 */
    select_disk(hd);

    uint32_t secs_op;
    uint32_t secs_done = 0;
    while(secs_done < sec_cnt)
    {
        if(secs_done + 256 <= sec_cnt)
        {
            secs_op = 256;
        }
        else
        {
            secs_op = sec_cnt - secs_done;
        }

        /* 写入待写入的扇区数和起始扇区号 */
        select_sector(hd, lba + secs_done, secs_op);

        /* 写入写命令 */
        cmd_out(hd->my_channel, CMD_WRItE_SECTOR);

        /* 将数据写入硬盘 */
        if(!busy_wait(hd))
        {
            char error[64];
            sprintf(error, "%s write sector %d failed|||||\n", hd->name, lba);
            PANIC(error);
        }

        /* 数据写入硬盘 */
        write2sector(hd, (void*)((uint32_t)buf + secs_done * 512), secs_op);

        /* 在硬盘响应期间阻塞自己,硬盘完成操作后会中断 */
        sema_down(&hd->my_channel->disk_done);

        secs_done += secs_op;

    } // end while
    lock_release(&hd->my_channel->channel_lock);
}

/* 硬盘中断处理程序 */
void intr_hd_handler(uint8_t irq_no)
{
    /* 获取通道索引 */
    uint8_t ch_no = irq_no - 0x2e;
    
    ide_channel* channel = &channels[ch_no];

    /* 由于加了通道锁，只会是同一块硬盘操作引起的中断 */
    if(channel->expecting_intr == true)
    {
        channel->expecting_intr = false;
        sema_up(&channel->disk_done);

        /* 读出status，清除中断 */
        inb(reg_status(channel));
    }
}


/* 硬盘数据结构初始化 */
void ide_init()
{
    printk("ide init start\n");

    /* 获取硬盘数量 */
    uint8_t hd_cnt = *((uint8_t*)(0x475));

    ASSERT(hd_cnt > 0);

    list_init(&partition_list);
    /* 一个ide通道上有两个硬盘 */
    channel_cnt = DIV_ROUND_UP(hd_cnt, 2);

    ide_channel* channel;
    
    uint8_t dev_no = 0;

    for(uint8_t channel_no = 0; channel_no < channel_cnt; channel_no++)
    {
        channel = &channels[channel_no];
        sprintf(channel->name, "ide%d", channel_no);

        switch(channel_no)
        {
        case 0:
            /* ide0通道的端口号起始地址是0x1f0 */
            channel->port_base = 0x1f0;
            /* ideo通道的中断号 */
            channel->irq_no    = 0x20 + 14;
            break;
        case 1:
            /* ide1的起始端口号 */
            channel->port_base = 0x170;
            /* ide1的中断号 */
            channel->irq_no    = 0x20 + 15;
            break;
        }
        /* 未向硬盘写入指令时不期待硬盘中断 */
        channel->expecting_intr = false;

        lock_init(&channel->channel_lock);

        /* 初始化为0， 向硬盘发出请求后阻塞 */
        sema_init(&channel->disk_done, 0);
        
        register_handler(channel->irq_no, intr_hd_handler);

        while(dev_no < 2)
        {
            disk* hd = &channel->devices[dev_no];
            hd->my_channel = channel;
            hd->dev_no = dev_no;
            sprintf(hd->name, "sd%c", 'a' + channel_no * 2 + dev_no);
            
            printk("hd name %s\n", hd->name);

            identify_disk(hd);
            if(dev_no != 0)
            {
                partition_scan(hd, 0);
            }
            p_no = 0;
            l_no = 0;
            dev_no++;
        }

        dev_no = 0;
    }
    
    printk("\n  all partition info\n");
    /* 打印分区表信息 */
    list_traversal(&partition_list, partition_info, (int)NULL);

    printk("ide_init done\n");
}






























