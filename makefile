
WARNFLG = -Wall -Wextra
# 使用 -D NODEBUG 可以关闭ASSERT
DEBUG = -g
STD = -std=c11
CFLAGS += $(DEBUG) $(WARNFLG) $(STD) -m32 -fno-stack-protector -fno-builtin
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
kernel.bin: main.o kernel.o console.o sync.o keyboard.o interrupt.o tss.o process.o stdio.o syscall-init.o syscall.o thread.o process.o time.o switch.o list.o memory.o bitmap.o ioqueue.o string.o debug.o print2.o print.o 
	ld -Ttext 0xc0001500 -e main -o $@ -m elf_i386 $^
	dd $(DDFLAGS)  if=$@ count=200 seek=9

bitmap.o: kernel/bitmap.c	
	$(CC) $(CFLAGS) -c -o $@ $^

console.o: kernel/console.c	
	$(CC) $(CFLAGS) -c -o $@ $^

debug.o: kernel/debug.c	
	$(CC) $(CFLAGS) -c -o $@ $^

interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) -c -o $@ $^

ioqueue.o: kernel/ioqueue.c
	$(CC) $(CFLAGS) -c -o $@ $^

kernel.o: kernel/kernel.S
	nasm -f elf -I boot/ -o $@ $^

keyboard.o: kernel/keyboard.c
	$(CC) $(CFLAGS) -c -o $@ $^

list.o: kernel/list.c
	$(CC) $(CFLAGS) -c -o $@ $^

main.o: kernel/main.c	
	$(CC) $(CFLAGS) -c -o $@ $^

memory.o: kernel/memory.c	
	$(CC) $(CFLAGS) -c -o $@ $^

process.o: kernel/process.c	
	$(CC) $(CFLAGS) -c -o $@ $^

stdio.o: kernel/stdio.c
	$(CC) $(CFLAGS) -c -o $@ $^

string.o: kernel/string.c	
	$(CC) $(CFLAGS) -c -o $@ $^

sync.o: kernel/sync.c	
	$(CC) $(CFLAGS) -c -o $@ $^

syscall-init.o: kernel/syscall-init.c	
	$(CC) $(CFLAGS) -c -o $@ $^

syscall.o: kernel/syscall.c	
	$(CC) $(CFLAGS) -c -o $@ $^

thread.o: kernel/thread.c	
	$(CC) $(CFLAGS) -c -o $@ $^

switch.o : kernel/switch.S
	nasm -f elf -o $@ $^

time.o: kernel/time.c	
	$(CC) $(CFLAGS) -c -o $@ $^

tss.o: kernel/tss.c	
	$(CC) $(CFLAGS) -c -o $@ $^

print.o: lib/kernel/print.S
	nasm -f elf -I boot/ -o $@ $^

print2.o: lib/kernel/print2.c
	$(CC) $(CFLAGS) -c -o $@ $^


# 创建一个60M大小的扇区大小为512字节的平坦模式的硬盘文件
hd60M.img:
	bximage -q -func="create" -hd=60 -imgmode="flat" -sectsize=512 hd60M.img 


clear:
	rm -f *.bin
	rm -f *.o
	