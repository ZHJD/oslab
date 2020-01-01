; ---------------------------------loader 和 kernl----------------------------------
LOADER_BASE_ADDR equ    0x900
LOADER_START_SECTOR equ 0x1
PAGE_DIR_TABLE_POS  equ 0x10_0000   ;内核位于1MB以下的物理内存，页目录表位于内存开始处

;-------------------全局描述符段表属性----------------------------------------------
DESC_G_4K 	    	equ 1_000_0000_0000_0000_0000_0000b    ; 段界限的单位设置为4KB
DESC_D_32 	    	equ  1_00_0000_0000_0000_0000_0000b    ; 指令中的有效地址和操作是32位，使用EIP寄存器
DESC_L   	        equ   0_0_0000_0000_0000_0000_0000b    ; L为0表示32bit的代码段，1则表示64b的代码段
DESC_AVL 	        equ    0__0000_0000_0000_0000_0000b    ; CPU不使用这个bit故设置为0
DESC_LIMIT_CODE2    equ       1111_0000_0000_0000_0000b    ; 段界限的第二个字段
DESC_LIMIT_DATA2    equ     DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2   equ       0000_0000_0000_0000_0000b    ; 定义显存段    
DESC_P              equ           1_000_0000_0000_0000b    ; 段是否在内存中
DESC_DPL_0          equ            00_0_0000_0000_0000b    ; 特权级0
DESC_DPL_1          equ            01_0_0000_0000_0000b    ; 特权级1
DESC_DPL_2          equ            10_0_0000_0000_0000b    ; 特权级2
DESC_DPL_3          equ            11_0_0000_0000_0000b    ; 特权级3
DESC_S_CODE         equ               1_0000_0000_0000b    ; s为1表示非系统段
DESC_S_DATA         equ               DESC_S_CODE
DESC_S_SYS          equ               0_0000_0000_0000b    ; 系统段
DESC_TYPE_CODE      equ                 1000_0000_0000b    ; x = 1, c = 0, r = 0, a = 0
DESC_TYPE_DATA      equ                 0010_0000_0000b    ; x=0, c=0, w=1, a=0

DESC_CODE_HIGH4     equ                 (0x00<<24) + DESC_G_4K + DESC_D_32 + \
										DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
										DESC_P + DESC_DPL_0 + DESC_S_CODE + \
										DESC_TYPE_CODE + 0x00


DESC_DATA_HIGH4     equ                 (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
										DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
										DESC_P + DESC_DPL_0 + DESC_S_DATA + \
										DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4    equ                 (0x00<<24) + DESC_G_4K + DESC_D_32 + \
										DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + \
										DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b
;---------------------选择子属性------------------------------------
RPL0         equ         00b
RPL1		 equ         01b
RPL2         equ         10b
RPL3         equ         11b   ;RPL 表示请求特权级

T1_GDT       equ         000b  ;T1=0表示在GDT中索引，T1=1表示在LDT中索引
T1_LDT       equ         100b

;--------------------------------- 页表相关属性--------------------------------------
; 0 | 1  | 2  |
; p | RW | US |
;------------------------------------------------------------------------------------
PG_P	  equ     1b
PG_RW_R   equ     0_0b ; 可读u不可写
PG_RW_W   equ     10b  ; 可读可写
PG_US_S   equ     000b ; user/superuser 位,若为1，则表示处于user级别，若为0，表示处于superuser级别
PG_US_S   equ     100b
