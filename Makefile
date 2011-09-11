
CFILES=$(shell ls *.c | grep -v boot32.c)
O32FILES=$(CFILES:.c=.o32)
O64FILES=$(CFILES:.c=.o64)
HFILES=$(shell ls *.h)

CFLAGS=-c -O
CFLAGS+=-Wall -Wextra -Wno-main -O -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -fno-builtin -I .

C32FLAGS=$(CFLAGS)
C64FLAGS=$(CFLAGS) -ffreestanding -mcmodel=large -nostdlib -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow

CC32=gcc -m32
CC64=gcc -m64

# symbols from 64 bit kernel (kernel64.elf64) to be transferred to kernel64.bin
KERNEL64_SYMBOLS="Realm32 Realm64 main hw_info isr0 idt"

default : kernel32.bin kernel64.bin

debug :
	@echo CFILES: $(CFILES)
	@echo O32FILES: $(O32FILES)
	@echo O64FILES: $(O64FILES)
	@echo CFLAGS: $(CFLAGS)

# 32 bit start code for 32 bit kernel
start32.o : start32.asm start.asm
	@echo NASM $< '->' $@
	@nasm -f aout -o $@ $<

# 32 bit start code for 64 bit kernel
start64.o : start64.asm start.asm
	@echo NASM $< '->' $@
	@nasm -f elf32 -o $@ $<

# 64 bit jump code for 64 bit kernel
jump64.o : jump64.asm
	@echo NASM $< '->' $@
	@nasm -f elf64 -o $@ $<

# C files into 32 bit objects
$(O32FILES) : %.o32 : %.c
	@echo CC32 $< '->' $@
	@$(CC32) $(C32FLAGS) -o $@ $<

# C files into 64 bit objects
$(O64FILES) : %.o64 : %.c
	@echo CC64 $< '->' $@
	@$(CC64) $(C64FLAGS) -o $@ $<

boot32.o : boot32.c
	@echo CC32 $< '->' $@
	@$(CC32) $(C32FLAGS) -o $@ $<

# link 32 bit kernel (a.out-multiboot)
kernel32.bin : link32.ld start32.o boot32.o $(O32FILES)
	@echo LD $^ '->' $@
	@ld -T link32.ld -m i386linux -o $@ start32.o boot32.o $(O32FILES)

# link 64 bit kernel that will be embedded into kernel64.bin
kernel64.elf64 : link64.ld jump64.o $(O64FILES)
	@echo LD $^ '->' $@
	@ld -nostdlib -nodefaultlibs -T link64.ld  -o kernel64.elf64 jump64.o $(O64FILES)

# generate .KERNEL64 section's data from 64 bit kernel
kernel64.section : kernel64.elf64
	@echo GETSECTION $^ '->' $@
	@readelf -SW "kernel64.elf64" | python getsection.py 0x140000 kernel64.elf64 kernel64.section

# export wanted symbols from KERNEL64 from 64 bit kernel
kernel64.symbols : kernel64.elf64
	@echo GETSYMBOLS $^ '->' $@
	@readelf -sW "kernel64.elf64" | python getsymbols.py $(KERNEL64_SYMBOLS) kernel64.symbols

# add section .KERNEL64 with 64 bit code and data to 32 bit start code (will still be ELF32)
kernel64.o : start64.o kernel64.section
	@echo OBJCOPY $^ '->' $@
	@objcopy --add-section .KERNEL64="kernel64.section" --set-section-flag .KERNEL64=alloc,data,load,contents start64.o kernel64.o

# finally link 32 bit start code with implanted .KERNEL64 (opaque blob)
# to a relocated ELF32-multiboot kernel image
kernel64.bin : link_start64.ld kernel64.symbols kernel64.o boot32.o scrn.o32 lib.o32
	@echo LD $^ '->' $@
	@ld -melf_i386 -T link_start64.ld -T kernel64.symbols   kernel64.o boot32.o scrn.o32 lib.o32 -o kernel64.bin 

depend : .depend
.depend : $(CFILES) boot32.c 
	@echo DEPEND $^
	@$(CC) -MM $^ | sed "s+\(.*\):\(.*\)+\1 \132 \164 :\2+" > $@

# The sed regex prints every line three time: the original (xxx.o : xxx.c yyy.h...)
# and two versions for 32 and 64 bit objects: (xxx.o32 : xxx.c yyy.h...) and (xxx.o64 : ...).
# The part right of the colon is not changed.
# The regex catches the parts left and right of the ':' in '\(.*\)' (with arbitrary characters)
# and the catched expressions are issued by \1 and \2 (each tree times, separated by newlin \n)

-include .depend


# start QEMU with 32 or 64 bit
q32 : kernel32.bin
	qemu -kernel kernel32.bin

q64 : kernel64.bin
	qemu-system-x86_64 -kernel kernel64.bin

smp : s64
s32 : kernel32.bin
	qemu-system-i386 -smp 2 -kernel kernel32.bin

s64 : kernel64.bin
	qemu-system-x86_64 -smp 2 -kernel kernel64.bin


# housekeeping
clean :
	-rm *.o *.o32 *.o64 *.symbols *.section *.bin *.elf64 .depend

.PHONY : default debug qemu clean depend
