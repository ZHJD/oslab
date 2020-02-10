#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

/*中断处理函数入口地址*/
typedef void* intr_handler;

/*************************
 * 定义中断的两种状态
 * INTR_OFF:值为0表示关中断
 * INTR_ON::值为1表示开中断
 */
enum intr_status
{
    INTR_OFF,
    INTR_ON
};

/************************************
 * 函数名:intr_get_status()
 * 功能:获取中断状态
 * 返回值:开中断还是关中断
 */
enum intr_status get_intr_status();

/************************************
 * 函数名:set_intr_status()
 * 功能:设置中断的状态为status
 * 参数:intr_status
 * 返回设置中断前的状态
 */
enum intr_status set_intr_status(enum intr_status status);

/************************************
 * 函数名:intr_enable()
 * 功能：开中断，并返回打开中断前的中断状态
 * 返回值：返回开中断前的中断状态
 */
enum intr_status intr_enable();

/************************************
 * 函数名:intr_disable()
 * 功能:关闭中断
 * 返回值:返回关中断前的状态
 */
enum intr_status intr_disable();

/*完成有关中断的所有初始化工作*/
void idt_init(void);

/*************************************
 * 函数名:register_handler()
 * vector_no:中断号
 * function: 中断处理程序
 * 功能:把中断处理程序注册到idt_table中
 */
void register_handler(const uint8_t vector_no, intr_handler function);


#endif
