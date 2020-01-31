#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

typedef void* intr_handler;

// 完成有关中断的所有初始化工作
void idt_init(void);

#endif
