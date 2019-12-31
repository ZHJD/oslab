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
;输入
;AH 子功能号 13h
;BH 页码
;BL = 属性
;CX = 字符串长度
;(DH,DL) = 坐标(行，列)
;ES:BF = 字符串起始地址
;AL 显示输出方式
;无返回值
;-------------------------------------------------------------------------------------------
	mov  sp,  LOADER_BASE_ADDR
	mov  bp,  loadermsg            ;ES:BP 字符串首地址,es在mbr中已经初始化为0
	mov  cx,  loader_size          ;cx字符串长度
	mov  ax,  0x1301               ;AH = 13, Al = 01
	mov  bx,  0x001f               ;页蒿为0
	mov  dx,  0x1800               
	int  0x10                      ;10h号中断

;------------------------------------------------------------------------------------------
; 打开A20
; 加载gdt
; 将cr0的pe位置1
;------------------------------------------------------------------------------------------

;---------------------打开A20--------------------------------------------------------------

	in  al, 0x92
	or  al, 0000_0010b
	out 0x92, al

;--------------------加载GDT---------------------------------------------------------------
   lgdt [gdt_ptr]

;--------------------------------cr0第0位设置为1-------------------------------------------
	mov eax, cr0             ;pe位设置为1
	or  eax, 0x1           
	mov cr0, eax


	jmp dword SECTION_CODE: p_mode_start ; 刷新流水线


[bits 32]
p_mode_start:
	mov ax,    SECTION_DATA
	mov ds,    ax
	mov es,    ax
	mov ss,    ax
	mov esp,   LOADER_BASE_ADDR
	mov ax,    SECTION_VIDEO
	mov gs,    ax


	mov byte  [gs:160],  'P'  ;一行有80个字符，显示一个字符占2个字节，此处显示在第二行第一个位置
	mov byte  [gs:161],  0xa4
	jmp $




