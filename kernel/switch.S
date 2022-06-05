[bits 32]
section .text
global switch_to
switch_to:
    push esi
    push edi
    push ebx
    push ebp
    mov eax, [esp+20]   ; esp+20是当前线程的PCB起始地址
    mov [eax], esp      ; 将当前的栈指针值写入PCB偏移0的地方, 即self_kstack

    mov eax, [esp + 24] ; esp+20是下一个线程的PCB的起始地址
    mov esp, [eax]      ; 从self_kstack恢复该线程的esp值
    pop ebp             ; 初始情况下, self_kstack指向ThreadStack的起始位置
    pop ebx             ; 因此后续弹出的值是其中设置的初始值
    pop edi             ; 执行ret时, esp正好指向ThreadStack中的eip变量, 从而将eip指定的位置当做返回地址返回
    pop esi             ; 这相当于虚空调用了一个函数后返回到eip指定的kernel_thread函数之中
    ret                 ;
                        ; 在第二次执行时, self_kstack已经指向了栈的其他位置, 而switch_to函数实际上是被schedule函数调用
                        ; 因此, 此时ret会回到schedule函数之中, 并最终从中断函数返回
                        ; 进出中断函数会执行上下文备份和恢复, 因此可以完整的恢复到中断之前的状态