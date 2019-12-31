%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR                       ;设置栈顶

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

;存放可用内存容量，此处偏移地址为0x200
total_mem_bytes       dd   0

;----------------以下是段选择子--------------------------------------------------------------
;------ 段选择子共16位，第0和1位是请求特权级
;------ 第2位用于区分是lgt还是gdt------------------------------------------------------------
SECTION_DATA          equ               (0x002<<3) + T1_GDT + RPL0  ;数据段选择子
SECTION_VIDEO         equ               (0x003<<3) + T1_GDT + RPL0  ;显存段选择子
SECTION_CODE          equ               (0x001<<3) + T1_GDT + RPL0  ;代码段选择子



;---------------------下面是gdt指针，前2字节是gdt界限，后4字节是gdt起始地址------------------
gdt_ptr:              dw                GDT_LIMIT
					  dd				GDT_BASE

;设置ards缓冲区
ards_buf times 224 db  0
ards_nr               dw                0 ; 用于记录ARDS结构体数量

; 错误提示消息
error_msg             db  'get memory failed!  '      
msg_size              equ  $ - error_msg

;此处偏移开始0x300
;--------------------------------------------------------------------------------------------

;--------------------e820获取内存大小------------------------------------
	xor  ebx,  ebx ;第一次调用时，ebx直要为0
	mov  edx,  0x534d4150 ; edx只赋值一次，循环体中不会改变
	mov  di,   ards_buf   ;ards结构体缓冲区

.e820_mem_get_loop:
	mov  ecx,  20  ;ARDS地址范围描述符结构大小是20字节
	int  0x15      
	jc   .e820_failed_so_try_e801  ;cf位为1则有错误发生，尝试0xe801子功能
	add  di,  cx
	inc  word [ards_nr]  ;记录ards数量
	cmp  ebx, 0 ; 若ebx为0且cf为1，则ards全部返回
	jnz  .e820_mem_get_loop

; 在所有的ards结构中，找出(base_add_low + length_low)的最大值，即内存的容量
	mov  cx, [ards_nr]
; 遍历所有的ards结构体
	mov  ebx, ards_buf
	xor  edx, edx     ;edx存储最大内存容量
.find_max_mem:  ;无需检查type类型
	mov  eax, [ebx]      ;base_low 32 bit
	add  eax, [ebx + 8]  ;内存长度的低32bit，字节为单位
	add  ebx, 20         ;指向缓冲区的下一个结构体
	cmp  edx, eax
	jge  .next_ards
	mov  edx, eax
.next_ards:
	loop .find_max_mem

	jmp .mem_get_ok

;----------------------------------------------------------------------------------------
; 使用e801获取最大内存大小
.e820_failed_so_try_e801:
	mov ax,  0xe801
	xor edx, edx   ;edx表示最大内存大小，提前清0
	int 0x15
	jc  .e801_failed_so_try_88
	
	; 先处理15MB以下的内存容量，单位是1KB
	mov cx, 0x400   ;0x400b = 1kb
	mul cx          ;ax 是被乘数，结果的低16bit在ax中，高16bit在dx中
	shl edx, 16     ;左移低位补0
	and eax, 0x0000_ffff
	or  edx, eax;   ;eax的高16位是0，低16bit是运算结果，edx的低16bit是0
	add edx, 0x10_0000  ; 结果要加上1MB
	mov esi, edx         ;备份edx

	; 处理16MB～4GB，此处单位是64KB
	mov eax, 0x00010000  ; 0x00010000表示64kb
	and ebx, 0x0000_ffff
	mul ebx               ; 低32bit在eax中，高32bit在edx中，
						 ; 由于乘积小于4GB,因此高位edx为0
	add eax, esi         ; 和16MB相加
	mov edx, eax
	jmp .mem_get_ok
;--------------------------------------------------------------------------------------------
; 使用0x88获取内存大小，最多获取64MB内存
.e801_failed_so_try_88:
	mov ah, 0x88
	int 0x15
	jc  .error_hit
	and eax, 0x0000_ffff
	mov cx, 0x400   ; 单位是4KB
	mul cx
	shl edx, 16
	or  edx, eax
	add edx, 0x100000 ;加上1MB

;------------------------------------------------------------------------------------------------
;处理失败的代码要放在前面，不然获取内存成功后也会执行这段代码
	mov cx, msg_size
	mov bx, error_msg
	mov dx, 320       ;第三行输出错误消息
	mov ax, 0xb800
	mov es, ax

.error_hit:
	mov al, [bx]
	mov di, bx  ;备份bx
	mov bx, dx
	mov [es:bx], al
	mov bx, di ; 恢复bx
	add dx, 2
	inc bx  ; 指向下一个字符x
	loop .error_hit
	jmp $              ; 出错，进行死循环
;-----------------------------------------------------------------------------------------------
.mem_get_ok:
	mov [total_mem_bytes], edx


;-----------------------------------------------------------------------------------------	


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
    ; 并更新段缓冲寄存器

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




