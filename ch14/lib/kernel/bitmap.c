#include "bitmap.h"
#include "string.h"
#include "debug.h"

/*用于判断位图中单个位是1还是0*/
#define BITMAP_MASK 1

/**************************************
 * 函数名:bitmap_init()
 * 功能:位图中所有位设置为0
 * 返回值:无
 */
 void bitmap_init(bitmap* btmp)
 {
     memset(btmp->bits, 0, btmp->btmp_bytes_len);
 }

  /**************************************
  * 函数名：bitmap_scan_test()
  * 判断bit_idx位是否为1，若为1，返回true，否则返回false
  * bit_idx:bit索引
  * btmp:指向位图的指针
  * 返回值:true or false
  */
 bool bitmap_scan_test(const bitmap* btmp, const uint32_t bit_idx)
 {
     /*计算字节偏移量*/
     uint32_t byte_idx = bit_idx / 8;

     /*计算字节中位的偏移量*/
     uint32_t bit_odd  = bit_idx % 8;

     /*得到包含指定位的字节*/
     uint8_t  byte     = (btmp->bits)[byte_idx];

    /*通过移位做&操作*/
     return byte & (BITMAP_MASK << bit_odd);
 }

  /****************************************
  * 函数名:bitmap_scan()
  * 在位图中连续申请cnt个位，成功返回起始位下标，失败返回-1
  * btmp:指向位图的指针
  * cnt:申请位的个数
  * 返回值:起始位的指针或者-1
  */
 int bitmap_scan(const bitmap* btmp, const uint32_t cnt)
 {
     uint32_t byte_idx = 0;
     /*先逐个字节比较*/
     while((0xff == btmp->bits[byte_idx]) 
            && byte_idx < btmp->btmp_bytes_len)
    {
        byte_idx++;
    }
    
    ASSERT(byte_idx < btmp->btmp_bytes_len);

    if(byte_idx == btmp->btmp_bytes_len)
    {
        return -1;
    }

    uint32_t bit_idx_start = 8 * byte_idx;

    uint32_t count = 0;

    while(bit_idx_start < (btmp->btmp_bytes_len) * 8)
    {
        /*如果该位为0*/
        if(!bitmap_scan_test(btmp, bit_idx_start))
        {
            count++;
        }
        else
        {
            count = 0;
        }
        if(count == cnt)
        {
            return bit_idx_start - cnt + 1;
        }
        bit_idx_start++;
    }
    return -1; 
 }

 /**************************************************************
  * 函数名:bitmap_set()
  * 功能:把bit_idx处设置为value
  * btmp:指向位图的指针
  * bit_idx:bit索引
  * value:二进制数
  * 返回值:无
  */
 void bitmap_set(bitmap* btmp, const uint32_t bit_idx, const int8_t value)
 {
     /*value 值只能为0或者1*/
     ASSERT(value == 0 || value == 1);
     uint32_t byte_idx = bit_idx / 8;
     uint32_t bit_offset  = bit_idx % 8;

     if(value)
     {
         btmp->bits[byte_idx] |= (BITMAP_MASK << bit_offset);
     }
     else
     {
         /* “~”号表示逐位取反*/
         btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_offset);
     }
 }
