#ifndef __KERNEL_BITMAP_H
#define __KERNEL_BITMAP_H
#include "global.h"
#include "stdbool.h"


typedef struct bitmap
{
    uint32_t btmp_bytes_len;
    /*在遍历位图的时候，整体上以字节为单位，最后逐位遍历*/
    uint8_t* bits;
}bitmap;

/**************************************
 * 函数名:bitmap_init()
 * 功能:位图中所有位设置为0
 * 返回值:无
 */
 void bitmap_init(bitmap* btmp);

 /**************************************
  * 函数名：bitmap_scan_test()
  * 判断bit_idx位是否为1，若为1，返回true，否则返回false
  * bit_idx:bit索引
  * btmp:指向位图的指针
  * 返回值:true or false
  */
 bool bitmap_scan_test(const bitmap* btmp, const uint32_t bit_idx);

 /**********************************************************
  * 函数名:bitmap_scan()
  * 在位图中连续申请cnt个位，成功返回起始位下标，失败返回-1
  * btmp:指向位图的指针
  * cnt:申请位的个数
  * 返回值:起始位的指针或者-1
  */
 int bitmap_scan(const bitmap* btmp, const uint32_t cnt);

 /**************************************************************
  * 函数名:bitmap_set()
  * 功能:把bit_idx处设置为value
  * btmp:指向位图的指针
  * bit_idx:bit索引
  * value:二进制数
  * 返回值:无
  */
 void bit_map_set(bitmap* btmp, const uint32_t bit_idx, const int8_t value);


#endif
