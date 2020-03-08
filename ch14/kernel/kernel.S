[bits 32]
%define ERROR_CODE	nop   ; 这是一个宏，若在相关中断中有error_code,
            						  ; 则什么也不做
%define ZERO push 0		    ; 如果该中断cpu不会向栈中压入error_code,
						              ; 则手动压入32位0，进行占位
[bits 32]
extern syscall_table
section .text
global syscall_handler
syscall_handler:
    ; 保存上下文环境
    push 0    ; 统一上下文环境
    push ds
    push es
    push fs
    push gs

    pushad
    
    push 0x80 ; 压入中断号统一格式
    
   ; 为系统调用功能压入参数
    push edx    ; 系统调用中第3个参数
    push ecx    ; 系统调用中第2个参数
    push ebx    ; 系统调用中第1个参数
    
    call [syscall_table + eax * 4];
    add esp, 12
    
    ; 将call调用后的返回值存入当前内核栈中eax的位置，作为返回值
    mov [esp + 8*4], eax
    jmp intr_exit 

extern idt_table          ; 声明在c语言中定义的中断处理函数表
extern put_str 			      ; 声明外部函数
extern put_int
section .data			        ; 数据节，编译成elf文件，会自动合并属性相同的节

intr_str db "interrupt occur!", 0xa, 0 ; 一个字符串，每个字符占用一个字节，0xa换行,0表示字符串结尾

global intr_entry_table	 			   ; 声明全局符号，这是中断描述符表
intr_entry_table:				    	   ; 这个数组的起始地址,由于节会合并的缘故

; 定义了一个名为VECTOR的宏，输入参数为2个
; 输入参数(中断向量号，中断处理程序的第一条指令)
%macro VECTOR 2
section .text
intr%1entry:		; 每个中断向量号对应一个中断处理程序，%1会被替换为中断向量号
				      ; 这个标号代表了该中断处理程序的入口地址
   
    ; 打印esp值
   ; push esp
   ; call put_int
   ; add esp, 4
      
	%2			    	; nop或者push 0
  ; 保护上下文环境，为调用c函数做准备，因为调用c代码可能会破坏这些寄存器
  push ds
  push es
  push fs
  push gs
  pushad        ; 保护32为数据寄存器


              	; 如果是从片上进入中断，除了从片上发送EOI外， 还要给主片发送EOI信号
	mov al, 0x20	; 中断结束命令EOI
	out 0xa0, al	; 从片发送
	out 0x20, al	; 主片发送
  
  push %1       ; 中断向量号入栈
  
  ; 调用idt_table中c语言写的中断处理函数
  call [idt_table + %1 * 4]
  jmp intr_exit

section .data
	dd 	intr%1entry ; 存储每个中断入口程序的地址
					        ; 形成intr_entry_table数组
%endmacro		    	; 结束宏定义

section .text
global intr_exit
intr_exit:

  add esp, 4   ;根据调用约定,跳过中断号
  ; 下面恢复上下文环境
  popad
  pop gs
  pop fs
  pop es
  pop ds
  add esp, 4     ; 跳过error_code
  iretd          ; 中断返回

VECTOR 0x00, ZERO
VECTOR 0x01, ZERO
VECTOR 0x02, ZERO
VECTOR 0x03, ZERO
VECTOR 0x04, ZERO
VECTOR 0x05, ZERO
VECTOR 0x06, ZERO
VECTOR 0x07, ZERO
VECTOR 0x08, ZERO
VECTOR 0x09, ZERO
VECTOR 0x0a, ZERO
VECTOR 0x0b, ZERO
VECTOR 0x0c, ZERO
VECTOR 0x0d, ERROR_CODE
VECTOR 0x0e, ERROR_CODE
VECTOR 0x0f, ZERO
VECTOR 0x10, ZERO
VECTOR 0x11, ZERO
VECTOR 0x12, ZERO
VECTOR 0x13, ZERO
VECTOR 0x14, ZERO
VECTOR 0x15, ZERO
VECTOR 0x16, ZERO
VECTOR 0x17, ZERO
VECTOR 0x18, ZERO
VECTOR 0x19, ZERO
VECTOR 0x1a, ZERO
VECTOR 0x1b, ZERO
VECTOR 0x1c, ZERO
VECTOR 0x1d, ZERO
VECTOR 0x1e, ERROR_CODE
VECTOR 0x1f, ZERO
VECTOR 0x20, ZERO   ; 时钟
VECTOR 0x21, ZERO   ; 键盘
VECTOR 0x22, ZERO
VECTOR 0x23, ZERO
VECTOR 0x24, ZERO
VECTOR 0x25, ZERO
VECTOR 0x26, ZERO
VECTOR 0x27, ZERO
VECTOR 0x28, ZERO
VECTOR 0x29, ZERO
VECTOR 0x2A, ZERO
VECTOR 0x2B, ZERO
VECTOR 0x2C, ZERO
VECTOR 0x2D, ZERO
VECTOR 0x2E, ZERO   ; 硬盘
VECTOR 0x2f, ZERO














