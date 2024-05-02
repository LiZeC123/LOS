
WARNFLG = -Wall -Wextra
# 使用 -D NODEBUG 可以关闭ASSERT
DEBUG = -g
STD = -std=c11
LIB = -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ -I thread/ -I userprog/ -I fs/ -I shell/
CFLAGS += $(DEBUG) $(WARNFLG) $(LIB) $(STD) -m32 -fno-stack-protector -fno-builtin
CC = gcc
DDFLAGS = of=hd60M.img bs=512 conv=notrunc

# 启动系统
run: all
	bochs -f bochsrc.disk -q  

all: hd60M.img hd80M.img mbr.bin loader.bin kernel.bin

rebuild: clear all

# 创建MBR引导文件
mbr.bin:  boot/mbr.S boot/boot.inc
	nasm -I boot/ -o $@ $<
	dd $(DDFLAGS) if=$@ count=1

# 创建加载器
loader.bin: boot/loader.S boot/boot.inc 
	nasm -I boot/ -o $@ $<
	dd $(DDFLAGS)  if=$@ count=4 seek=2

# 创建内核
kernel.bin: console.o ide.o ioqueue.o keyboard.o time.o \
			dir.o file.o fs.o inode.o superblock.o	\
			interrupt.o kernel.o main.o losmemory.o syscall-init.o \
			bitmap.o list.o print2.o print.o \
			debug.o loscall.o \
			stdio.o string.o \
			switch.o sync.o thread.o \
			process.o  tss.o     
	ld -Ttext 0xc0001500 -e main -o $@ -m elf_i386 $^
	dd $(DDFLAGS)  if=$@ count=200 seek=9

console.o: device/console.c	
	$(CC) $(CFLAGS) -c -o $@ $^

ide.o: device/ide.c	
	$(CC) $(CFLAGS) -c -o $@ $^

ioqueue.o: device/ioqueue.c
	$(CC) $(CFLAGS) -c -o $@ $^

keyboard.o: device/keyboard.c
	$(CC) $(CFLAGS) -c -o $@ $^

time.o: device/time.c	
	$(CC) $(CFLAGS) -c -o $@ $^

dir.o: fs/dir.c
	$(CC) $(CFLAGS) -c -o $@ $^

file.o: fs/file.c
	$(CC) $(CFLAGS) -c -o $@ $^

fs.o: fs/fs.c
	$(CC) $(CFLAGS) -c -o $@ $^

inode.o: fs/inode.c
	$(CC) $(CFLAGS) -c -o $@ $^

superblock.o: fs/superblock.c
	$(CC) $(CFLAGS) -c -o $@ $^

interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) -c -o $@ $^

kernel.o: kernel/kernel.S
	nasm -f elf -I boot/ -o $@ $^

main.o: kernel/main.c	
	$(CC) $(CFLAGS) -c -o $@ $^

losmemory.o: kernel/losmemory.c	
	$(CC) $(CFLAGS) -c -o $@ $^

syscall-init.o: kernel/syscall-init.c	
	$(CC) $(CFLAGS) -c -o $@ $^

bitmap.o: lib/kernel/bitmap.c	
	$(CC) $(CFLAGS) -c -o $@ $^

list.o: lib/kernel/list.c
	$(CC) $(CFLAGS) -c -o $@ $^

print.o: lib/kernel/print.S
	nasm -f elf -I boot/ -o $@ $^

print2.o: lib/kernel/print2.c
	$(CC) $(CFLAGS) -c -o $@ $^

debug.o: lib/user/debug.c	
	$(CC) $(CFLAGS) -c -o $@ $^

loscall.o: lib/user/loscall.c	
	$(CC) $(CFLAGS) -c -o $@ $^

stdio.o: lib/stdio.c
	$(CC) $(CFLAGS) -c -o $@ $^

string.o: lib/string.c	
	$(CC) $(CFLAGS) -c -o $@ $^

switch.o : thread/switch.S
	nasm -f elf -o $@ $^

sync.o: thread/sync.c	
	$(CC) $(CFLAGS) -c -o $@ $^

thread.o: thread/thread.c	
	$(CC) $(CFLAGS) -c -o $@ $^

process.o: userprog/process.c	
	$(CC) $(CFLAGS) -c -o $@ $^

tss.o: userprog/tss.c	
	$(CC) $(CFLAGS) -c -o $@ $^


# 创建一个60M大小的扇区大小为512字节的平坦模式的硬盘文件
hd60M.img:
	bximage -q -func="create" -hd=60 -imgmode="flat" -sectsize=512 hd60M.img 

hd80M.img:
	bximage -q -func="create" -hd=80 -imgmode="flat" -sectsize=512 hd80M.img 

clear:
	rm -f *.bin
	rm -f *.o
	