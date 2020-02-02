#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H

#include "stdint.h"

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE ((0x1 << 3) + (TI_GDT << 2) + RPL0) // 内核数据段中的代码段
#define SELECTOR_K_DATA ((0x2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK SELECTOR_K_DATA					// 栈段和数据段选择子相同
#define SELECTOR_K_GS   ((0x3 << 3) + (TI_GDT << 2) + RPL0) // 指向显存段

// IDT 描述符属性
#define IDT_DESC_P  	1
#define IDT_DESC_DPL0   0
#define IDT_DESC_DPL3	3
#define IDT_DESC_S		0    // 因为中断描述符表是给cpu使用的，属于系统段，故s为0
#define IDT_DESC_TYPE_386   0b1110  // 32位
#define IDT_DESC_TYPE_286	0b0110  // 16位

#define IDT_DESC_ATTR_DPL0	\
	((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_S + IDT_DESC_TYPE_386)

#define IDT_DESC_ATTR_DPL3  \
	((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_S + IDT_DESC_TYPE_386)

#endif
