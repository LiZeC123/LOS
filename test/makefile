WARNFLG = -Wall -Wextra
# 使用 -D NODEBUG 可以关闭ASSERT
DEBUG = -g
STD = -std=c11 -D NODEBUG -D TEST
CFLAGS += $(DEBUG) $(WARNFLG) $(STD) -m32 -fno-stack-protector -fno-builtin
CC = gcc
DDFLAGS = of=hd60M.img bs=512 conv=notrunc

all: bitmap string

bitmap:
	$(CC) $(CFLAGS) -o $@.out test_bitmap.c ../kernel/bitmap.c 
	./$@.out

string:
	$(CC) $(CFLAGS) -o $@.out test_string.c ../kernel/string.c 
	./$@.out

clear:
	rm *.out