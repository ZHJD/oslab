#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"
#include "sync.h"

void func1(void* arg);
void func2(void* arg);

lock lck;

int main(void) {
    put_str("I am a kernel.\n");
	init_all();
     
    thread_start("fun1", 8, func1, "aaa ");
    thread_start("fun2", 31, func2, "bbb ");
    
    intr_enable();
   // asm volatile("sti"); // 开中断
    lock_init(&lck);
    while(1)
    {
        lock_acquire(&lck);
        put_str("ccc ");
        lock_release(&lck);
    };
	return 0;
}

void func1(void* arg)
{
    char* ch = (char*)arg;
    for(;;)
    {
        lock_acquire(&lck);
        put_str(arg);
        lock_release(&lck);
    }
    while(1);
}


void func2(void* arg)
{
    char* ch = (char*)arg;
    for(;;)
    {
        lock_acquire(&lck);
        put_str(arg);
        lock_release(&lck);
    }
    while(1);
}
