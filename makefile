
WARNFLG = -Wall -Wextra
DEBUG = -g
STD = -std=c11
CFLAGS += $(DEBUG) $(WARNFLG) $(STD) -m32 -fno-stack-protector
CC = gcc
DDFLAGS = of=hd60M.img bs=512 conv=notrunc

# 启动系统
start: all
	bochs -f bochsrc.disk -q  

all: hd60M.img mbr.bin loader.bin kernel.bin

# 创建MBR引导文件
mbr.bin:  boot/mbr.S boot/boot.inc
	nasm -I boot/ -o $@ $<
	dd $(DDFLAGS) if=$@ count=1

# 创建加载器
loader.bin: boot/loader.S boot/boot.inc 
	nasm -I boot/ -o $@ $<
	dd $(DDFLAGS)  if=$@ count=4 seek=2

# 创建内核
kernel.bin: main.o kernel.o interrupt.o print.o print2.o
	ld -Ttext 0xc0001500 -e main -o $@ -m elf_i386 $^
	dd $(DDFLAGS)  if=$@ count=200 seek=9

interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) -c -o $@ $^

kernel.o: kernel/kernel.S
	nasm -f elf -I boot/ -o $@ $^

main.o: kernel/main.c	
	$(CC) $(CFLAGS) -c -o $@ $^

print.o: lib/kernel/print.S
	nasm -f elf -I boot/ -o $@ $^

print2.o: lib/kernel/print2.c
	$(CC) $(CFLAGS) -c -o $@ $^


# 创建一个60M大小的扇区大小为512字节的平坦模式的硬盘文件
hd60M.img:
	bximage -q -func="create" -hd=60 -imgmode="flat" -sectsize=512 hd60M.img 


clear:
	rm -f hd60M.img
	rm -f *.bin
	rm -f *.o
	