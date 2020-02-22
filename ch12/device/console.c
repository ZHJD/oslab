#include "console.h"
#include "print.h"
#include "sync.h"

/* 控制台锁 */
static lock console_lock;

/**********************************
 * 函数名:console_init()
 * 功能:初始化控制台锁
 */ 
void console_init()
{
    lock_init(&console_lock);
}

/************************************
 * 函数名:console_acquire
 * 功能:获取终端
 * 返回值:无
 */
static void console_acquire()
{
    lock_acquire(&console_lock);
}

/**************************************
 * 函数名:console_release()
 * 功能:释放终端
 * 返回值:无
 */
static void console_release()
{
    lock_release(&console_lock);
} 

/***************************************
 * 函数名:console_put_str()
 * str:指向以'\0'为结尾的字符串指针
 * 功能:打印字符串
 * 返回值:无
 */ 
void console_put_str(const char* str)
{
    console_acquire();
    put_str(str);
    console_release();
}

/******************************************
 * 函数名:console_put_char()
 * char_asci:asci字符
 * 功能:打印单个字符
 * 返回值:无
 */ 
void console_put_char(const uint8_t char_asci)
{
    console_acquire();
    put_char(char_asci);
    console_release();
}

/*********************************************
 * 函数名:console_put_int()
 * num:无符号整数
 * 功能:以十六进制形式打印无符号整数
 * 返回值:无
 */ 
void console_put_int(const uint32_t num)
{
    console_acquire();
    console_put_str("0x");
    put_int(num);
    console_release();
}
