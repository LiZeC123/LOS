#! /bin/bash

file=${1%.*}  # 去除文件后缀
lsc -C  "$file.c"  -o "$file.o"
lsca start.s
lscl "start.o" "crt/loscall.o"  "crt/stdio.o" "crt/string.o" "$file.o" 
mv z.out $file
dd of=../hd60M.img bs=512 conv=notrunc if=./$file seek=300