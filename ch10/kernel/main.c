#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"

void func1(void* arg);
void func2(void* arg);


int main(void) {
    put_str("I am a kernel.\n");
	init_all();
     
    thread_start("fun1", 1, func1, "aaa ");
    thread_start("fun2", 1, func2, "bbb ");
    
    intr_enable();
   // asm volatile("sti"); // 开中断
	while(1)
    {
        put_str("ccc ");
    };
	return 0;
}

void func1(void* arg)
{
    char* ch = (char*)arg;
    for(;;)
    {
        put_str(arg);
       // put_char('\n');
    }
    while(1);
}


void func2(void* arg)
{
    char* ch = (char*)arg;
    for(;;)
    {
        put_str(arg);
       // put_char('\n');
    }
    while(1);
}
