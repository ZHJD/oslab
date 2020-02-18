#include "print.h"
#include "init.h"
#include "memory.h"
#include "thread.h"
#include "sync.h"
#include "keyboard.h"
#include "ioqueue.h"
#include "string.h"


void func1(void* arg);
void func2(void* arg);

int main(void) {
    put_str("I am a kernel.\n");
	init_all();
     
    thread_start("fun1", 8, func1, " 1_ ");
    thread_start("fun2", 31, func2, " 2_ ");
    
    intr_enable();
    while(1);
	return 0;
}

void func1(void* arg)
{
    char str[20];
    char* ch = (char*)arg;
    int len = strlen(ch);
    strcpy(str, arg);
    while(1)
    {
        str[len] = ioq_getchar(&keyboard_buf);
        str[len + 1] = '\0';
        console_put_str(str);
    }
}

void func2(void* arg)
{
    char str[20];
    char* ch = (char*)arg;
    int len = strlen(ch);
    strcpy(str, arg);
    while(1)
    {
        str[len] = ioq_getchar(&keyboard_buf);
        str[len + 1] = '\0';
        console_put_str(str);
    }
}
