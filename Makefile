
CFILES=$(shell ls *.c)
O32FILES=$(CFILES:.c=.o32)
O64FILES=$(CFILES:.c=.o64)

CFLAGS=-c -O
CFLAGS+=-Wall -Wno-main -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I .

C32FLAGS=$(CFLAGS)
C64FLAGS=$(CFLAGS) -ffreestanding -mcmodel=large -nostdlib -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow

CC32=gcc -m32
CC64=gcc -m64

default : kernel32.bin

debug :
	@echo CFILES: $(CFILES)
	@echo O32FILES: $(O32FILES)
	@echo O64FILES: $(O64FILES)
	@echo CFLAGS: $(CFLAGS)

start32.o : start32.asm
	nasm -f aout -o $@ $^

start64.o : start64.asm
	nasm -f aout -o $@ $^

$(O32FILES) : %.o32 : %.c
	$(CC32) $(C32FLAGS) -o $@ $^

$(O64FILES) : %.o64 : %.c
	$(CC32) $(C32FLAGS) -o $@ $^

kernel32.bin : link32.ld start32.o $(O32FILES)
	ld -T link32.ld -m i386linux -o $@ start32.o $(O32FILES)

kernel64.bin : link64.ld start64.o $(O64FILES)
	ld -T link64.ld -m i386linux -o $@ start64.o $(O64FILES)

q32 : kernel32.bin
	qemu -kernel kernel32.bin

q64 : kernel64.bin
	qemu-system-x86_64 -kernel kernel64.bin

clean :
	rm *.o *.o32 *.o64 *.bin

.PHONY : default debug qemu clean
