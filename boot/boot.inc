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






