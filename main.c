#include "system.h"
#include "multiboot_struct.h"
#include "info.h"

hw_info_t hw_info; 

void print_multiboot_info(void) 
{
    multiboot_info_t *mbi = (multiboot_info_t*)(ptr_t)hw_info.mb_adr;
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
        unsigned i;
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
    asm volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func));
}
void cpuid2(unsigned func, unsigned subfunc, unsigned *eax, unsigned *ebx, unsigned *ecx, unsigned *edx)
{
    asm volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func), "c"(subfunc));
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




uint64_t tsc_per_usec = TSC_PER_USEC;
uint64_t tsc_per_sec = TSC_PER_USEC*1000000ul;

void udelay(unsigned long us)
{
    uint64_t tsc_now, tsc_end;
    tsc_now = rdtsc();
    //printf("tsc %lu\n", tsc_now.u64);
    tsc_end = tsc_now + (us * tsc_per_usec);
    while (tsc_now < tsc_end) {
        tsc_now = rdtsc();
        //printf("tsc %lu\n", tsc_now.u64);
    }
}
/* deactivate warning on divide-by-zero, b/c this is intentional in this function */
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
void test_div_zero()
{
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
    printf("1/0 = %d", 1/0);    /* divide by zero to test DIVZERO exception */
    printf("\nafter DIV ZERO\n");
    udelay(500000);
}

void reboot(int timeout)
{
    int i;
    printf("rebooting in ");
    for (i=timeout; i>0; i--) {
        printf("%d ", i);
        udelay(1000000);
    }
    printf("\n");

    asm volatile ("cli");
    unsigned char tmp;
    do {
        tmp = inportb(0x64);
        if (tmp & 0x01) {
            inportb(0x60);
        }
    } while (tmp & 0x02);
    outportb(0x64, 0xFE);

    /* if that didn't work */

    /* TODO: other possibility: kill IDT, issue Triple Fault */

    /* if that didn't work either */
    while (1) {
        asm volatile ("hlt");
    }
    


}
/*
 * TODO's
 *  - is the cache activated?
 *  - dynamic memory management (at least, malloc() should be implemented)
 */
#define DELAY 100

/*
 * this is the entry function only for the BSP
 */
void main(void)
{
    char *vendor[] = {"Intel", "AMD", "unknown"};

    *((uint32_t*)0xB8000) = 0x1F391F39;     /* "99" top left corner to say: "I've arrived in main()." */
    //status_putch(6, '/');

    udelay(100);

    init_video();
    puts("video initialized\n");
    printf("found %d %s CPUs and %d I/O APICs\n", (ptr_t)hw_info.cpu_cnt, vendor[hw_info.cpu_vendor], (ptr_t)hw_info.ioapic_cnt);
    //udelay(DELAY);

    idt_install();
    puts("idt installed\n");
    //udelay(DELAY);

    isr_install();
    puts("isr installed\n");
    //udelay(DELAY);

    apic_init();
    puts("apic initialized\n");

    puts("my kernel is running in main now...\n");

    //unsigned eax, ebx, ecx, edx;
    //cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    //printf("support 1 GB pages: %d\n", edx & (1<<26));
    
    printf("cpuid_max: 0x%x   cpuid_high_max: 0x%x\n", hw_info.cpuid_max, hw_info.cpuid_high_max);
    printf("cpuid_family: 0x%x\n", hw_info.cpuid_family);


    //int i; for (i=0; i< 40; i++) printf("Test line %d\n", i);
    //print_multiboot_info();
    //print_smp_iboot32.o nfo();

    //test_div_zero();
    printf("The end.\n");
    //reboot(5);
}


extern volatile unsigned cpu_online;

/*
 * this is this entry function for the APs.
 */
void main_smp(void)
{
    cpu_online++;
    status_putch(6+cpu_online, 'x');

    /* TODO : wait on semaphore/flag until the BSP releases our task */
    while (1) asm volatile ("hlt");


}
		
