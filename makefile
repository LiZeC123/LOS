
# 启动系统
start:
	bochs -f bochsrc.disk -q  

# 创建MBR引导文件
mbr: disk
	nasm -o mbr.bin boot/mbr.S
	dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc

# 创建一个60M大小的扇区大小为512字节的平坦模式的硬盘文件
disk:
	bximage -q -func="create" -hd=60 -imgmode="flat" -sectsize=512 hd60M.img 


clear:
	rm -rf hd60M.img mbr.bin
	