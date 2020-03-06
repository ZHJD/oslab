#include "stdio.h"
#include "string.h"
#include "syscall.h"

/* 整型转换成字符(integer to ascii),更新buf_ptr_addr值 */
static void itoa(uint32_t value, char** buf_ptr_addr, uint8_t base)
{
    /* 求模，得到最低位 */
    uint32_t m = value % base;
    uint32_t i = value / base;

    /* 不是最末位 */
    if(i != 0)
    {
        itoa(i, buf_ptr_addr, base);
    }
    if(m < 10)
    {
        /* 数字0-9转换为'0' - '9' */
        **buf_ptr_addr = m + '0';
        (*buf_ptr_addr)++;
    }
    else
    {
        /* 把大于等于10的数字转换为 'A' - 'F' */
        **buf_ptr_addr = m - 10 + 'A';
        (*buf_ptr_addr)++;
    }
}

/* 将参数 ap 转换成 format 输出到字符串 str, 并返回替换后 str 长度 */
uint32_t vsprintf(char* str, const char* format, va_list ap)
{
    char* buf_ptr = str;
    const char* index_ptr = format;

    /* format 中的第一个字符 */
    char index_char = *index_ptr;
    int32_t arg_int;
    char* arg_str;
    while(index_char != '\0')
    {
        /* 如果不是% 直接复制到输出缓冲 */
        if(index_char != '%')
        {
            *(buf_ptr++) = index_char;
            index_char = *(++index_ptr);
            continue;
        }

        /* 跳过 '%' */
        index_char = *(++index_ptr);
        switch(index_char)
        {
        case 'x': // 16进制数
            /* 获取可变参数对应的值 */
            arg_int = va_arg(ap, int);
            /* 转化为字符，并写入字符缓冲区 */
            itoa(arg_int, &buf_ptr, 16);
            /* 跳过格式化字符 */
            index_char = *(++index_ptr);
            break;
        case 's':
            arg_str = va_arg(ap, char*);
            strcpy(buf_ptr, arg_str);
            buf_ptr += strlen(arg_str);
            index_char = *(++index_ptr);
            break;
        case 'd':
            arg_int = va_arg(ap, int);
            if(arg_int < 0)
            {
                arg_int = -arg_int;
                *(buf_ptr++) = '-';
            }
            itoa(arg_int, &buf_ptr, 10);
            index_char = *(++index_ptr);
            break;
        case 'c':
            *(buf_ptr++) = va_arg(ap, char);
            index_char = *(++index_ptr);
            break;
        }
    }
  //  *buf_ptr = '\0';
    return strlen(str);
}

/* 格式化输出字符串format */
uint32_t printf(const char* format, ...)
{
    va_list args;

    /* args指向format的地址，args + 4指向第一个可变参数 */
    va_start(args, format);
    char buf[1024] = {'\0'};
    vsprintf(buf, format, args);
    
    /* 清空指针 */
    va_end(args);

    /* 返回写入字符串的长度 */
    return write(buf);
}

/* 各种类型转换为char*  */
void sprintf(char* str, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buf[1024] = {'\0'};
    vsprintf(buf, format, args);
    va_end(args);
    strcpy(str, buf);
}
