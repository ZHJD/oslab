#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"

// 负责初始化所有模块
void init_all(void)
{
    put_str("init_all\n");

     idt_init();  // 初始化中断
   
     timer_init(); // 初始化pit8253   
    
    thread_init();   
     mem_init();
}
