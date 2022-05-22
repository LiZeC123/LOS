;  --------------------  加载器基本配置  -------------------- 
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2

;  --------------------  GDT描述符属性  -------------------- 
DESC_G_4K equ (0x1 << 23) ; G位设置为4K粒度
DESC_D_32 equ (0x1 << 22) ; 设置为32位模式
DESC_L    equ (0x0 << 21) ; 不启用64位模式
DESC_AVL  equ (0x0 << 20) ; 保留字段置为0

DESC_LIMIT_CODE2  equ (0xF << 16)    ; 代码段限长的高位, 默认使用最大值,因此设置为0xF
DESC_LIMIT_DATA2  equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2 equ (0x0 << 16)    ; 显存段只有32KB, 因此最高位为0x0

DESC_P equ (0x1 << 15)      ; 指示段存在于内存之中
DESC_DLP_0 equ (00b << 13)   ; 四种特权等级
DESC_DLP_1 equ (01b << 13)
DESC_DLP_2 equ (10b << 13)
DESC_DLP_3 equ (11b << 13)

DESC_S_CODE equ (0x1 << 12) ; 代码段是非系统段
DESC_S_DATA equ DESC_S_CODE ; 数据段是非系统段
DESC_S_sys  equ (0x0 << 12)

DESC_TYPE_CODE equ (1000b << 8) ; 代码段可执行，非一致，不可读，访问位清空
DESC_TYPE_DATA equ (0010b << 8) ; 数据段不可执行，向上扩展，可写，访问位清空

DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
                    DESC_P + DESC_DLP_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
                    DESC_P + DESC_DLP_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + \
                    DESC_P + DESC_DLP_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b

; -------------------- 选择子属性 -------------------- 
RPL0 equ 00b    ; 最低两位表示四种特权等级
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b

TI_GDT equ 000b ; 第三位表示数据存储位置是GDT还是LDT
TI_LDT equ 100b


; -------------------- 页表相关属性 -------------------- 
PAGE_DIR_TABLE_POS equ 0x10_0000  ; 页表加载到内存的1M开始的位置

PG_P equ 1b         ; 指示页面是否存在
PG_RW_R equ 00b     ; 指示页面读写权限
PG_RW_W equ 10b
PG_US_S equ 000b    ; 指示页面是用户级还是系统级
PG_US_U equ 100b


; -------------------- 内核相关属性 -------------------- 
KERNEL_START_SECTOR     equ 0x09        ; 内核存储在硬盘的第9个扇区
KERNEL_BIN_BASE_ADDR    equ 0x7_0000    ; 从这个位置开始, 大约有190KB的可用空间
KERNEL_ENTRT_POINT      equ 0xc000_1500 ; 内核代码的起始位置, 链接内核的时候需要设置为该值

PT_NULL equ 0x0 ; 该程序头项为空项目
PT_LOAD equ 0x1 ; 该程序头项为可加载项
