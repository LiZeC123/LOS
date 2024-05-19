使用LSC编译系统编译用户程序
=============================


在使用GCC进行编译时, 可能由于参数设置问题, 编译的程序在执行时会产生缺页中断, 导致用户程序无法执行. 由于开发机通常是64位系统, 因此交叉编译比较麻烦, 容易出现问题.

此时可以使用我之前开发的类C语言编辑系统LSC对用户程序进行编译. LSC仅可编译出32位可执行程序, 且其中不包含任何多余的指令, 因此非常适合在此场景中使用.

![系统执行示例](doc/images/执行用户进程.PNG)


系统调用兼容性问题
-----------------------

程序运行过程中可能会使用一些系统, LSC系统的基础输入输出依赖操作系统提供的系统调用. LSC系统使用的是Linux系统在32位的系统调用, 而LOS系统在构建系统调用时特意参考了Linux的规范并尽可能保持一致, 因此使用LSC语言开发的程序可以无缝的在LOS中运行.


编译脚本
------------

在当前系统安装后LSC编译组件后, 可参考如下脚本编译用户程序


```sh
#! /bin/bash

file=${1%.*}  # 去除文件后缀
lsc -C  "$file.c"  -o "$file.o"
lsca start.s
lscl "start.o" "crt/loscall.o"  "crt/stdio.o" "crt/string.o" "$file.o" 
mv z.out $file
dd of=../hd60M.img bs=512 conv=notrunc if=./$file seek=300
```

其中依赖一些LOS组件的文件, 此类文件可通过Makefile中的如下部分编译生成

```makefile
# 编译运行时静态库, 使得第三方程序可使用相关的系统调用
crt: DEBUG += -D NODEBUG
crt: CFLAGS += -no-pie -fno-pic
crt: clear crt.a

crt.a: stdio.o loscall.o string.o start.o
	cp stdio.o   ./command/crt/
	cp loscall.o ./command/crt/
	cp string.o  ./command/crt/
	cp start.o   ./command/crt/
```

以上编译参数需要注意
1. 关闭DEBUG指令, 避免因为ASSERT相关的代码引入不必要的依赖
2. 使用`-no-pie`和`-fno-pic`去掉一些GCC生成的无关指令