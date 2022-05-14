

%include "boot.inc"
; Main
; ----------------------
; vstart指令要求编译器将此处的地址假定为0x7c00，后续计算位置时都以此为基准
SECTION MBR vstart=0x7c00
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax
  mov sp, 0x7c00
  mov ax, 0xb800  ; 显示器映射内存的位置
  mov gs, ax


; 调用BIOS中断向量实现清屏
  mov ax, 0x600
  mov bx, 0x700
  mov cx, 0
  mov dx, 0x184f
  int 0x10

  mov byte [gs:0x00], 'L'
  mov byte [gs:0x01], 0xA4

  mov byte [gs:0x02], 'O'
  mov byte [gs:0x03], 0xA4

  mov byte [gs:0x04], 'S'
  mov byte [gs:0x05], 0xA4

  mov byte [gs:0x06], '!'
  mov byte [gs:0x07], 0xA4

  mov eax, LOADER_START_SECTOR  ; 加载器在硬盘上的逻辑地址
  mov bx, LOADER_BASE_ADDR      ; 加载器在内存中的地址
  mov cx, 4                     ; 读取4个扇区
  call rd_disk_m_16

  jmp LOADER_BASE_ADDR          ; 跳转到loader

; ---------------------------------------------
; 函数：从硬盘读取n个扇区的内容到制定的内存位置
; eax: LAB扇区号
; bx : 读取的数据写入到内存的位置
; cx : 读取的扇区数
rd_disk_m_16:
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
  mov [bx], ax
  add bx, 2
  loop .go_on_read
  ret

  
  ; $是当前地址， $$是这一段的开始地址, 因此($-$$)就是上面的代码已经占据的字节数
  ; 因此这行代码就是用0填充剩余的空间，使得整个程序最终占据512字节 
  times 510-($-$$) db 0 
  db 0x55, 0xaa
