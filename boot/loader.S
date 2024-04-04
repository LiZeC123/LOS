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

  ; GDT指针, 与GDTR寄存器的内容一致, 低2字节为LIMIT值, 高4字节为地址值
  gdt_ptr dw GDT_LIMIT
          dd GDT_BASE
  
  ; buf和nr占据剩余的空间，使得loader_start对齐在一个整数位置
  ards_buf times (0x300 -($-$$) - 2) db 0
  ; 最后两字节表示实际的结构体数量
  ards_nr dw 0

; LOADER_BASE_ADDR + 0x300 = 0xC00
loader_start:
  mov sp, LOADER_BASE_ADDR

  call check_mem

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

; ---------------------------------------------
; 过程：检查可用内存数量
; 可用内存的字节数写入total_men_bytes所在位置
check_mem:
; ---------------------------------------------
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
  mov edx, -1
  mov [total_men_bytes], edx
  ret

.mem_get_ok:
  mov [total_men_bytes], edx
  ret

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

  ; 加载内核到指定位置
  mov eax, KERNEL_START_SECTOR
  mov ebx, KERNEL_BIN_BASE_ADDR
  mov ecx, 200
  call rd_disk_m_32

  call setup_page

  sgdt [gdt_ptr]

  mov ebx, [gdt_ptr + 2]
  or dword [ebx + 0x18 + 4], 0xc000_0000

  add dword [gdt_ptr + 2], 0xc000_0000
  add esp, 0xc000_0000
; 设置分页目录地址寄存器
  mov eax, PAGE_DIR_TABLE_POS
  mov cr3, eax
; 启用分页机制
  mov eax, cr0
  or eax, 0x8000_0000
  mov cr0, eax
; 重新加载GDT表
  lgdt [gdt_ptr]

  mov byte [gs:162], "V"

  ; 进入内核代码
  jmp SELECTOR_CODE:enter_kernel  ;强制刷新流水线
enter_kernel:
  call kernel_init
  mov esp, 0xc009_f000
  mov eax, [KERNEL_BIN_BASE_ADDR + 24]  ; 此处是e_entry, 表示入口函数的虚拟地址
  jmp eax


; ---------------------------------------------
; 函数：从硬盘读取n个扇区的内容到制定的内存位置
; eax: LAB扇区号
; bx : 读取的数据写入到内存的位置
; cx : 读取的扇区数
rd_disk_m_32:
; ---------------------------------------------
  mov esi, eax
  mov di, cx  ; di si 是数据相关的寄存器
; Step1: 设置要读取的扇区数
  mov dx, 0x1f2
  mov al, cl
  out dx, al
  mov eax, esi

; Step 2: 将LBA地址存入寄存器
  mov dx, 0x1f3
  out dx, al

  mov cl, 8
  shr eax, cl
  mov dx, 0x1f4
  out dx, al

  shr eax, cl
  mov dx, 0x1f5
  out dx, al

  shr eax, cl
  and al, 0x0f  ; 最后的4位地址放置到地位
  or al, 0xe0   ; 高位设置为1110，表示lab模式
  mov dx, 0x1f6
  out dx, al

; Step 3: 写入读指令
  mov dx, 0x1f7
  mov al, 0x20
  out dx, al

; Step 4: 检查硬盘状态
.not_ready:
  nop
  in al, dx     ; 状态寄存器和指令寄存器是同一个端口
  and al, 0x88  ; 获取第3位和第7位的状态
  cmp al, 0x08  ; 第三位为1表示数据已经准备好， 第7位为1表示硬盘正忙
  jnz .not_ready

; Step 5: 从端口读取数据
  mov ax, di    ; 一开始就已经对di赋值，其中是需要读取的扇区数量
  mov dx, 256   ; 每次可读取2字节，每个扇区512字节
  mul dx        ; 因此需要读取 di×512/2 = di×256字节
  mov cx, ax
  mov dx, 0x1f0
.go_on_read:
  in ax, dx
  mov [ebx], ax     ; 为了加载到超过0xffff的位置, 这里需要使用ebx
  add ebx, 2
  loop .go_on_read
  ret


; ---------------------------------------------
; 过程：创建页目录和页表
setup_page:
; ---------------------------------------------
; 清空页目录所在内存空间
  mov ecx, 4096
  mov esi, 0
.clear_page_dir:
  mov byte [PAGE_DIR_TABLE_POS + esi], 0
  inc esi
  loop .clear_page_dir
