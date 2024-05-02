%include "boot.inc"

; 保护模式下, 选择子低3bit是属性, 高13bit是索引
SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

[bits 32]
section .text
global put_char
put_char:
    pushad                  ; 备份寄存器缓存, 8个寄存器会全部入栈
    mov ax, SELECTOR_VIDEO  ; 重新给视频选择子赋值, 确保该寄存器取值是正确的
    mov gs, ax

    ; 获取当前光标位置
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
    mov bx, ax      ; 将光标的坐标写入bx


    ; 获取栈上的字符, 并判断其类型
    mov ecx, [esp + 36]
    cmp cl, 0xd
    jz .is_carriage_return
    cmp cl, 0xa
    jz .is_line_feed
    cmp cl, 0x8
    jz .is_backspace
    jmp .put_other


.is_backspace:
    dec bx
    shl bx, 1

    mov byte [gs:bx], 0x20
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    jmp .set_cursor


.put_other:
    shl bx, 1
    mov byte [gs:bx], cl
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    inc bx
    cmp bx, 2000            ; 如果没有超过屏幕限制, 则设置新的光标值
    jl .set_cursor          ; 否则附带进行一次换行操作

.is_carriage_return:
.is_line_feed:
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si          ; dx:ax / si , 即对坐标除以80 
    sub bx, dx      ; 余数表示当前行已经打印的字符数, 减去后光标回到当前行的起始位置

.is_carriage_return_end:
    add bx, 80      ; 加80后, 光标移动到下一行的末尾
    cmp bx, 2000
.is_line_feed_end:
    jl .set_cursor

.roll_screen:
    cld                 ; 滚屏操作实际上就是从第二行开始把数据复制到第一行开始的位置
    mov ecx, 960
    mov esi, 0xc00b80a0
    mov edi, 0xc00b8000
    rep movsd
    mov ebx, 3840       ; 然后将最后一行填充空白
    mov ecx, 80
.cls:
    mov word [gs:ebx], 0x0720
    add ebx, 2
    loop .cls
    mov bx, 1920

.set_cursor:
    ; 将更新后的光标位置写入显卡的寄存器
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

global cls_screen
cls_screen:
   pushad
   ;;;;;;;;;;;;;;;
	; 由于用户程序的cpl为3,显存段的dpl为0,故用于显存段的选择子gs在低于自己特权的环境中为0,
	; 导致用户程序再次进入中断后,gs为0,故直接在put_str中每次都为gs赋值. 
   mov ax, SELECTOR_VIDEO	       ; 不能直接把立即数送入gs,须由ax中转
   mov gs, ax

   mov ebx, 0
   mov ecx, 80*25
 .cls:
   mov word [gs:ebx], 0x0720		  ;0x0720是黑底白字的空格键
   add ebx, 2
   loop .cls 
   mov ebx, 0

 .set_cursor:				  ;直接把set_cursor搬过来用,省事
;;;;;;; 1 先设置高8位 ;;;;;;;;
   mov dx, 0x03d4			  ;索引寄存器
   mov al, 0x0e				  ;用于提供光标位置的高8位
   out dx, al
   mov dx, 0x03d5			  ;通过读写数据端口0x3d5来获得或设置光标位置 
   mov al, bh
   out dx, al

;;;;;;; 2 再设置低8位 ;;;;;;;;;
   mov dx, 0x03d4
   mov al, 0x0f
   out dx, al
   mov dx, 0x03d5 
   mov al, bl
   out dx, al
   popad
   ret
