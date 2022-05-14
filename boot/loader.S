%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
  jmp loader_start

  ; GDT表开始的位置, 第0行为空
  GDT_BASE: dd 0x0000_0000
            dd 0x0000_0000
  ; GDT表第一项, 存储代码段相关属性
  CODE_DESC: dd 0x0000_FFFF
             dd DESC_CODE_HIGH4

  DATA_STACK_DESC: dd 0x0000_FFFF
                   dd DESC_DATA_HIGH4

  VIDEO_DESC: dd 0x8000_0007      ; 显存的位置是0xb80000, 因此低位设置为0x8000, 第二段基地址设置为0xb
                                  ; 32KB显存可以分为8个4K, 从0开始计数, 因此段限长设置为0x7
              dd DESC_VIDEO_HIGH4

  GDT_SIZE  equ $ - GDT_BASE
  GDT_LIMIT equ GDT_SIZE - 1
  ; 使用空白预留60个表项的空间
  times 60 dq 0

  ; 定义对应的选择子, 代码段的索引是1, 其他段同理
  SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
  SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
  SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

  ; GDT指针, 与GDTR寄存器的内容一致, 第2字节为LIMIT值, 高4字节为地址值
  gdt_ptr dw GDT_LIMIT
          dd GDT_BASE
  
  loadermsg db '2 loader in real'

loader_start:
  mov sp, LOADER_BASE_ADDR
  mov bp, loadermsg
  mov cx, 17
  mov ax, 0x1301
  mov bx, 0x001f
  mov dx, 0x1800
  int 0x10

; 准备进入保护模式
; 打开A20
  in al, 0x92
  or al, 0000_0010b
  out 0x92, al 
; 设置GDT寄存器
  lgdt [gdt_ptr]
; CR0寄存器设置PE位
  mov eax, cr0
  or eax, 0x00000001
  mov cr0, eax
  ; 刷新流水线
  jmp dword SELECTOR_CODE:p_mode_start

[bits 32]
p_mode_start:
  mov ax, SELECTOR_DATA
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov esp, LOADER_STACK_TOP
  mov ax, SELECTOR_VIDEO
  mov gs, ax
  mov byte [gs:160], "P"

  jmp $