; 创建页目录
; 页目录对应地址中的10bit, 因此需要0x400项, 其中 0x0~0x2ff 项分配给用户, 0x300~0x3ff 项分配给内核
; 整个虚拟地址对应4GB空间, 因此用户使用低3GB空间, 内核使用高1GB空间
; 页目录每一项占据4字节, 页目录表正好使用0x400 * 4 = 0x1000(4K)字节的空间
.create_pde:
  mov eax, PAGE_DIR_TABLE_POS

  add eax, 0x1000   ; 移动到页目录表末尾
  mov ebx, eax      ; eax和ebx现在指向页目录后的空闲空间, 即第一个页表项的起始位置
  
  or eax, PG_US_U | PG_RW_W | PG_P          ; 给eax附加页面的属性
  ; 将用户空间的第一项和内核空间的第一项指向同一个页表, 稍后设置该页表的内容
  mov [PAGE_DIR_TABLE_POS + 4*0x000], eax  
  mov [PAGE_DIR_TABLE_POS + 4*0x300], eax

  ; 页目录最后一项规定指向自己
  sub eax, 0x1000             ; eax重新指向页目录的真实物理位置
  mov [PAGE_DIR_TABLE_POS + 4*0x3ff], eax

; 创建页表项, 一个页表也对应10bit地址, 因此也有0x400项, 每一项需要4字节, 整个页表同样占据0x1000字节空间
; 首先映射真实物理内存中低区1M空间
  mov ecx, 0x100 ; 每一页大小为4KB, 因此1M空间需要分配 1M(0x10_0000) / 4K(0x1000) = 0x100 项
  mov esi, 0     ; 页表项索引
  mov edx, PG_US_U | PG_RW_W | PG_P ; 真实物理地址从0开始
.create_pte:
  mov [ebx+esi*4], edx              ; ebx在上一步指向了页目录的末尾, 也即页表的起始位置
  add edx, 0x1000                   ; 实际的物理地址值, 每次增加4K
  inc esi
  loop .create_pte

; 给内核的虚拟地址分配页表空间
; 内核占据高1GB的空间, 为了便于后续的共享, 对这些空间提前分配页表
  mov eax, PAGE_DIR_TABLE_POS
  add eax, 0x2000                     ; 页目录占据0x1000字节, 低位的1M空间的页表占据0x1000字节, 此后是可用空间
  or eax, PG_US_U | PG_RW_W | PG_P
  mov ebx, PAGE_DIR_TABLE_POS
  mov ecx, 0x100 - 2  ; 虚拟地址的第一页和最后一页已经做了特殊处理, 所以跳过
  mov esi, 0x301      ; 从第301项开始映射, 直到最后一项之前
.create_kernel_pde:
  mov [ebx+esi*4], eax
  inc esi 
  add eax, 0x1000     ; 每个页表需要0x1000字节的空间 
  loop .create_kernel_pde
  ret


; ---------------------------------------------
; 过程：解析内核文件
; ---------------------------------------------
kernel_init:
  xor eax, eax
  xor ebx, ebx  ; 执行当前解析的程序头项的位置
  xor ecx, ecx  ; 记录程序头项的数量
  xor edx, edx  ; 记录每个程序头项的尺寸

  mov dx, [KERNEL_BIN_BASE_ADDR + 42]   ; 此处是e_phentsize, 表示程序头项的大小
  mov ebx, [KERNEL_BIN_BASE_ADDR + 28]  ; 此处是e_phoff, 表示程序头项在文件中的偏移值, 理论上应为0x34
  add ebx, KERNEL_BIN_BASE_ADDR
  mov cx, [KERNEL_BIN_BASE_ADDR + 44]   ; 此处是e_phnum, 表示有几个程序头项

.each_segment:
  cmp byte [ebx + 0], PT_LOAD     ; 如果不是可加载项, 则直接跳过
  jne .PTNULL

  cmp dword [ebx+8], 0xc000_0000  ; 如果需要加载的地址不在内核空间, 则跳过
  jb .PTNULL                      ; 这种情况是gcc额外添加的段

  ; 开始解析
  push dword [ebx + 16]           ; 此处为p_filesz, 表示该段的长度
  
  mov eax, [ebx+4]                ; 此处为p_offset, 表示该段在文件中的偏移值
  add eax, KERNEL_BIN_BASE_ADDR   ; 相加后即为该段的物理地址
  push eax

  push dword [ebx+8]              ; 此处为p_vaddr, 即希望该段加载的虚拟地址

  call mem_cpy                    ; 根据上述信息, 将段内的实际数据复制到指定位置
  add esp, 12
.PTNULL:
  add ebx, edx          ; ebx指向下一个程序头项
  loop .each_segment
  ret


; ---------------------------------------------
; 函数：逐一拷贝数据
; 参数: dst, src, size
mem_cpy:
; ---------------------------------------------
  cld
  push ebp
  mov ebp, esp
  push ecx

  mov edi, [ebp + 8]
  mov esi, [ebp + 12]
  mov ecx, [ebp + 16]
  rep movsb

  pop ecx
  pop ebp
  ret

