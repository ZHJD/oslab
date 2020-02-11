[bits 32]
global set_cursor
set_cursor:
;;;;;;;;;;;;;;;;;;; set_cursor 函数 ;;;;;;;;;;;;;;;;;;;;;
;参数:
;   屏幕光标，通过栈传参数
;返回值:无

;   此处是call指令压入的返回值
; ABI要求被调用函数保护的寄存器
    push esi
    push edi
    push ebx
    push ebp
    ; esp 由调用约定设置
    
    ; 获取屏幕光标值
    mov ebx, [esp + 4 * 5]

    ; 把光标设为bx的值
;;;;;;;;;;;;;;;;;;; 1 设置高8为 ;;;;;;;;;;;;;;;;;;;;
    mov dx, 0x03d4      ; 索引寄存器
    mov al, 0x0e        ; 提供光标值的高8位
    out dx, al          
    mov dx, 0x03d5      ; 读写该端口设置光标位置
    mov al, bh      
    out dx, al
    
;;;;;;;;;;;;;;;;; 2 设置低8位 ;;;;;;;;;;;;;;;;;;;;;
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
; 恢复现场
    pop ebp
    pop ebx
    pop edi
    pop esi
    ret

