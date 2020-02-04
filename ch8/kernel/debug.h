#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H

/**************************************************************************
 * 函数名:panic_spin()
 * 功能:打印文件名，行号，函数名，ASSERT测试条件，并使程序悬停
 * filename:程序文件名
 * line:代码行号
 * func:函数名
 * condition:ASSERT参数
 * 返回值:无
 */
void panic_spin(char* filename, int line, const char* func, const char* condition);

/******************* __VA_ARGS__ ****************************************
 * __VA_ARGS__是预处理器所支持的专用标识符
 * 代表所有于省略号相对应的参数
 * “..."表示定义的宏参数可变
 * __FILE__ 被编译的文件名
 * __LINE__ 被编译文件中的行号
 * __func__ 被编译的函数名
 * __VA_ARGS__和comdition相对应
 */
#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

/*如果在编译时候设置-D参数,就会把ASSERT(CONDITION)替换成空语句*/
#ifdef MDEBUG
    #define ASSERT(CONDITION) ((void)0)
#else
    #define ASSERT(CONDITION)   \
        /*如果条件测试为真，什么都不做*/   \
        if(CONDITION)           \
            {}                  \
        else                    \
        /* 符号#让编译器把宏的参数转化为字符串常量*/  \
            {PANIC(#CONDITION)} 
#endif /* __NDEBUG */

#endif /*__KERNEL_DEBUG_H*/
