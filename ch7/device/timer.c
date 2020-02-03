#include "timer.h"
#include "print.h"
#include "io.h"

/*计数器频率，中断频率*/
#define IRQ0_FREQUENCY     100

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
                          uint8_t pit_control_point.
                          uint8_t counter_no,
                          uint8_t rwl,
                          uint8_t counter_mode,
                          uint16_t counter_value
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

/************************************************************
 * 函数名：timer_init()
 * 功能：初始化PIT8253
 * 返回值：无
 */
void timer_init()
{
    frequency_set(COUNTER_POINT,
                  PIT_CONTROL_POINT,
                  COUNTER_NO,
                  READ_WRITE_LATCH,
                  COUNTER_MODE,
                  COUNTER_VALUE,
                  IS_BCD
                 );
    put_str("timer init done"\n");
}
