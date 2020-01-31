[bits 32]
%define ERROR_CODE	nop   ; 这是一个宏，若在相关中断中有error_code,
						  ; 则什么也不做
%define ZERO push 0		  ; 如果该中断cpu不会向栈中压入error_code,
						  ; 则手动压入32位0，进行占位

extern put_str 			  ; 声明外部函数

section .data			  ; 数据节，编译成elf文件，会自动合并属性相同的节

intr_str db "interrupt occur!", 0xa, 0 ; 一个字符串，每个字符占用一个字节，0xa换行,0表示字符串结尾

global intr_entry_table				   ; 声明全局符号，这是中断描述符表
intr_entry_table:

; 定义了一个名为VECTOR的宏，输入参数为2个
; 输入参数(中断向量号，中断处理程序的第一条指令)
%macro VECTOR 2
section .text
intr%1entry:		; 每个中断向量号对应一个中断处理程序，%1会被替换为中断向量号
					; 这个标号代表了该中断处理程序的入口地址
	
	%2				; nop或者push 0
	push intr_str   ; 传入要打印的字符串首地址
	call put_str
	add  esp, 4		; 根据调用约定，清理栈


	; 如果是从片上进入中断，除了从片上发送EOI外， 还要给主片发送EOI信号
	mov al, 0x20	; 中断结束命令EOI
	out oxa0, al	; 从片发送
	out 0x20, al	; 主片发送

	add esp,  4		; 跳过error_code，或者手动加入的0
	iret			; 从中断返回

section .data
	dd 	intr%1entry ; 存储每个中断入口程序的地址
					; 形成intr_entry_table数组
%endmacro			; 结束宏定义

VECTOR 0x00 ZERO
VECTOR 0x01 ZERO
VECTOR 0x02 ZERO
VECTOR 0x03 ZERO
VECTOR 0x04 ZERO
VECTOR 0x05 ZERO
VECTOR 0x06 ZERO
VECTOR 0x07 ZERO
VECTOR 0x08 ZERO
VECTOR 0x09 ZERO
VECTOR 0x0a ZERO
VECTOR 0x0b ZERO
VECTOR 0x0c ZERO
VECTOR 0x0d ZERO
VECTOR 0x0e ZERO
VECTOR 0x0f ZERO
VECTOR 0x10 ZERO
VECTOR 0x11 ZERO
VECTOR 0x12 ZERO
VECTOR 0x13 ZERO
VECTOR 0x14 ZERO
VECTOR 0x15 ZERO
VECTOR 0x16 ZERO
VECTOR 0x17 ZERO
VECTOR 0x18 ZERO
VECTOR 0x19 ZERO
VECTOR 0x1a ZERO
VECTOR 0x1b ZERO
VECTOR 0x1c ZERO
VECTOR 0x1d ZERO
VECTOR 0x1e ERROR_CODE
VECTOR 0x1f ZERO
VECTOR 0x20 ZERO

