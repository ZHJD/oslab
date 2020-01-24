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
	mov  eax,  0xe820 ; 执行中断后，eax变为0x534d4150
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

;-------------------加载kernel------------------------------------------------------------------

mov eax, KERNEL_START_SECTOR  ; kernel.bin 所在的扇区号
mov ebx, KERNEL_BIN_BASE_ADDR ; 
; 从磁盘读出后， 写入到ebx指定的地址
mov ecx, 200

call rd_disk_m_32


;------------------------------------启用分页机制-------------------------------------------------
; 创建页目录及页表并初始化页内存位置
call setup_page

; 把描述符表地址及偏移梁写入到内存gdt_ptr处，之后重新加载
sgdt [gdt_ptr]
; [gdt_ptr]共6字节，前2字节存储gdt表长度，后四字节存储gdt表基地址

; 把显存段的段基址放在内核空间中
mov ebx, [gdt_ptr + 2]
or dword [ebx + 0x18 + 4], 0xc000_0000  ; 显存段位于第三段，表中每项8字节

; 把gdt基址加上0xc000_0000，使其成为内核地址
add dword [gdt_ptr + 2], 0xc000_0000
add esp, 0xc000_0000 ; 把栈指针移到内核中

; 把页目录地址赋给cr3
mov eax, PAGE_DIR_TABLE_POS
mov cr3, eax                ; 存放的是页目标表的物理地址，不需要再做变换

; 打开cr0的PG位， 位于cr0的第32位
mov eax, cr0
or  eax, 0x8000_0000
mov cr0, eax

; 开启分页后，用gdt新的地址重新加载
lgdt [gdt_ptr] ; 必须在开启分页后再重新加载，因为此时的段描述符表中的地址是虚地址

mov byte [gs:480], 'V'
jmp $

;---------------------读磁盘------------------------------------------------------------------
rd_disk_m_32:
;---------------------------------------------------------------------------------------------
													; eax = LBA 扇区号
													; ebx = 将数据写入的内存地址
													; ecx = 读入的扇区数
	mov esi, eax  ; 备份eax
	mov di,	 cx   ; 备份ecx

;读写硬盘:
;设置要读取的扇区数:
	mov dx, 0x1f2;
	mov al, cl
	out dx, al

	mov eax, esi   ; 恢复eax

;第二步，把LBA地址存入0x1f3-0x1f6
	
	; 0-7位写入端口0x1f3
	mov dx, 0x1f3
	out dx, al

	; LBA8-15位地址存入0x1f4
	mov cl, 8
	shr eax, cl ; 整体向右移动8bit
	mov dx, 0x1f4
	out dx, al

	; LBA 16-23 bit
	shr eax, cl
	mov ax, 0x1f5
	out dx, al
	
	shr eax, cl
	and al, 0x0f ; lba第24-27bit
	or  al, 0xe0 ; 设置4-7bit为1110， 表示lba模式
	mov dx, 0x1f6
	out dx, al

; 第三步，向0x20端口写入读命令
	mov dx, 0x1f7
	mov al, 0x20
	out dx, al
; 第四步，检测硬盘状态
.not_ready:
	;同一端口，写的时候表示写入命令字，读的时候表示读入硬盘成功
	nop
	in al, dx
	and al, 0x88 ; 第三位为1表示硬盘控制器已准备好数据传输，第七位为1表示硬盘忙
	cmp    al, 0x08 ; 
	jnz .not_ready

; 从0x1f0端口读数据
	mov ax, di
	mov dx, 256 ; 一个扇区有512字节，每次读入一个字，共需要di*512/2,所以di*256
	mul dx
	mov cx, ax
	mov dx, 0x1f8
.go_on_read:
	in ax, dx
	mov [bx], ax
	add bx, 2
	loop .go_on_read
	ret






;-----------------------创建页目录及页表--------------------------------------------------------
setup_page:
; 把页目录所使用的空间全部清0
	mov ecx, 4096  ; 二级页目录项总共占4k,即4096B
	mov esi, 0;
.clear_byte:
	mov byte [PAGE_DIR_TABLE_POS + esi], 0
	inc esi
	loop .clear_byte

; 开始创建页目录项(PDE) page directory entry
.create_pde:
	mov eax, PAGE_DIR_TABLE_POS
	add eax, 4096 ; eax 直是第一个页表的物理地址
	mov ebx, eax


	or eax, PG_US_U | PG_RW_W | PG_P

	mov [PAGE_DIR_TABLE_POS + 0x0], eax    ; 确保使用分页机制前的物理地址和使用分页机制后虚拟地址
										   ; 对应的物理地址相同

	mov [PAGE_DIR_TABLE_POS + 0xc00], eax  ; 把高1GB虚拟地址映射到低端1MB处

	sub eax, 4096
	mov [PAGE_DIR_TABLE_POS + 4092], eax

; 创建页表项(PTE)
	mov ecx, 256         ; 1M低端内存 / 每页大小4K = 256
						 ; 1M的内核仅仅需要256个页表项
	mov esi, 0
	mov edx, PG_US_U | PG_RW_W | PG_P
.create_pte:
	mov [ebx + esi], edx
	add esi, 4
	add edx, 4096
	loop .create_pte

; 创建其他pde  虚拟地址的高1GB的映射
	mov ecx, 1022 - 768
	mov esi, 769
	add eax, 0x2000
.create_kernal_pde:
	mov [PAGE_DIR_TABLE_POS + esi * 4], eax
	inc esi
	add eax, 0x1000
	loop .create_kernal_pde
	ret

	


