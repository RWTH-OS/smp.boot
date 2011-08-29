
CFILES=$(shell ls *.c)
OFILES=$(CFILES:.c=.o)

CFLAGS=-c -O
CFLAGS+=-Wall -Wno-main -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I .

CC32=gcc -m32

default : kernel32.bin

debug :
	@echo CFILES: $(CFILES)
	@echo OFILES: $(OFILES)
	@echo CFLAGS: $(CFLAGS)

start32.o : start32.asm
	nasm -f aout -o $@ $^

$(OFILES) : %.o : %.c
	$(CC32) $(CFLAGS) -o $@ $^

kernel32.bin : link32.ld start32.o $(OFILES)
	ld -T link32.ld -m i386linux -o $@ start32.o $(OFILES)

qemu : kernel32.bin
	qemu -kernel kernel32.bin


clean :
	rm *.o *.bin

.PHONY : default debug qemu clean
