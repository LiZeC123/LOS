%include "boot.inc"

SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text
global put_char
put_char:
    pushad
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    in al, dx
    mov ah, al
    
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al 
    mov dx, 0x03d5
    in al, dx

    mov bx, ax
    mov ecx, [esp + 36]

    cmp cl, 0xd
    jz .is_carriage_return
    cmp cl, 0xa
    jz .is_line_feed

    cmp cl, 0x8
    jz .is_backspace
    jmp .put_other

.is_carriage_return:
.is_line_feed:
.is_backspace:
.put_other:
    ret

