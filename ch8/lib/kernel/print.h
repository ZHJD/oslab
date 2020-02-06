#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H

#include "stdint.h"

void put_char(uint8_t char_asci);

// 打印以'\0'为结尾的字符串
void put_strc(const char* message);

// 以16进制打印字符
void put_int(uint32_t num);
#endif
