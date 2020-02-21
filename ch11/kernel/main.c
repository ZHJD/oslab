#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"
#include "sync.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "string.h"
#include "process.h"


void func1(void* arg);
void func2(void* arg);
void prog_a();
void prog_b();

int test_var_a = 0;
int test_var_b = 0;

int main(void) {
    put_str("I am a kernel.\n");
    init_all();
     
//    thread_start("fun1", 31, func1, " 1_ ");
//    thread_start("fun2", 31, func2, " 2_ ");
    process_execute(prog_a, "prog_a");
//    process_execute(prog_b, "prog_b");
    
    intr_enable();
    while(1);
    return 0;
}

void func1(void* arg)
{;
    while(1)
    {
        console_put_str("v_a:0x");
        console_put_int(test_var_a);
    }
}

void func2(void* arg)
{
    while(1)
    {
        console_put_str("v_b:0x");
        console_put_int(test_var_b);
    }
}

void prog_a()
{
    while(1)
    {
        test_var_a++;
    }
}

void prog_b()
{
    while(1)
    {
        test_var_b++;
    }

}
