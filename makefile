
# 启动系统
start: all
	bochs -f bochsrc.disk -q  

all: hd60M.img mbr.bin loader.bin kernel.bin

# 创建MBR引导文件
mbr.bin: boot/boot.inc boot/mbr.S 
	nasm -I boot/ -o mbr.bin boot/mbr.S
	dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc
	ls -l mbr.bin

# 创建加载器
loader.bin: boot/boot.inc boot/loader.S
	nasm -I boot/ -o loader.bin boot/loader.S
	dd if=loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc
	ls -l loader.bin

kernel.bin: main.o print.o
	ld -Ttext 0xc0001500 -e main -o kernel.bin -m elf_i386 main.o print.o
	dd if=kernel.bin of=hd60M.img bs=512 count=200 seek=9 conv=notrunc
	ls -l kernel.bin

main.o: kernel/main.c
	gcc -c -m32 -o main.o kernel/main.c

print.o: lib/kernel/print.S
	nasm -f elf -I boot/ -o print.o lib/kernel/print.S


# 创建一个60M大小的扇区大小为512字节的平坦模式的硬盘文件
hd60M.img:
	bximage -q -func="create" -hd=60 -imgmode="flat" -sectsize=512 hd60M.img 


clear:
	rm -f hd60M.img
	rm -f *.bin
	rm -f *.o
	