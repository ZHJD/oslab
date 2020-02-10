#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"

void func1(void* arg);
void func2(void* arg);


int main(void) {
    put_str("I am a kernel.\n");
	init_all();
     
    thread_start("fun1", 31, func1, "aaaaaaaaaaaa");
    thread_start("fun2", 8, func2, "bbbbbbbbbbbb");
    
    put_str("开启中断\n");    
    intr_enable();
   // asm volatile("sti"); // 开中断
	while(1)
    {
        put_str("Main ");
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
