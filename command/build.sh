#! /bin/bash

BIN="prog_no_arg"
CFLAGS="-Wall -W -Wstrict-prototypes -Wmissing-prototypes -Wsystem-headers -c -m32 -fno-stack-protector -fno-builtin -std=c11 -I ../lib"

dd of=../hd60M.img bs=512 conv=notrunc if=./$BIN seek=300