/*
 * This is 32 bit C code for help during the boot sequence.
 * It is used for the 64 bit kernel but should be suited for the 32 bit kernel, too.
 */

#include <multiboot.h>
#include <apic.h>
#include <stddef.h>
#define MP_FLT_SIGNATURE 0x5f504d5f

extern multiboot_info_t *global_mbi;
extern apic_mp_t *global_mp;


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
    cpuid_max_low = eax;

    cpuid(0x80000000);
    cpuid_max_high = eax;

    cpuid(1);
    cpuid_family = (eax>>8)&0x1F;


}




void search_mp()
    // search for Multi-Processor info structure. See: MetalSVM/arch/x86/kernel/apic.c:apic_probe()
{
	unsigned long addr;
	unsigned i;//, count;

    global_mp = 0;

	// searching MP signature in the reserved memory areas
	if (global_mbi && (global_mbi->flags & MULTIBOOT_INFO_MEM_MAP)) {
 		multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) global_mbi->mmap_addr;
		multiboot_memory_map_t* mmap_end = (void*) ((unsigned long) global_mbi->mmap_addr + global_mbi->mmap_length);

		while (mmap < mmap_end) {
			if (mmap->type == MULTIBOOT_MEMORY_RESERVED) {
				addr = mmap->addr;

				/*
				 * MultiProcessor Specification 1.4:
				 * =================================
				 * The following is a list of the suggested memory spaces for the MP configuration table:
				 * a. In the first kilobyte of Extended BIOS Data Area (EBDA), or
				 * b. Within the last kilobyte of system base memory if the EBDA segment is undefined, or
				 * c. At the top of system physical memory, or
				 * d. In the BIOS read-only memory space between 0E0000h and 0FFFFFh.
				 */
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

found_mp:
    return;
}
