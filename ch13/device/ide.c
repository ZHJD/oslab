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
    printk("disk not ready!\n");
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

    /* 一个ide通道上有两个硬盘 */
    channel_cnt = DIV_ROUND_UP(hd_cnt, 2);

    ide_channel* channel;

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
    }

    printk("ide_init done\n");
}






























