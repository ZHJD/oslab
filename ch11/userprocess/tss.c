#include "tss.h"
#include "global.h"
#include "string.h"
/* 任务状态段tss*/
struct tss
{
    /* 用于回退到上个tss结构 */
    uint32_t back_link;
    
    uint32_t* esp0;
    uint32_t  ss0;

    uint32_t* esp1;
    uint32_t  ss1;

    uint32_t* esp2;
    uint32_t  ss2;

    uint32_t  cr3;
    uint32_t  (*eip)(void);
    uint32_t  eflags;
    uint32_t  eax;
    uint32_t  ecx;
    uint32_t  edx;
    uint32_t  ebx;
    uint32_t  esp;
    uint32_t  ebp;
    uint32_t  esi;
    uint32_t  edi;
    uint32_t  es;
    uint32_t  cs;
    uint32_t  ss;
    uint32_t  ds;
    uint32_t  fs;
    uint32_t  gs;

    /* ldt选择子 */
    uint32_t  ldt;

    uint32_t  trace;
    
    /* io位图在tss中的偏移地址 */
    uint32_t  io_base;
};

static struct tss tss;

/******************************************************
 * 函数名:update_tss_esp0()
 * pthread:线程pcb起始地址
 * 功能:设置esp0的值
 * 返回值:无
 */ 
void update_tss_esp0(task_struct* pthread)
{
    tss.esp0 = (uint32_t*)((uint32_t)pthread + PAGE_SIZE);
}

/********************************************************
 * 函数名:male_gdt_desc()
 * desc_addr:desc描述符中的基址
 * limit:低20位为界限，其余为0
 * attr_low:属性
 * attr_high:高4位是属性，低四位是0
 * 功能:构造一个全局段描述符
 * 返回值:desc描述符
 */ 
static gdt_desc make_gdt_desc(uint32_t* desc_addr,
                              uint32_t limit,
                              uint8_t attr_low,
                              uint8_t attr_high)
{
    uint32_t desc_base = (uint32_t)desc_addr;
    gdt_desc desc;
    desc.limit_low_word = limit & 0x0000ffff;
    desc.base_low_word  = desc_base & 0xffff0000;
    desc.base_mid_byte  = (desc_base & 0x00ff0000) >> 16;
    desc.attr_low_byte  = attr_low;
    desc.limit_high_attr_high = 
                        ((uint8_t)((limit & 0x000f0000) >> 16)) |
                        attr_high;

    desc.base_high_byte = desc_base >> 24;
    return desc;
}

/************************************************************
 * 函数名:tss_init()
 * 功能:添加用户层段描述符并初始化tss
 * 返回值:无
 */ 
void tss_init()
{
    put_str("tss init start\n");
    memset(&tss, 0, sizeof(tss));
    /* 保护模式下段寄存器中保存选择子 */
    tss.ss0 = SELECTOR_K_STACK;

    /* io位图在tss中的偏移地址 */
    tss.io_base = sizeof(tss);

    /******************************************
     * gdt基址是0x900，第1个位置置0，第2-4个位置分别放了
     * 代码段、数据段和显存段描述符，每个描述符占64bit
     * 故第4个描述符应该放在第5个位置，起始地址为0x920
     */ 

    /* tss 描述符 */
    gdt_desc* gdt_addr = (gdt_desc*)0xc0000920;
    *((gdt_desc*)0xc0000920) = make_gdt_desc(&tss,              
                              sizeof(tss) - 1,  
                              TSS_ATTR_LOW,     
                              TSS_ATTR_HIGH     
                              );
    /* dpl3下的代码段 */
    gdt_addr = (gdt_desc*)((uint32_t)gdt_addr + 0x8);
    *gdt_addr = make_gdt_desc((uint32_t*)0,
                             0xfffff,
                             GDT_CODE_ATTR_LOW_DPL3,
                             GDT_ATTR_HIGH
                             );
    /* dpl3下的数据段 */
    gdt_addr = (gdt_desc*)((uint32_t)gdt_addr + 0x8);
    *gdt_addr = make_gdt_desc((uint32_t*)0,
                             0xfffff,
                             GDT_DATA_ATTR_LOW_DPL3,
                             GDT_ATTR_HIGH
                             );

    /* 低16位表示表的大小，高32位表示段描述符表基址 */
    uint64_t gdt_operand = ((8 * 7 - 1) | 
                (uint64_t)((uint32_t)0xc000900 << 16));
    
    /* 重新加载gdt基址 */
    asm volatile ("lgdt %0"::"m"(gdt_operand));

    /* 加载tss选择子 */
    asm volatile ("ltr %w0"::"r"(SELECTOR_TSS));
    put_str("tss init and ltr done \n");


}
