#include <system.h>
#include <multiboot.h>
#include <apic.h>

u32 global_mbi;
u32 global_mp;
uint32_t cpuid_max_low, cpuid_max_high, cpuid_family;

void print_multiboot_info(multiboot_info_t *mbi) 
{
    char mem_type[][10] = {"mem", "other"};

    printf("Multiboot-Flags: 0x%x\n", mbi->flags);

    if (mbi->flags & (1<<0)) {
        printf("flags[0] - mem_lower: 0x%x=%d  mem_upper: 0x%x=%d\n", mbi->mem_lower, mbi->mem_lower, mbi->mem_upper, mbi->mem_upper);
    }

    if (mbi->flags & (1<<1)) {
        printf("flags[1] - boot_device: %x %x %x %x (Part 3/2/1 / Drive)\n", mbi->boot_device&0xFF, (mbi->boot_device>>8)&0xFF, 
                (mbi->boot_device>>16)&0xFF, (mbi->boot_device>>24)&0xFF);
    }

    if (mbi->flags & (1<<2)) {
        printf("flags[2] - cmdline: '%s'\n", mbi->cmdline);
    }

    if (mbi->flags & (1<<3)) {
        printf("flags[3] - mods_count: %d  mods_addr: 0x%x\n", mbi->mods_count, mbi->mods_addr);
        int i;
        for (i=0; i<mbi->mods_count; i++) {
            printf("  mod[%d]...\n", i);
        }
    }

    if (mbi->flags & (1<<4)) {
        printf("flags[4] - Symbol table for a.out image...\n");
    }

    if (mbi->flags & (1<<5)) {
        printf("flags[5] - Section header table for ELF kernel...\n");
    }

    if (mbi->flags & (1<<6)) {
        printf("flags[6] - mmap_length: %d  mmap_addr: 0x%x\n", mbi->mmap_length, mbi->mmap_addr);
        multiboot_memory_map_t* p = (multiboot_memory_map_t*)(long)mbi->mmap_addr;
        for ( ; p < (multiboot_memory_map_t*)(long)(mbi->mmap_addr+mbi->mmap_length); p = ((void*)p + p->size + 4)) {
            printf("  mmap[0x%x] - addr:0x%x  len:0x%x  type: %d (%s)\n",
                   p, (multiboot_uint32_t)(p->addr), (multiboot_uint32_t)(p->len), p->type, mem_type[p->type==1?0:1]);
        }
    }

    /* more bits available in flags, but their display is not implemented, yet */
    /* see: http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format */
}

void cpuid(unsigned func, unsigned *eax, unsigned *ebx, unsigned *ecx, unsigned *edx)
{
    asm volatile ("cpuid" : "=a"(*eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(func));
}
void cpuid2(unsigned func, unsigned subfunc, unsigned *eax, unsigned *ebx, unsigned *ecx, unsigned *edx)
{
    asm volatile ("cpuid" : "=a"(*eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(func), "c"(subfunc));
}
void print_smp_info(void)
{
    unsigned eax, ebx, ecx, edx;
    volatile unsigned *register_apic_id;
    unsigned i;

    cpuid(0x0B, &eax, &ebx, &ecx, &edx);
    printf("APIC ID from CPUID.0BH:EDX[31:0]: %d\n", edx);

    register_apic_id = (volatile unsigned *) (0xFEE00000 + 0x20);
    i = *register_apic_id;
    i = i >> 24;
    printf("local APIC ID from register: %d\n", i);

    cpuid(0x01, &eax, &ebx, &ecx, &edx);
    printf("support SMP: %d\n", edx & (1<<28));
    printf("addressable logical processors: %d\n", (ebx>>16)&0xFF);

    cpuid2(0x04, 0x00, &eax, &ebx, &ecx, &edx);
    i = (eax>>26)+1;
    printf("addressable processor cores: %d\n", i);
}

static inline void stackdump(int from, int to)
{
    long *sp;
    long *p;
#   ifdef __x86_64__
    asm volatile ("mov %%rsp, %%rax" : "=a"(sp));
#   else
    asm volatile ("mov %%esp, %%eax" : "=a"(sp));
#   endif
    printf("sp: %x\n", sp);
    for (p = sp-to; p <= sp-from; p++) {
        printf("[%x]   %x\n", p, *p);
    }
}
#define TSC_PER_USEC (2666ul)
#define TSC_PER_SEC (TSC_PER_USEC*1000ul*1000ul)

inline static uint64_t rdtsc(void)
{
	union {
		uint64_t u64;
		uint32_t u32[2];
	} x;
	asm volatile ("rdtsc" : "=a" (x.u32[0]), "=d"(x.u32[1]));
	return x.u64;
}
void udelay(unsigned long us)
{
    uint64_t tsc_now, tsc_end;
    tsc_now = rdtsc();
    //printf("tsc %lu\n", tsc_now.u64);
    tsc_end = tsc_now + us * TSC_PER_USEC;
    while (tsc_now < tsc_end) {
        tsc_now = rdtsc();
        //printf("tsc %lu\n", tsc_now.u64);
    }
}


void main(multiboot_info_t *mbi)
{
    *((uint32_t*)0xB8000) = 0x0F300F32;     /* "20" top left corner to say: "I've arrived in main()." */
    init_video();
    puts("video initialized\n");
    udelay(500000);
    idt_install();
    puts("idt installed\n");
    udelay(500000);
    isr_install();
    puts("isr installed\n");
    udelay(500000);

    puts("my kernel is running in main now...\n");

    //unsigned eax, ebx, ecx, edx;
    //cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    //printf("support 1 GB pages: %d\n", edx & (1<<26));
    
    printf("mp: %x\n", global_mp);
    printf("cpuid_max_low: 0x%x   cpuid_max_high: 0x%x\n", cpuid_max_low, cpuid_max_high);
    printf("cpuid_family: 0x%x\n", cpuid_family);


    print_multiboot_info(mbi);
    //print_smp_iboot32.o nfo();

    printf("DIV ZERO in 1 sec ");
    udelay(100000);
    printf("9");
    udelay(100000);
    printf("8");
    udelay(100000);
    printf("7");
    udelay(100000);
    printf("6");
    udelay(100000);
    printf("5");
    udelay(100000);
    printf("4");
    udelay(100000);
    printf("3");
    udelay(100000);
    printf("2");
    udelay(100000);
    printf("1");
    udelay(100000);
    printf("0");
    udelay(100000);
    printf("1/0 = %d", 1/0);
    printf("\nafter DIV ZERO\n");
    udelay(500000);
    printf("The end.\n");
}
		
