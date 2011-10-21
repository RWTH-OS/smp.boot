#include "system.h"
#include "multiboot_struct.h"
#include "info.h"
#include "smp.h"
#include "sync.h"
#include "mm.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_MAIN > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_MAIN > 1)

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


void multiboot_info(void)
{
    multiboot_info_t *p_mbi;
    printf("hw_info.mb_adr = %x\n", hw_info.mb_adr);
    p_mbi = (multiboot_info_t*)(ptr_t)hw_info.mb_adr;
    printf("p_mbi->flags = %x\n", p_mbi->flags);
    if (p_mbi->flags & (1<<2)) {
        printf("p_mbi->cmdline = %x ", p_mbi->cmdline);
        char *str = (char*)(ptr_t)p_mbi->cmdline;
        printf("'%s'\n", str);
    }
    printf("hw_info->cmd_maxcpu = %u\n", hw_info.cmd_maxcpu);
    printf("hw_info->cmd_cpumask = %u\n", hw_info.cmd_cpumask);
}


uint64_t tsc_per_usec = TSC_PER_USEC;
uint64_t tsc_per_sec = TSC_PER_USEC*1000000ul;

void udelay(unsigned long us)
{
    uint64_t tsc_now, tsc_end;
    smp_status('u');
    tsc_now = rdtsc();
    //printf("tsc %lu\n", tsc_now.u64);
    tsc_end = tsc_now + (us * tsc_per_usec);
    while (tsc_now < tsc_end) {
        tsc_now = rdtsc();
        //printf("tsc %lu\n", tsc_now.u64);
    }
    smp_status('.');
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

#if 0
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
#endif

/*
 * TODO's
 *  - is the cache activated?
 *  - dynamic memory management (at least, malloc() should be implemented)
 */
#define DELAY 100
barrier_t mainbarrier = BARRIER_INITIALIZER(MAX_CPU); /* max is later reduced to the actual number of CPUs */

extern volatile unsigned cpu_online;
void main();

/*
 * this is the entry function only for the BSP
 */
void main_bsp(void)
{
    char *vendor[] = {"Intel", "AMD", "unknown"};

    *((uint32_t*)0xB8000) = 0x1F391F39;     /* "99" top left corner to say: "I've arrived in main()." */
    //status_putch(6, '/');

    init_video();
    smp_init();

    IFV puts("main(): video initialized\n");
    IFVV printf("found %d %s CPUs and %d I/O APICs\n", (ptr_t)hw_info.cpu_cnt, vendor[hw_info.cpu_vendor], (ptr_t)hw_info.ioapic_cnt);
    //udelay(DELAY);

    mm_init();

    idt_install();
    IFV puts("idt installed\n");
    //udelay(DELAY);

    isr_install();
    IFV puts("isr installed\n");
    //udelay(DELAY);
    
    apic_init();
    IFV puts("apic initialized\n");

    IFV puts("my kernel is running in main_bsp now...\n");

    //unsigned eax, ebx, ecx, edx;
    //cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    //printf("support 1 GB pages: %d\n", edx & (1<<26));
    
    //printf("cpuid_max: 0x%x   cpuid_high_max: 0x%x\n", hw_info.cpuid_max, hw_info.cpuid_high_max);
    //printf("cpuid_family: 0x%x\n", hw_info.cpuid_family);


    //int i; for (i=0; i< 40; i++) printf("Test line %d\n", i);
    //print_multiboot_info();
    //print_smp_iboot32.o nfo();

    //printf("offset of stack[0] : %x\n", &(stack[0]));
    //printf("new[0]: cpu_info = %x cpu_id = %x\n", my_cpu_info(), my_cpu_info()->cpu_id);
    
    //multiboot_info();

    cpu_online++;       // the BSP is there, too
    mainbarrier.max = cpu_online;
    barrier(&mainbarrier);

    main();
}



/*
 * this is the entry function for the APs.
 */
void main_ap(void)
{
    cpu_online++;
    //status_putch(6+cpu_online, 'x');
    smp_status('x');

    //udelay(3000000*my_id);
    //printf("new[%d]: cpu_info = %x cpu_id = %x\n", cpu_online, my_cpu_info(), my_cpu_info()->cpu_id);

    barrier(&mainbarrier);
    main();
}

#include "payload.h"
void stop();

/*
 * this is the main function that is entered by all CPUs after the initialization 
 * (no difference between BSP and AP, anymore)
 */
void main()
{
    IFVV printf("CPU %d/%d entering in main()\n", my_cpu_info()->cpu_id, cpu_online);

    /* call a payload */
    payload_benchmark();

    /* all CPUs leaving the payload: go to sleep */
    stop();
}

#define KBRD_INTRFC  0x64
#define KBRD_BIT_KDATA 0
#define KBRD_BIT_UDATA 1

#define KBRD_IO  0x60
#define KBRD_RESET 0xFE

#define bit(n)   (1 << (n))
#define check_flag(flags, n)  ((flags) & bit(n))

void reboot()
{
    int s= 9 + cpu_online, i;
    
    status_putch(s++, 'R');
    status_putch(s++, 'e');
    status_putch(s++, 'b');
    status_putch(s++, 'o');
    status_putch(s++, 'o');
    status_putch(s++, 't');
    status_putch(s++, ' ');
    status_putch(s++, 'i');
    status_putch(s++, 'n');
    status_putch(s++, ' ');

    for (i=1; i<=5; i++) {
        status_putch(s++, '6'-i);
        udelay(1000*1000);
    }
    status_putch(s++, '0');
    //udelay(10);

#if 1
    char temp;
    asm volatile ("CLI");
    /* empty keyboard buffer */
    do {
        temp = inportb(KBRD_INTRFC);
        if (check_flag(temp, KBRD_BIT_KDATA) != 0) {
            inportb(KBRD_IO);
        }
    } while (check_flag(temp, KBRD_BIT_UDATA) != 0);

    /* issue reboot command */
    outportb(KBRD_INTRFC, KBRD_RESET);

    udelay(1000*1000);
    status_putch(s++, '+');
#endif

#if 1
    static struct {
        unsigned short length;
        unsigned long base;
    } __attribute__((__packed__)) IDTR;
 
    IDTR.length = 0;
    IDTR.base = (unsigned long)0;
    asm( "lidt %0" : : "m"(IDTR) );
    asm volatile ("int $32");

    udelay(1000*1000);
    status_putch(s++, '+');
#endif

    //udelay(100);
    smp_status('_');

    while (1) asm volatile ("hlt");
}
void stop()
{
    static unsigned cpus_halted = 0;
    static mutex_t m = MUTEX_INITIALIZER;

    mutex_lock(&m);
    cpus_halted++;
    mutex_unlock(&m);

    IFV printf("halt CPU %d (now %d down)\n", my_cpu_info()->cpu_id, cpus_halted);
    smp_status('_');
    if (cpus_halted < cpu_online) {
        while (1) asm volatile ("hlt");
    } else {
        reboot();
    }

}
		
