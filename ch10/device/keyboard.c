#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"

/* 键盘控制器8024的输出缓冲区寄存器端口号 */
#define KBD_BUF_PORT 0x60

static void intr_keyboard_handler()
{
    put_char('k');
    uint8_t code = inb(KBD_BUF_PORT);
    put_int(code);
}

void keyboard_init()
{
    register_handler(0x21, intr_keyboard_handler);
}
