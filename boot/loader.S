%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR                       ;设置栈顶
jmp loader_start

;-------------------------构建gdt的内部描述符--------------------------
GDT_BASE:             dd    0x0000_0000              ;gdt的第0个位置不能使用，所以全部填充0
					  dd    0x0000_0000              ;dd----------define a double word

GDT_DESC:             dd    0x0000_ffff              ;定义代码段
					  dd    DESC_CODE_HIGH4 

DATA_STACK_DESC:      dd   0x0000_ffff               ;定义数据段
					  dd   DESC_DATA_HIGH4;

VIDEO_DESC:           dd   0x8000_0007               ;limit=(0xbffff - 0xb8000) / 4k = 0x7
					  dd   DESC_VIDEO_HIGH4          ;定义显存段，使用的是文字区

GDT_SIZE              equ  $ - GDT_BASE              ; '$'代表当前行的地址

GDT_LIMIT             equ  GDT_SIZE - 1
times 60 dq 0                                        ;times是伪指令，dq -- define quad word 64位，为后面留空间

;----------------以下是段选择子--------------------------------------------------------------
;------ 段选择子共16位，第0和1位是请求特权级
;------ 第2位用于区分是lgt还是gdt------------------------------------------------------------
SECTION_DATA          equ               (0x002<<3) + T1_GDT + RPL0  ;数据段选择子
SECTION_VIDEO         equ               (0x003<<3) + T1_GDT + RPL0  ;显存段选择子
SECTION_CODE          equ               (0x001<<3) + T1_GDT + RPL0  ;代码段选择子



;---------------------下面是gdt指针，前2字节是gdt界限，后4字节是gdt起始地址------------------
gdt_ptr:              dw                GDT_LIMIT
					  dd				GDT_BASE

loadermsg             db                '2 loader is real.'
loader_size           equ               $ - loadermsg

;--------------------------------------------------------------------------------------------

loader_start:
;--------------------------------------------------------------------------------------------
;INT  0x10          功能号:  0x13        功能描述:打印字符串
;--------------------------------------------------------------------------------------------








