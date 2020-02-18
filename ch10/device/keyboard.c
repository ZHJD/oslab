#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"


/* 定义键盘缓冲区 */
ioqueue keyboard_buf;

/* 键盘控制器8024的输出缓冲区寄存器端口号 */
#define KBD_BUF_PORT 0x60

/* 用转义字符定义部分控制字符 */
#define esc         '\033' // 八进制表示字符
#define backspace   '\b'
#define tab         '\t'
#define enter       '\r'
#define delete      '\x7f'  // 八进制表示字符

/* 以下不可见字符定义为0 */
#define char_invisable 0
#define ctrl_l_char    char_invisable
#define ctrl_r_char    char_invisable
#define shift_l_char   char_invisable
#define shift_r_char   char_invisable
#define alt_l_char     char_invisable
#define alt_r_char     char_invisable
#define caps_lock_char char_invisable

/*******************************************
 * 定义控制字符的通码和断码，健被按下产生通码
 * 也就是make,弹开后产生断码break
 */ 
#define shift_l_make   0x2a
#define shift_r_make   0x36
#define alt_l_make     0x38
#define alt_r_make     0xe038
#define alt_r_break    0xe0b8
#define ctrl_l_make    0x1d
#define ctrl_r_make    0xe01d
#define ctrl_r_break   0xe09d
#define caps_lock_make 0x3a

/* 定义变量记录相应按键是否被按下，用于组合操作,true为按下，false弹起 */
static bool ctrl_status;
static bool shift_status;
static bool alt_status;
static bool caps_lock_status;

/* 扩展码标记 */
static bool ext_scancode;


/* 以下为通码make_code为索引的二维数组,索引是通码，
*  第一列是未与shift组合,第二列是和shift组合
*/
static char keymap[][2] = 
{
/* 扫描码未与shift组合*/
/*  *****************************************  */
/* 0x00 */      {0,             0},  // 没有通码为0的键
/* 0x01 */      {esc,           esc},
/* 0x02 */      {'1',           '!'},
/* 0z03 */      {'2',           '@'},
/* 0x04 */      {'3',           '#'},
/* 0z05 */      {'4',           '$'},
/* 0z06 */      {'5',           '%'},
/* 0z07 */      {'6',           '^'},
/* 0z08 */      {'7',           '&'},
/* 0z09 */      {'8',           '*'},
/* 0z0A */      {'9',           '{'},
/* 0z0B */      {'0',           '}'},
/* 0z0C */      {'-',           '_'},
/* 0z0D */      {'=',           '+'},
/* 0z0E */      {backspace,     backspace},
/* 0z0F */      {tab,           tab},
/* 0z10 */      {'q',           'Q'},
/* 0z11 */      {'w',           'W'},
/* 0z12 */      {'e',           'E'},
/* 0z13 */      {'r',           'R'},
/* 0z14 */      {'t',           'T'},
/* 0z15 */      {'y',           'Y'},
/* 0z16 */      {'u',           'U'},
/* 0z17 */      {'i',           'I'},
/* 0z18 */      {'o',           'O'},
/* 0z19 */      {'p',           'P'},
/* 0z1A */      {'[',           '{'},
/* 0z1B */      {']',           '}'},
/* 0z1C */      {enter,         enter},
/* 0z1D */      {ctrl_l_char,    ctrl_l_char},
/* 0z1E */      {'a',           'A'},
/* 0z1F */      {'s',           'S'},
/* 0z20 */      {'d',           'D'},
/* 0z21 */      {'f',           'F'},
/* 0z22 */      {'g',           'G'},
/* 0z23 */      {'h',           'H'},
/* 0z24 */      {'j',           'J'},
/* 0z25 */      {'k',           'K'},
/* 0z26 */      {'l',           'L'},
/* 0z27 */      {';',           ':'},
/* 0z28 */      {'\'',           '"'},
/* 0z29 */      {'`',           '~'},
/* 0z2A */      {shift_l_char,  shift_l_char},
/* 0z2B */      {'\\',           '|'},
/* 0z2C */      {'z',            'Z'},
/* 0z2D */      {'x',            'X'},
/* 0z2E */      {'c',            'C'},
/* 0z2F */      {'v',            'V'},
/* 0z30 */      {'b',            'B'},
/* 0z31 */      {'n',            'N'},
/* 0z32 */      {'m',            'M'},
/* 0z33 */      {',',            '<'},
/* 0z34 */      {'.',            '>'},
/* 0z35 */      {'/',            '?'},
/* 0z36 */      {shift_r_char,   shift_r_char},
/********* 小键盘区 *********/
/* 0z37 */      {'*',            '*'},
/* 0z38 */      {alt_l_char,     alt_l_char},
/* 0z39 */      {' ',            ' '},
/* 0z3A */      {caps_lock_char, caps_lock_char}
/******* 其它键暂时不处理 **********************/
};


