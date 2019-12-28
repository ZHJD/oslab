; 主引导程序
; ----------------------
%include "boot.inc"
SECTION MBR vstart=0x7c00 ; 此处等号左右不能有空格出现，不然编译不通过
	; 保存主要寄存器内容
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov sp, 0x7c00
	mov ax, 0xb800
	mov gs, ax	
	
	; 清屏使用0x06号功能，上卷全部行以实现清屏
	; ------------------------------------------
	; INT 0x10  功能号:0x06  功能描述: 上卷窗口
	; ------------------------------------------
	; 输入
	; AH 功能号: 0x06
	; AL = 上卷的行数(如果为0表示全部行)
	; BH = 上卷行的属性
	; (CL, CH) = 窗口左上角的(X,Y)位置
        ; (DL, DH) = 窗口右下角的位置
	; 无返回值
	;-------------------------------------------
	
	mov ax, 0x600
	mov bx, 0x700
	mov cx, 0
	mov dx, 0x184f   ; VGA文本模式中，一行只能容纳80个字符，共25行，
			 ; 下标从0开始	
	int 0x10

	; 获取光标位置
	
	mov ah, 3     ; 3号功能是获取光标位置，提前存入ah寄存器
	mov bh, 0     ; bh寄存器存储待获取光标的页号
	
	int 0x10      ; 输出: ch = 光标开始行， cl = 光标结束行
		      ; dh = 光标所在行号， dl = 光标所在列号
        ; 获取光标位置结束

	; 打印字符串
        ; 输出背景色绿色，前景色红色，且跳动的字符串
	; -----------------------------------------------------
	mov byte [gs:0x00], '1'
	mov byte [gs:0x01], 0xA4
	
	mov byte [gs:0x02], ' '
	mov byte [gs:0x03], 0xA4

	mov byte [gs:0x04], 'M'
	mov byte [gs:0x05], 0xA4
	; 打印字符串结束
	
	;----------------------------加载loader----------------------------
	mov eax, LOADER_START_SECTOR ; 起始扇区地址
	mov bx, LOADER_BASE_ADDR     ; 写入的内存地址
	mov cx, 4                    ; 待读入的扇区数
	call rd_disk_m_16            ; 以下读取程序的起始部分
	jmp LOADER_BASE_ADDR
	;-----------------------------------------------------------------

	;-----------------------------------------------------------------
	;功能: 读取硬盘n个扇区
	rd_disk_m_16:
	;--------- 参数--------------------------------------------------------
		     ; eax
		     ; bx
		     ; cx
	mov esi, eax ; 备份eax
	mov di,  cx  ; 备份cx
	
	;读写硬盘;
	; 第一步，设置要读取的扇区数
	
	mov dx, 0x1f2
	mov al, cl
	out dx, al ; 设置要读取的扇区数
	
	mov eax, esi   ; 恢复ax
	
	;第二步  把LBA（逻辑磁盘地质）写入0x1f3 - 0x1f6
	mov dx, 0x1f3
	out dx, al
	
	mov cl, 8
	shr eax, cl
	mov dx, 0x1f4
	out dx, al

	shr eax, cl
	mov dx, 0x1f5
	out dx, al
	
	shr eax, cl
	and al, 0x0f ; 高四位全部置0，以避免干扰
	or  al, 0xe0
	mov dx, 0x1f6
	out dx, al
	;--------------------------写入地址结束----------------------------
	
	;第三步，向0x1f7中写入读取磁盘的命令
	mov dx, 0x1f7
	mov al, 0x20
        out dx, al
	;-----------------------------------------------------------------

	;第四步，读状态寄存器0x1f7,检查是否读取成功
	
	.not_ready:
		nop           ; 延迟一下
		mov dx, 0x1f7 ; 写入该寄存器的是命令，读出的是硬盘的状态
		in  al, dx
		and al, 0x88
		cmp al, 0x08
		jnz .not_ready 
	
	; 计算要读取的子节数
	mov ax, di ; di = cx为要读取的扇区数
	mov dx, 256
	mul dx
	mov cx, ax ; cx中保存循环次数
	
	mov dx, 0x1f0	
	;第五步，读取数据
	.go_on_ready:
		in ax, dx
		mov [bx], ax
		add bx,  2
		loop .go_on_ready
		ret

	times 510 - ($ - $$) db 0
	db 0x55, 0xaa
