
# 启动系统
start:
	bochs -f bochsrc.disk -q  

# 创建MBR引导文件
mbr: 
	nasm -I boot/ -o mbr.bin boot/mbr.S
	dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc
	ls -lh mbr.bin

# 创建加载器
loader:
	nasm -I boot/ -o loader.bin boot/loader.S
	dd if=loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc
	ls -lh loader.bin

kernel:
	gcc -c -o main.o kernel/main.c
	ld main.o -Ttext 0xc0001500 -e main -o kernel.bin

# 创建一个60M大小的扇区大小为512字节的平坦模式的硬盘文件
disk:
	bximage -q -func="create" -hd=60 -imgmode="flat" -sectsize=512 hd60M.img 


clear:
	rm -rf hd60M.img mbr.bin loader.bin
	