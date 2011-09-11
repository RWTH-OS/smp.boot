/*
 * This is 32 bit C code for help during the boot sequence.
 * It is used for the 64 bit kernel but should be suited for the 32 bit kernel, too.
 * This is called from startXX.asm in REAL MODE (physical addresses)
 */

#include "bda_struct.h"
#include "acpi_struct.h"
#include "mps_struct.h"
#include "info.h"

#define MP_FLT_SIGNATURE 0x5f504d5f


extern hw_info_t hw_info; 

/* scrn.c */
void cls();
void putch(char c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();
void itoa (char *buf, int base, long d);
void printf (const char *format, ...);


extern uint32_t cpuid_max_low, cpuid_max_high, cpuid_family;
void cpu_features()
{
    uint32_t eax, ebx, ecx, edx;
    void cpuid(unsigned func)
    {
        asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(func));
    }

    /* it was checked before, that the CPUID instruction is available */
    cpuid(0);
    hw_info.cpuid_max = eax;

    cpuid(0x80000000);
    hw_info.cpuid_high_max = eax;

    cpuid(1);
    hw_info.cpuid_family = (eax>>8)&0x1F;


}

static ptr_t search_rsdp(ptr_t offset, ptr_t size) 
{
    union {
        char str[9];
        uint32_t u32[2];
    } __attribute__((packed)) acpi_sig = { "RSD PTR " };

    volatile const uint32_t *p;
    unsigned i;

    p = (uint32_t*)offset;

    for (i = 0; i < size/sizeof(uint32_t); i++) {
        if (p[i] == acpi_sig.u32[0] && p[i+1] == acpi_sig.u32[1]) {
            return (ptr_t)(void*)&p[i];
        }
    }

    return 0;
}

/*
 * get_info()
 *
 * This function is called from the 32 bit startup code in real-mode to collect
 * information about the hardware from
 *  - BIOS Data Area (BDA) / Extended BDA (EBDA)
 *  - ACPI (if available) (1)
 *  - Multiprocessor Specification tables (if available)
 * The collected information is stored in internal data structures that are
 * shared with the protected mode kernel (32- or 64-bit).
 *
 *
 *  (1) note: this OS does not support ACPI Power Management but only reads
 *      information about available CPUs and APICs from the ACPI tables.
 */

void get_info()
{
	//unsigned long addr;
	//unsigned i;//, count;
    init_video();
    printf("get_info()\n");

    /*
     * first, read BIOS Data Area
     */
    bda_t *bda = (bda_t*)0x400;

    /* get EBDA Segment 16-bit real-mode segment, i.e. shift left by 4 bits to get physical address */
    ebda_t *ebda = (ebda_t*)((ptr_t)bda->segm_ebda << 4);
    hw_info.ebda_adr = bda->segm_ebda << 4;
    hw_info.ebda_size = ebda->size;
    

    /*
     * search for ACPI tables 
     *  - EBDA
     *  - physical address range 0xE0000 .. 0xFFFFF
     */
    rsdp_t *rsdp = (rsdp_t*)search_rsdp(hw_info.ebda_adr, hw_info.ebda_size);
    if (rsdp == 0) {
        rsdp_t *rsdp = (rsdp_t*)search_rsdp(0xE0000, 0x20000);
    }
    if (rsdp == 0) {
        printf("no RSDP found.\n");
        goto skip_acpi;
    }

skip_acpi:
    goto skip_acpi;
/*
    // search for Multi-Processor info structure. See: MetalSVM/arch/x86/kernel/apic.c:apic_probe()
	// searching MP signature in the reserved memory areas
	if (global_mbi && (global_mbi->flags & MULTIBOOT_INFO_MEM_MAP)) {
 		multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) global_mbi->mmap_addr;
		multiboot_memory_map_t* mmap_end = (void*) ((unsigned long) global_mbi->mmap_addr + global_mbi->mmap_length);

		while (mmap < mmap_end) {
			if (mmap->type == MULTIBOOT_MEMORY_RESERVED) {
				addr = mmap->addr;

				/ *
				 * MultiProcessor Specification 1.4:
				 * =================================
				 * The following is a list of the suggested memory spaces for the MP configuration table:
				 * a. In the first kilobyte of Extended BIOS Data Area (EBDA), or
				 * b. Within the last kilobyte of system base memory if the EBDA segment is undefined, or
				 * c. At the top of system physical memory, or
				 * d. In the BIOS read-only memory space between 0E0000h and 0FFFFFh.
				 * /
				for(i=0; (i<mmap->len-sizeof(unsigned)) && (addr < 0x0FFFFF); i++, addr++) {
					if (*((unsigned*) addr) == MP_FLT_SIGNATURE) {
						global_mp = (apic_mp_t*) addr;
						if (!((global_mp->version > 4) || global_mp->features[0]))
							goto found_mp;
					}
				}
				
			}
			mmap++;
		}
	}
    */

//found_mp:
    return;
}
