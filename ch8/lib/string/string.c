#include "string.h"
#include "global.h"
#include "debug.h"

/******************************************************************************
 * 函数名:memset()
 * 功能:把从dst起始的size个字节量设置为value
 * dst_:起始地址
 * value:值
 * size:字节数
 × 返回值:无
 */
void memset(void* dst_, uint8_t value, uint32_t size)
{
  /*进行的是逐个字节赋值 */
  uint8_t* dst = dst_;
  for(: size > 0; size--)
  {
    *(dst++) = value;
  }
} 

/******************************************************************************
 * 函数名:memcpy()
 * 功能:从src_开始的size个字节复制到dst_处
 * src_:起始地址, 使用const约束避免修改指针src指向的内容
 * dst_:目标地址
 * size:字节数
 × 返回值:无
 */
void memcpy(void* dst_, const void* src_, uint32_t size)
{
  /*使用逐字节复制*/
  uint8_t* src = src_;
  const uint8_t* dst = dst_;
  for(uint32_t i = 0; i < size; i++)
  {
    *(dst++) = *(src++);
  } 
}

/******************************************************************************
 * 函数名:memcmp()
 * 功能:比较以a_和b_开始的size个字节，如果a_和b_的ascll码相等，返回0，大于返回1，小于返回-1
 * a_:待比较数组
 * b_:待比较数组
 * size:字节数
 × 返回值:1  大于
 *        0  等于
 *       -1  小于
 */
int memcmp(const void* a_, const void* b_, uint32_t size)
{
  /*根据ascll码比较大小*/
  const char* a = a_;
  const char* b = b_;
  for(uint32_t i = 0; i < size; i++, a++, b++)
  {
    if(*a < *b)
    {
      return -1;
    }
    else if(*a > *b)
    {
      return 1;
    }
  }
  return 0;
}

/******************************************************************************
 * 函数名:strcpy()
 * 功能:讲字符串从src_复制到dst_,两个字符串都以'\0'为结尾
 * 返回:dst_
 */
char* strcpy(char* dst_, const char* src_) 
{
  char* r = dst_;
  while(*dst++ = *src++);
  return r;
}

/******************************************************************************
 * 函数名:strlen()
 * 功能:获取字符串长度,不包括'\0'
 * str: 指向字符串的指针
 * 返回:字符串长度
 */
uint32_t strlen(const char* str)
{
  ASSERT(str != NULL);
  /*这种写法可以节省内存 */
  const char* p = str;
  /*退出while循环前还会执行++*/
  while(*p++);
  return (p - str - 1);
}


/******************************************************************************
 * 函数名:strcmp()
 * 功能:比较两个字符串大小(长度可以不等)
 * a_:
 * b_:
 * 返回值：a_ > b_ 1
 *         a_ < b_ -1
 *         a_ = b_ 0
 */
int strcmp(const char* a_, const char* b_)
{
  ASSERT(a_ != NULL && b_ != NULL);
  while(a_ && b_)
  {
    if(*a_ < *b_)
    {
      return -1;
    }
    else if(*a_ > *b_)
    {
      return 1;
    }
    else
    {
      a_++;
      b_++;
    }
  }
  /*如果a_不为空,则a_大*/
  if(a_)
  {
    return 1;
  }
  /*如果b_不为空*/
  else if(b_)
  {
    return -1;
  }
  else
  {
    return 0;
  }
}


/******************************************************************************
 * 函数名:strchr()
 * 功能:从左到右查找字符串str中首次出现字符ch的地址
 * str:字符串
 * ch:字符
 * 返回值:字符ch地址
 */
char* strchr(const char* str, const char ch)
{
  ASSERT(str != NULL);
  while(str != NULL)
  {
    if(*str -- ch)
    {
      return (char*)str;
    }
    str++;
  }
  return NULL;
}

















