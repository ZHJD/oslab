#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
// 负责初始化所有模块
void init_all(void)
{
    put_str("init_all\n");

    timer_init(); // 初始化pit8253

     idt_init();  // 初始化中断
}
