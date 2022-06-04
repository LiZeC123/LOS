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
    pop ebp             ; 从而进一步恢复该线程的寄存器值
    pop ebx
    pop edi
    pop esi
    ret                 ; 执行ret时, esp正好执行PCB中的eip, 从而将eip指定的位置当做返回地址返回
                        ; 