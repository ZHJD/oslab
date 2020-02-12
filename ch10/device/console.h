#ifndef __CONSOLE_H
#define __CONSOLE_H

/***************************************
 * 函数名:console_put_str()
 * str:指向以'\0'为结尾的字符串指针
 * 功能:打印字符串
 * 返回值:无
 */ 
void console_put_str(const char* str);

/******************************************
 * 函数名:console_put_char()
 * char_asci:asci字符
 * 功能:打印单个字符
 * 返回值:无
 */ 
void console_put_char(const uint8_t char_asci);

/*********************************************
 * 函数名:console_put_int()
 * num:无符号整数
 * 功能:以十六进制形式打印无符号整数
 * 返回值:无
 */ 
void console_put_int(const uint32_t num);

#endif
