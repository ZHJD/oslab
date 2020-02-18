#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H
#include "ioqueue.h"

/* 键盘缓冲区是共享变量 */
extern ioqueue keyboard_buf;

void keyboard_init();

#endif
