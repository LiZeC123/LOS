
WARNFLG = -Wall -Wextra
# 使用 -D NODEBUG 可以关闭ASSERT
DEBUG = -g
STD = -std=c11
LIB = -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ -I thread/ -I userprog/ -I fs/ -I shell/
CFLAGS += $(DEBUG) $(WARNFLG) $(LIB) $(STD) -m32 -fno-stack-protector -fno-builtin
CC = gcc
DDFLAGS = of=hd60M.img bs=512 conv=notrunc

# 启动系统
start: all
	bochs -f bochsrc.disk -q  

all: hd60M.img hd80M.img mbr.bin loader.bin kernel.bin

# 创建MBR引导文件
mbr.bin:  boot/mbr.S boot/boot.inc
	nasm -I boot/ -o $@ $<
	dd $(DDFLAGS) if=$@ count=1

# 创建加载器
loader.bin: boot/loader.S boot/boot.inc 
	nasm -I boot/ -o $@ $<
	dd $(DDFLAGS)  if=$@ count=4 seek=2

# 创建内核
kernel.bin: main.o kernel.o stdio.o ide.o console.o sync.o keyboard.o interrupt.o tss.o process.o stdio.o syscall-init.o syscall.o thread.o process.o time.o switch.o list.o memory.o bitmap.o ioqueue.o string.o debug.o print2.o print.o 
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

interrupt.o: kernel/interrupt.c
	$(CC) $(CFLAGS) -c -o $@ $^

kernel.o: kernel/kernel.S
	nasm -f elf -I boot/ -o $@ $^

main.o: kernel/main.c	
	$(CC) $(CFLAGS) -c -o $@ $^

memory.o: kernel/memory.c	
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

syscall.o: lib/user/syscall.c	
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
	