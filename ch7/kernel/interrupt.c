// 有关中断的内容在这个文件实现
#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"

#define IDT_DESC_CNT 0x21	// 目前总共支持的中断数,共33个中断处理程序

//---------------------------------------------------------------------------------
#define PIC_M_CTRL  0x20  // 主片的控制端口是0x20
#define PIC_M_DATA  0x21  // 主片的数据端口是0x21
#define PIC_S_CTRL  0xa0  // 从片的控制端口是0xa0
#define PIC_S_DATA  0xa1  // 从片的数据端口是0xa1


//--------------------------------------------------------------------------------
// 中断门描述符结构体,按照中断描述符格式定义
struct gate_desc
{
	uint16_t	func_offset_low_word;     // 中断处理程序在目标段内的偏移两的15-0位
	uint16_t	selector;				  // 中断处理程序在目标代码段的段描述符选择子
	uint8_t		dcount;					  // 0-3位未使用，4-7位全部为0, 不用考虑

	uint8_t		attribute;				  // 包含4位type, 1位s,2位dpl,1位p
	uint16_t	func_offset_high_word;    // 中断处理程序在目标段内的偏移量31-16位
};

//---------------------------------------------------------------------------------
// 使用static关键字， 该函数名和变量名在其它文件中不可见
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);

// 初始化中断描述符表
static void idt_desc_init(void);

// 初始化可编程中断控制器8259A
static void pic_init(void);

static struct gate_desc idt[IDT_DESC_CNT]; // idt是中断描述符表，定义了一个中断描述符表数组

extern intr_handler intr_entry_table[IDT_DESC_CNT];	// 声明引用定义在kernel.S中的中断处理函数入口数组


static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function)
{
	p_gdesc->func_offset_low_word  = (uint32_t)function & 0x0000ffff;
	// 段分为系统段和数据段，数据段中能执行的是代码段,系统段中的数据是cpu要用到的，如中断描述符表
	p_gdesc->selector 			   = SELECTOR_K_CODE;  // SELECTION_K_CODE 定义在global.h中，指向内核数据段中的代码段的选择子
	p_gdesc->dcount				   = 0;
	p_gdesc->attribute			   = attr;
	p_gdesc->func_offset_high_word = (uint32_t)function & 0xffff0000;
}

static void idt_desc_init(void)
{
	int i = 0;
	for(i = 0; i < IDT_DESC_CNT; i++)
	{
		// IDT_DESC_ATTR_DPL0 定义在global.h中
		make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]);
	}
	put_str(" idt_desc_init done\n");
}

static void pic_init(void)
{
  // 初始化主片
  outb(PIC_M_CTRL, 0x11);
  outb(PIC_M_DATA, 0x20);

  outb(PIC_M_DATA, 0x04);
  outb(PIC_M_DATA, 0x01);

  // 初始化从片
  outb(PIC_S_CTRL, 0x11);
  outb(PIC_S_DATA, 0x28);

  outb(PIC_S_DATA, 0x02);
  outb(PIC_S_DATA, 0x01);

  // 打开主片上的IR0，目前只接受时钟产生的中断
  outb(PIC_M_DATA, 0xfe);
  outb(PIC_S_DATA, 0xff);

  put_str(" pic_init done\n");
}


// 完成有关中断的所有初始化工作
void idt_init(void)
{
	put_str("idt_init start\n");
	idt_desc_init();			// 初始化中断描述符表
	pic_init();					// 初始化8259A

	// 加载idt
	uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)((uint32_t)idt << 16)));
	asm volatile("lidt %0" : : "m" (idt_operand));
	put_str("idt_init done\n");
}
