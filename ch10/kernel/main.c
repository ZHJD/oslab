#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"
#include "sync.h"
#include "keyboard.h"
#include "ioqueue.h"

void func1(void* arg);
void func2(void* arg);

lock lk;

int main(void) {
    put_str("I am a kernel.\n");
	init_all();
     
    thread_start("fun1", 2, func1, " 1_ ");
    thread_start("fun2", 2, func2, " 2_ ");
    
    lock_init(&lk);

    intr_enable();
    while(1);
	return 0;
}

void func1(void* arg)
{
    char* str = (char*)arg;
    while(1)
    {
        lock_acquire(&lk);
        console_put_str(str);
        char byte = ioq_getchar(&keyboard_buf);
        console_put_char(byte);
        lock_release(&lk);
    }
}

void func2(void* arg)
{
    char* str = (char*)arg;
    while(1)
    {
        lock_acquire(&lk);
        console_put_str(str);
        char byte = ioq_getchar(&keyboard_buf);
        console_put_char(byte);
        lock_release(&lk);
    }
}
