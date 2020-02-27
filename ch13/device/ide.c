#include "ide.h"
#include "stdio_kernel.h"
#include "debug.h"



/* 定义硬盘各寄存器的端口号 */
#define reg_data(channel)       (channel->port_base + 0)
#define reg_error(channel)      (channel->port_base + 1)
#define reg_sect_cnt(channel)   (channel->port_base + 2)
#define reg_lba_I(channel)      (channel->port_base + 3)
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
    }

    printk("ide_init done\n");
}
