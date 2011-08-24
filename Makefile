
default : kernel.bin

start.o : start.asm
	nasm -f aout -o start.o start.asm

main.o : main.c system.h
	gcc -m32 -Wall -Wno-main -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I . -c -o main.o main.c

scrn.o : scrn.c system.h
	gcc -m32 -Wall -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I . -c -o scrn.o scrn.c

kernel.bin : link.ld start.o main.o scrn.o
	ld -T link.ld -m i386linux -o kernel.bin start.o main.o scrn.o

qemu : kernel.bin
	qemu -kernel kernel.bin

.PHONY : default qemu
