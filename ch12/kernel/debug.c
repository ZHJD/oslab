#include "debug.h"
#include "print.h"
#include "interrupt.h"


/**************************************************************************
 * 函数名:panic_spin()
 * 功能:打印文件名，行号，函数名，ASSERT测试条件，并使程序悬停
 * filename:程序文件名
 * line:代码行号
 * func:函数名
 * condition:ASSERT参数
 * 返回值:无
 */
void panic_spin(char* filename, int line, const char* func, const char* condition)
{
    /*关闭中断*/
    intr_disable();    
    put_str("\n\n !!!!!!!!!! ERROR !!!!!!!!!!!!\n");
   
    put_str("filename:");
    put_str(filename);
    put_char('\n');

    put_str("line:");
    put_int(line);
    put_char('\n');
    
    put_str("function name:");
    put_str(func);
    put_char('\n');

    put_str("condition:");
    put_str(condition);
    put_char('\n');

    while(1);
}
