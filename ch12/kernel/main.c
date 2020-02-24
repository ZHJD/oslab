#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"
#include "sync.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "string.h"
#include "process.h"
#include "stdio.h"
#include "syscall.h"

void func1(void* arg);
void func2(void* arg);
void prog_a();
void prog_b();

int test_var_a = 0;
int test_var_b = 0;

int main(void) {
    put_str("I am a kernel.\n");
    init_all();
     
       thread_start("fun1", 31, func1, " 1_ ");
       thread_start("fun2", 31, func2, " 2_ ");
       process_execute(prog_a, "prog_a");
//       process_execute(prog_b, "prog_b");
    
    intr_enable();
    while(1);
    return 0;
}

void func1(void* arg)
{
       void* addr =  sys_malloc(33);
       console_put_str("I am thread_a, sys_malloc(33), addr is 0x");
       console_put_int((int)addr);
    console_put_char('\n');
    while(1);
}

void func2(void* arg)
{
    void* addr = sys_malloc(33);
    console_put_str("I am thread_a, sys_malloc(33), addr is 0x");
    console_put_int((int)addr);
    console_put_char('\n');
    while(1);
}

void prog_a()
{
    printf("pid is %d\n", getpid());
}

void prog_b()
{
    while(1)
    {
        test_var_b++;
    }

}