/************************************************
 * 函数名:intr_keyboard_handler()
 * 功能:键盘驱动程序
 */ 
static void intr_keyboard_handler()
{
    /* 记录上次的按键是否是这三个控制键 */
    bool ctrl_down_last = ctrl_status;
    bool shift_down_last = shift_status;
    bool caps_lock_last = caps_lock_status;

    bool break_code;
    /* 从端口中读入9位数据 */
    uint8_t scancode = inb(KBD_BUF_PORT);

    if(scancode == 0xe0)
    {
        /* 打开扩展码标记 */
        ext_scancode = true;

        /***************************************
         * 退出中断处理程序，此次中断的编码不止1个字节,
         * 8042的特点是通码有几个字节，产生几次中断，
         */ 
        return;
    }
    /* 如果上次是以0xe0开头， 把扫描码合并 */
    if(ext_scancode)
    {
        scancode = ((0xe000) | scancode);

        /* 关闭e0标记 */
        ext_scancode = false;
    }

    /* 断码的第7位是1 */
    break_code = ((scancode & 0x0080) != 0);

    if(break_code)
    {
        /****************************************************
         * 断码 = 通码 + 0x80
         * 目前只处理两字节的
         */ 
        uint16_t make_code = (scancode & 0xff7f);

        /* 判断以下三个键是否被弹起  */
        if(make_code == ctrl_l_make || make_code == ctrl_r_make)
        {
            ctrl_status = false;
        }
        else if(make_code == alt_l_make || make_code == alt_r_make)
        {
            alt_status = false;
        }
        else if(make_code == shift_l_make || make_code == shift_r_make)
        {
            shift_status = false;
        }
        /* 由于caps_lock不是弹起后关闭，所以要单独处理 */

        return;
    }

    /* 若为通码则只处理已经定义的键 */
    else if((scancode > 0x00 && scancode < 0x3b) ||
            (scancode == alt_r_make) ||
            (scancode == ctrl_r_make))
    {
        /* 判断是否与shift结合 */
        bool shift = false;

        if((scancode < 0x0e) || (scancode == 0x29) ||
            (scancode == 0x1a) || (scancode == 0x1b) ||
            (scancode == 0x2b) || (scancode == 0x27) ||
            (scancode == 0x28) || (scancode == 0x33) ||
            (scancode == 0x34) || (scancode == 0x35))
        {
            /**** 代表两个字符的键 ***************
             * 0x0e 数字'0'-'9',字符'-','='
             * 0x29 字符'''
             * 0x1a 字符'['
             * 0x1b 字符']'
             * 0x2b 字符'\\'
             * 0x27 字符';'
             * 0x28 字符'\'
             * 0x33 字符','
             * 0x34 字符'.'
             * 0x35 字符'/'
             * 这些双字符键显示的内容和caps无关
             */
            shift = shift_down_last;
        }
        else // 处理字母的大写和小写问题
        {   
            if(shift_down_last && caps_lock_last)
            {
                /* 如果shift和caps同时按下 */
                shift = false;
            }
            else if(shift_down_last || caps_lock_last)
            {
                /* 如果shift和caps任意被按下 */
                shift = true;
            }
            else
            {
                shift = false;
            }
        }

        uint8_t index = (scancode &= 0x00ff);
        char cur_char = keymap[index][shift];

        /* 只处理asci不为0的键 */
        if(cur_char)
        {
           // put_char(cur_char);
            /* 放入键盘缓冲区 */
            ioq_putchar(&keyboard_buf, cur_char);
            return;
        }

        /* 记录本次是否按下下面几个控制键 */
        if(scancode == ctrl_l_make || scancode == ctrl_r_make)
        {
            ctrl_status = true;
        }
        else if(scancode == shift_l_make || scancode == shift_r_make)
        {
            shift_status = true;
        }
        else if(scancode == caps_lock_make)
        {
            caps_lock_status = !caps_lock_status;
        }
    }
    else
    {
         put_str("unknown key\n");
    }
}

void keyboard_init()
{
    ioqueue_init(&keyboard_buf);
    register_handler(0x21, intr_keyboard_handler);
}
