%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
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
  ; 一共定义64个表项，每个占8字节，共消耗2x256 = 0x200 字节
  ; 因此这里的位置是 LOADER_BASE_ADDR + 0x200
  total_men_bytes dd 0

  ; 定义对应的选择子, 代码段的索引是1, 其他段同理
  SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
  SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0
  SELECTOR_VIDEO equ (0x0003 << 3) + TI_GDT + RPL0

  ; GDT指针, 与GDTR寄存器的内容一致, 第2字节为LIMIT值, 高4字节为地址值
  gdt_ptr dw GDT_LIMIT
          dd GDT_BASE
  
  ; buf和nr占据剩余的空间，使得loader_start对齐在一个整数位置
  ards_buf times (0x300 -($-$$) - 2) db 0
  ; 最后两字节表示实际的结构体数量
  ards_nr dw 0

; LOADER_BASE_ADDR + 0x300
loader_start:
  mov sp, LOADER_BASE_ADDR

  xor ebx, ebx
  mov edx, 0x534d4150
  mov di, ards_buf
.e820_mem_get_loop:
  mov eax, 0x0000e820
  mov ecx, 20
  int 0x15
  jc .e820_failed_so_try_e801
  add di, cx
  inc word [ards_nr]
  cmp ebx, 0
  jnz .e820_mem_get_loop

  mov cx, [ards_nr]
  mov ebx, ards_buf
  xor edx, edx
.find_max_mem_area:
  mov eax, [ebx]
  add eax, [ebx+8]
  add ebx, 20
  cmp edx, eax
  jge .next_ards
  mov edx, eax
.next_ards:
  loop .find_max_mem_area
  jmp .mem_get_ok

.e820_failed_so_try_e801:
  mov ax, 0xe801
  int 0x15
  jc .e801_failed_so_try_88

  mov cx, 0x400
  mul cx
  shl edx, 16
  and eax, 0x0000_FFFF
  or edx, eax
  add edx, 0x10_0000
  mov esi, edx

  xor eax, eax
  mov ax, bx
  mov ecx, 0x1_0000
  mul ecx

  add esi, eax
  mov edx, esi
  jmp .mem_get_ok

.e801_failed_so_try_88:
  mov ah, 0x88
  int 0x15
  jc .error_hlt
  add eax, 0x0000_FFFF

  mov cx, 0x400
  mul cx
  shl edx, 16
  or edx, eax
  add edx, 0x10_0000
  jmp .mem_get_ok

.error_hlt:
  jmp $

.mem_get_ok:
  mov [total_men_bytes], edx

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

  ; setup_page
setup_page:
  mov ecx, 4096
  mov esi, 0
.clear_page_dir:
  mov byte [PAGE_DIR_TABLE_POS + esi], 0
  inc esi
  loop .clear_page_dir
.create_pde:
  mov eax, PAGE_DIR_TABLE_POS
  add eax, 0x1000
  mov ebx, eax
  





  jmp $