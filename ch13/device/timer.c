#include "timer.h"
#include "print.h"
#include "io.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"
#include "global.h"

/*计数器频率，中断频率,每秒钟端数*/
#define IRQ0_FREQUENCY     100

/* 中断周期为10ms */
#define mil_seconds_per_intr 1000 / IRQ0_FREQUENCY

/*计数器的工作脉冲信号频率 */
#define INPUT_FREQUENCY    1193180      

/*计数器初始值*/
#define COUNTER_VALUE      INPUT_FREQUENCY / IRQ0_FREQUENCY

/*设置计数器初值的端口号*/
#define COUNTER_POINT      0x40

/*用来设置在控制字中指定所使用的计数器号码，设置sc1和sc2*/
#define COUNTER_NO         0

/*设置计数器的工作方式，对应于m2-m0 */
#define COUNTER_MODE       2

/*设置读写方式，写读写低8位，后读写高8位*/
#define READ_WRITE_LATCH   3

/*控制字寄存器端口,向其中写入工作方式*/
#define PIT_CONTROL_POINT  0x43

/*计数器初始值是否采用bcd10进制*/
#define IS_BCD              0


/* ticks 是内核自中断开启以来总共的滴答数 */
uint32_t ticks = 0;

/****************************************************************
 * 函数名：frequency_set()
 * 功能： 往控制寄存器中写入控制字，并设置计数器初值
 * counter_point:计数器端口号，用于指定counter_value
 * pit_control_point:控制寄存器端口
 * counter_no:指定使用的计数器
 * rwl:设置读写锁存方式
 * counter_mode:设置工作方式m2-m0
 * counter_value:设置计数器初始值
 * is_bcd: 1 采用bcd10进制，0表示采用二进制
 * 返回值:无
 */
static void frequency_set(uint8_t counter_point, 
                          uint8_t pit_control_point,
                          uint8_t counter_no,
                          uint8_t rwl,
                          uint8_t counter_mode,
                          uint16_t counter_value,
                          uint8_t is_bcd)
{
    /*拼接控制字 */
    uint8_t control_byte = counter_no << 6 |
                          rwl << 4         |
                        counter_mode << 1  |
                        is_bcd;
    outb(pit_control_point, control_byte);
    /*写入低8位*/
    outb(counter_point, (uint8_t)counter_value);
    /*写入高8位*/
    outb(counter_point, (uint8_t)(counter_value >> 8));
}

/***********************************************
 * 函数名:intr_timer_handler()
 * 功能:当发生时钟中断时，进入该函数进行处理
 * 返回值:无
 */
static void intr_timer_handler(void)
{
    task_struct* cur_thread = get_running_thread_pcb();
    ASSERT(cur_thread->stack_magic == THREAD_MAGIC_NUM);
    
    /* 记录此线程占用的cpu时间 */
    cur_thread->elapsed_ticks++;

    /* 总共发生的滴答数 */
    ticks++;

    if(cur_thread->ticks == 0)
    {
        schedule();
    }
    else
    {
        cur_thread->ticks--;
    }
    
}

/* 以tick为单位sleep,任何时间形式的sleep都转换为tick形式 */
static void ticks_to_sleep(uint32_t sleep_ticks)
{
    uint32_t start_tick = ticks;
    
    /* 当间隔时间小于sleep_ticks时， 让出cpu,不是特别准确 */
    while(ticks - start_tick < sleep_ticks)
    {
        thread_yield();
    }
}


/* 以ms为单位的顺眠函数  */
void mtime_sleep(uint32_t m_seconds)
{
    /* 转化为中断次数  */
    uint32_t sleep_ticks = DIV_ROUND_UP(m_seconds, mil_seconds_per_intr);
    ticks_to_sleep(sleep_ticks);
}


/************************************************************
 * 函数名：timer_init()
 * 功能：初始化PIT8253
 * 返回值：无
 */
void timer_init()
{
    put_str("\ntimer init start!\n");
    frequency_set(COUNTER_POINT,
                  PIT_CONTROL_POINT,
                  COUNTER_NO,
                  READ_WRITE_LATCH,
                  COUNTER_MODE,
                  COUNTER_VALUE,
                  IS_BCD
                 );
    register_handler(0x20, intr_timer_handler);
    put_str("timer init done\n");
}
