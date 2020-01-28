TI_GDT	equ		0
RPL0    equ     0
SELECTOR_VIDEO   equ   (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text
;------------------------------put_char-----------------------------------
;功能描述:	把栈中的1个字符写入光标所在处
;-------------------------------------------------------------------------
global put_char
put_char:
	pushad     ;备份32位寄存器环境
	;需要保证gs中为正确的段选自子
	;为保险起见，每次打印时都为gs赋值
	mov ax, SELECTOR_VIDEO
	mov gs, ax				;不能直接把立即数送入段寄存器


;;;;;;; 获取当前光标位至 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;先获得高8位
	mov dx,	 0x03d4  ; 索引寄存器
	mov ax,  0x0e	 ; 用于提供光标位置的高8位
	out dx,  al
	mov dx,  0x03d5	 ; 通过读写数据端口0xd5来获得或者设置光标位置
	in  al,  dx		 ; 得到了光标位置的高8位
	mov ah,  al

	;再获取低8位
	mov dx,  0x03d4
	mov al,  0x0f
	out dx,  al
	mov dx,  0x03d5
	in  al,  dx

	;把光标存入bx
	mov bx,  ax
	;下面这行是在栈中获取待打印的字符
	mov ecx, [esp + 36]  ; pushsd押入4 * 8 = 32字节
						 ; 加上主调函数4字节的返回地址， 故esp + 36字节
    mov cl,  0xd                     ; CR是0x0d, LP是0x0a
	jz  .is_carriage_return
	cmp cl,  0xa
	jz  .is_line_feed

	cmp cl, 0x8
	jz  .is_backspace

	jmp  .put_other
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
