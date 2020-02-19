#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H

#include "stdint.h"
#include "stdbool.h"

#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

/* 特权级0下的选择子 */
#define SELECTOR_K_CODE     ((0x1 << 3) + (TI_GDT << 2) + RPL0) // 内核数据段中的代码段
#define SELECTOR_K_DATA     ((0x2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK    SELECTOR_K_DATA					// 栈段和数据段选择子相同
#define SELECTOR_K_GS       ((0x3 << 3) + (TI_GDT << 2) + RPL0) // 指向显存段

/* tss 选择子 */
#define SELECTOR_TSS        ((0x4 << 3) + (TI_GDT << 2) + RPL0)

/* 特权级3下的选咋子  */
#define SELECTOR_U_CODE     ((0x5 << 3) + (TI_GDT << 2) + RPL3);
#define SELECTOR_U_DATA     ((0x6 << 3) + (TI_LDT << 2) + RPL3);
#define SELECTOR_U_STACK    SELECTOR_U_DATA




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


#define NULL ((void*)0)

/* 定义页框大小 */
#define PAGE_SIZE 4096

/* 线程栈中的魔数 */
#define THREAD_MAGIC_NUM  0x19870916

/*********************** GDT 描述符属性 **********************************************/

/* 段界限单位是4KB还是1B */
#define DESC_G_4K           1

/* 用eip还是ip，esp还是sp  */
#define DESC_D_32           1

/* 32位代码段 */
#define DESC_L_32           0

/* cpu不用此位，置为0 */
#define DESC_AVL            0

/* 表示在内存中 */
#define DESC_P              1

/* 特权级 */
#define DESC_DPL_0          0
#define DESC_DPL_1          1
#define DESC_DPL_2          2
#define DESC_DPL_3          3

/* s为1，数据段，s=0，为系统段 */
#define DESC_S_CODE         1
#define DESC_S_DATA         DESC_S_CODE

/* 表示系统段 */
#define DESC_S_SYS          0

/*x=1,c=0,r=0,a=0 可执行，非依从，不可读，access为0 */
#define DESC_TYPE_CODE      8

/* x=0,e=0,w=1,a=0 不可执行，非依从，可写，access为0 */
#define DESC_TYPE_DATA      2

/* B位为0，不忙,代表可用的80386TSS */
#define DESC_TYPE_TSS       9

/******************* DESC 描述符*******************************************/
/* DESC中高32位的20-23位 */
#define GDT_ATTR_HIGH       ((DESC_G_4K << 7) + (DESC_D_32 << 6) + \
                            (DESC_L_32 << 5) + (DESC_AVL << 4))

/* DESC中高32位的8-15位 */
#define GDT_CODE_ATTR_LOW_DPL3 \
                            ((DESC_P << 7) + (DESC_DPL_3 << 5) + \
                            (DESC_S_CODE << 4) + DESC_TYPE_CODE)

#define GDT_DATA_ATTR_LOW_DPL3 \
                            ((DESC_P << 7) + (DESC_DPL_3 << 5) + \
                            (DESC_S_DATA << 4) + DESC_TYPE_DATA)



/*************************  tss描述符属性   *******************************/
/* tss的D和L位都是0 */
#define TSS_DESC_D          0

#define TSS_ATTR_HIGH       ((DESC_G_4K << 7) + (TSS_DESC_D << 6) + \
                            (DESC_L_32 << 5) + (DESC_AVL << 4) + 0x0)

#define TSS_ATTR_LOW        ((DESC_P << 7) + (DESC_DPL_0 << 5) + \
                            (DESC_S_SYS << 4) + DESC_TYPE_TSS)


/* 定义gdt中描述符的结构 */
typedef struct gdt_desc
{
    /* 段界限0-15 */
    uint16_t    limit_low_word;

    /* 段基址0-15 */
    uint16_t    base_low_word;

    /* 段基址16-23 */
    uint8_t     base_mid_byte;

    /* 属性 */
    uint8_t     attr_low_byte;

    /* 属性和段界限16-19 */
    uint8_t     limit_high_attr_high;

    /* 段基址24-31 */
    uint8_t     base_high_byte;
}gdt_desc;

/**************************************************************************/

/* 保留值，设为1 */
#define EFLAGS_MBS  (1 << 1)

/* 开中断 */
#define EFLAGS_IF_! (1 << 9)

/* 关中断 */
#define EFLAGS_IF_0 (1 << 9)

/* 2位 */
#define EFLAGS_IOPL_3 (3 << 12)

#define EFLAGS_IOPL_0 (0 << 12)

#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))









































#endif
