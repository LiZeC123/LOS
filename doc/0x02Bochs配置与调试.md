Bochs配置与调试
================


启动Bochs
-----------

可使用如下的指令启动Bochs

```
bochs -f bochsrc.disk -q  
```

其中`-f`参数指定了bochs的配置文件, `-q`参数指定了快速启动, 可跳过启动前的配置询问.

配置文件
----------

本项目使用的Bochs配置文件如下

```
# memory: guest=32, host=32

romimage: file=$BXSHARE/BIOS-bochs-latest, options=fastboot
vgaromimage: file=$BXSHARE/VGABIOS-lgpl-latest


boot: disk

log: bochsout.txt

mouse: enabled=0

# 配置硬盘
ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata0-master: type=disk, path="hd60M.img", mode=flat
```

其中指定了硬件的基本参数和硬盘文件的位置. 


帮助系统
---------

在Bochs启动后, 可输入`help`指令查看帮助, 例如

```
<bochs:1> help
h|help - show list of debugger commands
h|help command - show short command description
-*- Debugger control -*-
    help, q|quit|exit, set, instrument, show, trace, trace-reg,
    trace-mem, u|disasm, ldsym, slist
-*- Execution control -*-
    c|cont|continue, s|step, p|n|next, modebp, vmexitbp
-*- Breakpoint management -*-
    vb|vbreak, lb|lbreak, pb|pbreak|b|break, sb, sba, blist,
    bpe, bpd, d|del|delete, watch, unwatch
-*- CPU and memory contents -*-
    x, xp, setpmem, writemem, crc, info,
    r|reg|regs|registers, fp|fpu, mmx, sse, sreg, dreg, creg,
    page, set, ptime, print-stack, ?|calc
-*- Working with bochs param tree -*-
    show "param", restore
```

使用`help`和具体的指令, 可查看该指令的用法, 例如

```
<bochs:2> help x
x  /nuf <addr> - examine memory at linear address
xp /nuf <addr> - examine memory at physical address
    nuf is a sequence of numbers (how much values to display)
    and one or more of the [mxduotcsibhwg] format specificators:
    x,d,u,o,t,c,s,i select the format of the output (they stand for
        hex, decimal, unsigned, octal, binary, char, asciiz, instr)
    b,h,w,g select the size of a data element (for byte, half-word,
        word and giant word)
    m selects an alternative output format (memory dump)
```


设置断点
-----------

### 内存断点

使用`lb`或者`pb`对指定的内存位置设置断点, 例如

```
pb 0x7c00
```
可在MBR起始位置设置断点, 查看MRB执行情况.

断点命中后, 可使用`s`或者`n`执行step或者next操作. 使用`s`会进入子程序或者中断, 而使用`n`会直接执行整个程序.

### 时间断点

使用`sba`可设置时钟断点, 使得CPU在执行第N条指令之前中断. 例如如下指令将会在CPU执行第5243747条指令之前中断.

```
sba 5243747
```

使用`sb`可设置增量执行n条执行后中断, 例如如下指令表示再执行10条指令后中断

```
sb 10
```

-------------

通常Bochs会显示下一条执行的t值, 例如

```
Next at t=5243747
(0) [0x000000007c00] 0000:7c00 (unk. ctxt): mov ax, cs                ; 8cc8
```

直接使用此时输出的值, 即可在执行该指令前设置断点, 无需再手动减一.

### IO断点

使用`watch`指令设置IO断点, 对应的内存地址有读写操作时触发中断, 例如

```
watch r 0xabc
watch w 0xdef
```

### 其他断点指令

指令    | 效果
-------|----------------------
blist   | 显示所有断点信息
bpd     | 禁用指定编号的断点
bpe     | 启用指定编号的断点
d       | 删除指定编号的断点
watch   | 显示所有读写断点
unwatch | 清除所有读写断点


查看内存内容
-----------

使用`x`指令查看线性地址(虚拟地址), 使用`xp`查看物理地址. 其格式如下

```
xp /nuf <addr>
```

`n`表示要查看多少个单位的内存, `u`表示每个单位展示的格式, . 
`u`表示每个单位展示的格式, 取值为: x(hex), d(decimal), u(unsigned), o(octal), b(binary), c(char), s(asciiz), i(instr)
`f`表示每个单位的数据长度, 取值为: b(byte), h(half-word), w(word), g(giant word)

基于以上选项, 可得到一些常见的用法

----------------------

查看指定位置的内存

```
xp
```


启用提示信息
----------------

指令      | 效果
----------|-------------------------------------------------------
show mode | CPU发生模式切换时(例如从实模式进入保护模式)打印提示信息
show int  | 发生中断时打印提示信息, 具体可选softint, extint和iret

显示寄存器信息
------------------

指令      | 效果
----------|-------------------------------------------------------
r         | 显示寄存器信息
sreg      | 显示段寄存器信息


显示状态信息
-------------

使用`info`系列指令, 可查看各类重要数据结构的信息, 具体如下


指令        | 效果
------------|-------------
info pb     | 显示所有断点信息
info cpu    | 显示CPU的所有状态信息
info idt    | 显示中断向量表IDT
info gdt    | 显示全局描述符表GDT
info ldt    | 显示局部描述符表LDT
info tss    | 显示任务状态段TSS


内存映射
--------------

指令           | 效果
--------------|----------------------------
info tab      | 显示页表信息
page <addr>   | 显示线性地址对应的物理地址


参考资料
------------

更多Bochs的调试指令, 可参考如下的文章

- [Bochs调试常用命令|Ney的技术博客](https://petpwiuta.github.io/2020/05/09/Bochs%E8%B0%83%E8%AF%95%E5%B8%B8%E7%94%A8%E5%91%BD%E4%BB%A4/)