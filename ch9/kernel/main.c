#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"

void func(void* arg);

int main(void) {
    put_str("I am a kernel.\n");
	init_all();
    
    thread_start("print", 31, func, "thread_start");

   // asm volatile("sti"); // 开中断
	while(1);
	return 0;
}

void func(void* arg)
{
    char* ch = (char*)arg;
    for(int i = 0; i < 10; i++)
    {
        put_str(arg);
        put_char('\n');
    }
    while(1);
}
