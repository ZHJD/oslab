[bits 32]
section .text
;;;;;;;;;;;;;;;;;;;;;;; switch_to 函数 ;;;;;;;;;;;;;;;;;;;;;;;;;
;参数:
;   cur:指向task_struct的指针
;   next:指向task_struct的指针
;功能:保护上下文，切换任务
global switch_to
switch_to:
    ; 栈中此处是返回地址
    push esi
    push edi
    push ebx
    push ebp
    
    ; 得到栈中参数cur
    mov eax, [esp + 4 * 5]
    ; 保存此时的栈顶指针esp,self_kstack在task_struct中偏移为0
    ; 所以直接在thread开头保存即可
    mov [eax], esp

;--------- 以上是备份当前线程的环境，下面是恢复下一个线程的环境 ------------
    mov eax, [esp + 24] ; 获取参数next
    ; next的值同时也是task_struct第一个成员self_kstack的地址，里面存放的是
    ; 0级栈的栈顶指针
    mov esp, [eax]        ; 恢复esp

    pop ebp
    pop ebx
    pop edi
    pop esi

    ret ; 若未通过中断进入，则会返回到kernel_thread处执行
        ; 若由中断进入，则会返回到进入该函数前的地址

