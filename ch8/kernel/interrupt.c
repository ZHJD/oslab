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


// ---------------------------------------------------------------------------------
char* intr_name[IDT_DESC_CNT];   // 用于保存异常的名字
// 定义中断处理程序数组，在kernel.S中定义的intrXXentry
// 只是中断处理程序的入口，最终调用的是ide_table中的处理程序
intr_handler idt_table[IDT_DESC_CNT];

//---------------------------------------------------------------------------------
// 使用static关键字， 该函数名和变量名在其它文件中不可见
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);

// 初始化中断描述符表
static void idt_desc_init(void);

// 初始化可编程中断控制器8259A
static void pic_init(void);

// 完成一般中断处理函数注册及异常名称注册
static void exception_init(void);

// 通用的中断处理函数， 一般用在异常出现时处理
// vec_nr 中断向量号
static void general_intr_handler(uint8_t vec_nr);

static struct gate_desc idt[IDT_DESC_CNT]; // idt是中断描述符表，定义了一个中断描述符表数组

extern intr_handler intr_entry_table[IDT_DESC_CNT];	// 声明引用定义在kernel.S中的中断处理函数入口数组


//*********************************************************************************************************
static void general_intr_handler(uint8_t vec_nr)
{
  if(vec_nr == 0x27 || vec_nr == 0x2f)
  {
    // IRQ7和IRQ15会产生伪中断，无需处理
    // 0x2f是从片8259A上的最后一个引脚IRQ,保留项
    return;
  }
  put_str("int vector : 0x");
  put_int(vec_nr);
  put_char('\n');
}

static void exception_init(void)
{
  int i;
  for(i = 0; i < IDT_DESC_CNT; i++)
  {
    // idt_table中的函数在kernel.S中根据中断向量号调用
    // 如call [idt_table + 向量号*4]
    // 每个指针占用四个字节
    idt_table[i] = general_intr_handler;
    intr_name[i] = "unknown";
  }
  intr_name[0]  = "#DE Divide Error";
  intr_name[1]  = "#DB Debug Exception";
  intr_name[2]  = "No Maskable Interrupt";
  intr_name[3]  = "#BP Breakpoint Exception";
  intr_name[4]  = "#OF Overflow Exception";
  intr_name[5]  = "#BR Bound Range Exceeded Exception";
  intr_name[6]  = "#UD Invalid Opcode Exception";
  intr_name[7]  = "#NM Device not Available Exception";
  intr_name[8]  = "DF Double Fault Exception";
  intr_name[9]  = "Coprocessor Segment Overrun";
  intr_name[10] = "#TS Invaild TSS Exception";
  intr_name[11] = "#NP Segment Not Present";
  intr_name[12] = "#SS Stack Fault Exception";
  intr_name[13] = "#GP General Protection Exception";
  intr_name[14] = "#PF Page-Fault Exception";
  // 15是保留项，还没有使用
  intr_name[16] = "#MF x87 FPU Floating-Point Error";
  intr_name[17] = "#AC Alignment Check Exception";
  intr_name[18] = "#MC Machine-Check Exception";
  intr_name[19] = "#XF SIMD Floating-Point Exception";
}

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
	exception_init();     // 异常名称化并注册通常的中断处理函数
  pic_init();					  // 初始化8259A
  
	// 加载idt
	uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)((uint32_t)idt << 16)));
	asm volatile("lidt %0" : : "m" (idt_operand));
	put_str("idt_init done\n");
}
