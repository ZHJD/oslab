; 通过写显卡实现put_char函数

TI_GDT	equ		0
RPL0    equ     0
SELECTOR_VIDEO   equ   (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text
;------------------------------put_char-----------------------------------
;功能描述:	把栈中的1个字符写入光标所在处
;-------------------------------------------------------------------------
global put_char ;函数名字导出为全局符号
put_char:       ; push all double的缩写，该指令压入双字长的寄存器，所有的一共是8个
	pushad     ;备份32位寄存器环境
	;需要保证gs中为正确的段选自子
	;为保险起见，每次打印时都为gs赋值
	mov ax, SELECTOR_VIDEO
	mov gs, ax				;不能直接把立即数送入段寄存器


;;;;;;; 获取当前光标位至 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;先获得高8位
	;向端口0x03d4中写入寄存器索引，之后再向数据寄存器中写入数值
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
	mov bx,  ax ; 此时光标存在于bx寄存器中，光标是一个一维数组
	;下面这行是在栈中获取待打印的字符
	mov ecx, [esp + 36]  ; pushsd押入4 * 8 = 32字节
						 ; 加上主调函数4字节的返回地址， 故esp + 36字节
    cmp cl,  0xd                     ; CR是0x0d, LP是0x0a
	jz  .is_carriage_return  ; CR是回车符，LF是换行符
	cmp cl,  0xa
	jz  .is_line_feed

	cmp cl, 0x8            ; backspace的关键码是asc是8
	jz  .is_backspace

	jmp  .put_other		   ; 如果不是以上几个控制字符，则认为是可见字符
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.is_backspace:
	; 此时bx的值是等待打印字符的位置，由于2个字节存储一个字符，所以要减去1乘以2，
	dec  bx				  ; 是退格后在显存中的地址
	shl  bx, 1

	mov byte [gs:bx], 0x20  ; 低字节处，填补空格0x20
	inc bx
	mov byte [gs:bx], 0x07  ; 高字节处，填补属性
	shr bx, 1				; 恢复坐标值
	jmp .set_cursor         ; bx的值要写入光标寄存器才可以
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.put_other:
	shl	bx, 1               ; bx为显存偏移数值
	
	mov [gs:bx], cl         ; ecx中存储的是待显示字符的数值
	inc bx
	mov byte [gs:bx], 0x07	; 字符属性
	shr bx,	 1		      	; 恢复老的光标值
	inc	bx
	cmp bx,		2000		; 2000是一个屏幕能显示的最大字符数
	jl	.set_cursor			; 如果小于2000，更新bx到显卡的寄存器当中
							; 否则按回车符号处理，并且进行滚屏

.is_line_feed:				; 是换行符LF('\n')
.is_carriage_return:		; 是回车符CR(\r)
							; 如果是换行符，则把光标移到行首位置
	xor dx,	dx				; dx是被除数的高16位
	mov ax, bx				; bx是被除数的低16位

	mov si, 80				; 处理为下一行的行数

	div si

	sub bx, dx				; 此时bx为当前行行首

.is_carriage_return_end:
	add bx, 80				; 计算后为下一行的行首
	cmp bx, 2000			
.is_line_feed_end:
	jl .set_cursor  		; 如果小于2000，直接写入显卡

; 滚屏方式是把(1 - 24) --> (0 -- 23)行，24行填上空格
.roll_screen:				; 否则进行滚屏
	cld
	mov ecx, 960			; 1920 * 2 / 4 = 960

	mov esi, 0xc00b80a0		; 第1行首地址
	mov edi, 0xc00b8000		; 第0行首地址
	rep movsd

;;;;;最后一行填充为空的0
	mov ebx, 3840
	mov ecx, 80

.cls:
	mov word [gs:ebx], 0
	add ebx, 2
	loop .cls
	mov ebx, 1920

.set_cursor:
; 把光标设置为bx的数值
;;;;; 先设置为高8位 ;;;;;;;;;;;;;;;;;;;;
	mov dx, 0x03d4
	mov al, 0x0e
	out dx, al
	mov dx, 0x03d5
	mov al, bh
	out dx, al


	mov dx, 0x03d4
	mov al, 0x0f
	out dx, al
	mov dx, 0x03d5
	mov al, bl
	out dx, al

.put_char_done:
	popad
	ret
