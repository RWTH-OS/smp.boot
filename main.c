#include "system.h"
#include "multiboot_struct.h"
#include "info.h"
#include "smp.h"
#include "keyboard.h"
#include "sync.h"
#include "mm.h"
#include "pit.h"
#include "debug.h"
#include "cpu.h"
#include "pci.h"
#include "smm.h"
#include "menu.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_MAIN > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_MAIN > 1)

hw_info_t hw_info; 



extern volatile unsigned cpu_online;    // from apic.c
void main();                            // further down in this file

#if __x86_64__
#define VERSION_BITS "64"
#else
#define VERSION_BITS "32"
#endif
#include "version.h"
static char name_version[] = "BareMetalKernel" VERSION_BITS " " SVN_REV " OPT=" OPT;

/*
 * this is the entry function only for the BSP
 */
void main_bsp(void)
{
    char *vendor[] = {"Intel", "AMD", "unknown"};

    *((uint32_t*)0xB8000) = 0x1F381F39;     /* "98" */

    {
        char *chp = "gwCharpointer\n";
        char str[] = "gwString\n";
        char buf[20] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

        itoa(buf, 'x', vendor[0]);      *((uint32_t*)0xB8008) = 0x1F301F30;
        putch(buf[0]);                  *((uint32_t*)0xB8008) = 0x1F311F30;
        putch(buf[1]);                  *((uint32_t*)0xB8008) = 0x1F321F30;
        putch(buf[2]);                  *((uint32_t*)0xB8008) = 0x1F331F30;
        putch(buf[3]);                  *((uint32_t*)0xB8008) = 0x1F341F30;
        putch(buf[4]);                  *((uint32_t*)0xB8008) = 0x1F351F30;
        putch(buf[5]);                  *((uint32_t*)0xB8008) = 0x1F361F30;
        putch(buf[6]);                  *((uint32_t*)0xB8008) = 0x1F371F30;
        putch(buf[7]);                  *((uint32_t*)0xB8008) = 0x1F381F30;
        putch('=');                     *((uint32_t*)0xB8008) = 0x1F391F30;
        putch(*(vendor[0]));            *((uint32_t*)0xB8008) = 0x1F411F30;
        putch(*(vendor[0]+1));          *((uint32_t*)0xB8008) = 0x1F421F30;
        putch(*(vendor[0]+2));          *((uint32_t*)0xB8008) = 0x1F431F30;
        putch(*(vendor[0]+3));          *((uint32_t*)0xB8008) = 0x1F441F30;
        putch(*(vendor[0]+4));          *((uint32_t*)0xB8008) = 0x1F451F30;
        putch('\n');                    *((uint32_t*)0xB8008) = 0x1F461F30;

        itoa(buf, 'x', str);            *((uint32_t*)0xB8008) = 0x1F301F31;
        putch(buf[0]);                  *((uint32_t*)0xB8008) = 0x1F311F31;
        putch(buf[1]);                  *((uint32_t*)0xB8008) = 0x1F321F31;
        putch(buf[2]);                  *((uint32_t*)0xB8008) = 0x1F331F31;
        putch(buf[3]);                  *((uint32_t*)0xB8008) = 0x1F341F31;
        putch(buf[4]);                  *((uint32_t*)0xB8008) = 0x1F351F31;
        putch(buf[5]);                  *((uint32_t*)0xB8008) = 0x1F361F31;
        putch(buf[6]);                  *((uint32_t*)0xB8008) = 0x1F371F31;
        putch(buf[7]);                  *((uint32_t*)0xB8008) = 0x1F381F31;
        putch('=');                     *((uint32_t*)0xB8008) = 0x1F391F31;
        putch(*(str));                  *((uint32_t*)0xB8008) = 0x1F411F31;
        putch(*(str+1));                *((uint32_t*)0xB8008) = 0x1F421F31;
        putch(*(str+2));                *((uint32_t*)0xB8008) = 0x1F431F31;
        putch(*(str+3));                *((uint32_t*)0xB8008) = 0x1F441F31;
        putch(*(str+4));                *((uint32_t*)0xB8008) = 0x1F451F31;
        putch('\n');                    *((uint32_t*)0xB8008) = 0x1F461F31;

        itoa(buf, 'x', chp);            *((uint32_t*)0xB8008) = 0x1F301F32;
        putch(buf[0]);                  *((uint32_t*)0xB8008) = 0x1F311F32;
        putch(buf[1]);                  *((uint32_t*)0xB8008) = 0x1F321F32;
        putch(buf[2]);                  *((uint32_t*)0xB8008) = 0x1F331F32;
        putch(buf[3]);                  *((uint32_t*)0xB8008) = 0x1F341F32;
        putch(buf[4]);                  *((uint32_t*)0xB8008) = 0x1F351F32;
        putch(buf[5]);                  *((uint32_t*)0xB8008) = 0x1F361F32;
        putch(buf[6]);                  *((uint32_t*)0xB8008) = 0x1F371F32;
        putch(buf[7]);                  *((uint32_t*)0xB8008) = 0x1F381F32;
        putch('=');                     *((uint32_t*)0xB8008) = 0x1F391F32;
        putch(*(chp));                  *((uint32_t*)0xB8008) = 0x1F411F32;
        putch(*(chp+1));                *((uint32_t*)0xB8008) = 0x1F421F32;
        putch(*(chp+2));                *((uint32_t*)0xB8008) = 0x1F431F32;
        putch(*(chp+3));                *((uint32_t*)0xB8008) = 0x1F441F32;
        putch(*(chp+4));                *((uint32_t*)0xB8008) = 0x1F451F32;
        putch('\n');                    *((uint32_t*)0xB8008) = 0x1F461F32;

        //printf(str);                    *((uint32_t*)0xB8008) = 0x1F301F33;
        //printf(chp);                    *((uint32_t*)0xB8008) = 0x1F311F33;
        //printf(vendor[0]);              *((uint32_t*)0xB8008) = 0x1F321F33;
        *((uint32_t*)0xB8000) = 0x1F391F39;     /* "99" top left corner to say: "I've arrived in main()." */
        while (1) {
            __asm__ volatile ("hlt");
        }
    }



    *((uint32_t*)0xB8000) = 0x1F391F39;     /* "99" top left corner to say: "I've arrived in main()." */
    //status_putch(6, '/');

    init_video();
    smp_init();
    IFVV printf("my_cpu_info()->cpu_id: %u\n", my_cpu_info()->cpu_id);


    IFV puts("main(): video initialized\n");
    IFVV printf("found %d %s CPUs and %d I/O APICs\n", 
            (ptr_t)hw_info.cpu_cnt, vendor[hw_info.cpu_vendor], (ptr_t)hw_info.ioapic_cnt);
    //udelay(DELAY);

    mm_init();
    IFVV printf("my_cpu_info()->cpu_id: %u\n", my_cpu_info()->cpu_id);

#if SCROLLBACK_BUF_SIZE
    init_video_scrollback();
#endif

    idt_install();
    IFV puts("idt installed\n");
    //udelay(DELAY);

    isr_install();
    IFV puts("isr installed\n");
    //udelay(DELAY);
    
    pit_init();
    IFV puts("tsc calibrated\n");
    
    apic_init();    /* this is where the APs are waked up */
    IFV puts("apic initialized\n");

    pci_init();
    IFV puts("pci initialized\n");

    smm_init();
    IFV puts("SMM initialized\n");

    keyboard_init(kbm_poll);
    IFV puts("keyboard initialized\n");


    IFV puts("my kernel is running in main_bsp now...\n");

    char *p = name_version;
    unsigned u;
    for (u=40; u<75 && *p != 0; u++) {
        status_putch(u, *p++);
    }

    //unsigned eax, ebx, ecx, edx;
    //cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    //printf("support 1 GB pages: %d\n", edx & (1<<26));
    
    //printf("cpuid_max: 0x%x   cpuid_high_max: 0x%x\n", hw_info.cpuid_max, hw_info.cpuid_high_max);
    //printf("cpuid_family: 0x%x\n", hw_info.cpuid_family);


    //int i; for (i=0; i< 40; i++) printf("Test line %d\n", i);
    //print_multiboot_info();
    //print_smp_iboot32.o nfo();

    IFVV {
        printf("offset of stack[0] : 0x%x ", &(stack[0]));
        ptr_t sp;
#       if __x86_64__
        __asm__ volatile("movq %%rsp, %%rax" : "=a"(sp) );
#       else
        __asm__ volatile("movl %%esp, %%eax" : "=a"(sp) );
#       endif
        printf("my sp: 0x%x ", sp);
        printf("my_cpu_info: 0x%x ", my_cpu_info());
        printf("[%u]\n", my_cpu_info()->cpu_id);
    }
    //printf("new[0]: cpu_info = %x cpu_id = %x\n", my_cpu_info(), my_cpu_info()->cpu_id);
    
    //multiboot_info(); 
    

    cpu_online++;       // the BSP is there, too
    global_barrier.max = cpu_online;
    /* wait until all others are in the following barrier */
    while (global_barrier.cnt < (global_barrier.max-1)) {};
    barrier(&global_barrier);

    main();
}



/*
 * this is the entry function for the APs.
 */
void main_ap(void)
{
    cpu_online++;
    /*
     * Signal, that the initialization of this CPU is done. 
     * If I'm not here in time, apic_init() will trylock() my wakelock
     * and I will block here.
     */
    mutex_lock(&(my_cpu_info()->wakelock)); 

    apic_init_ap(cpu_online);     // activate localAPIC on Application Processors
    idt_install_ap();

    smp_status(STATUS_RUNNING);

    //udelay(3000000*my_id);
    //printf("new[%d]: cpu_info = %x cpu_id = %x\n", cpu_online, my_cpu_info(), my_cpu_info()->cpu_id);

    barrier(&global_barrier);
    main();
}


#include "payload.h"
#include "tests.h"

/*
 * this is the main function that is entered by all CPUs after the initialization 
 * (no difference between BSP and AP, anymore)
 */
void main()
{

    IFVV printf("CPU %d/%d entering in main()\n", my_cpu_info()->cpu_id, cpu_online);

    if (my_cpu_info()->cpu_id == 0) {
        printf("*****************************************\n");
        printf("* CPU Vendor: %s \n", vendor_string[hw_info.cpu_vendor]);
        printf("* CPU Name: '%s' \n", hw_info.cpuid_processor_name.c);
        printf("* max CPUID fn: 0x%x, 0x%x\n", hw_info.cpuid_max, hw_info.cpuid_high_max);
        printf("* Nbr of threads/package: %u \n", hw_info.cpuid_threads_per_package);
        printf("* Cache-Line Size: %u \n", (ptr_t)hw_info.cpuid_cachelinesize);
        printf("* L1$: %#uB %c + %#uB %c\n", 
                hw_info.cpuid_cache[0].size, 
                hw_info.cpuid_cache[0].type, 
                hw_info.cpuid_cache[1].size,
                hw_info.cpuid_cache[1].type); 
        printf("* L2$: %#uB\n", hw_info.cpuid_cache[2].size);
        if (hw_info.cpuid_cache[3].size > 0) 
            printf("* L3$: %#uB\n", hw_info.cpuid_cache[3].size);
        printf("*****************************************\n");

    }

#if OFFER_MENU

    barrier(&global_barrier);
    static volatile enum {mode_default, mode_menu} mode = mode_default;

    if (CPU_ID == 0) {
        int i;
        keyboard_clear_buf();
        printf("To interrupt default mode and get a menu, press any key within 5 Sek.\b\b\b\b\b\b");
        for (i=5; i>-1; i--) {
            udelay(1000000);
            printf("%i\b", i);
            if (keyboard_get_scancode() != 0) {
                mode = mode_menu;
                break;
            }
        }
        printf("\n");
    }

    barrier(&global_barrier);

    if (mode == mode_menu) {
        menu_entry_t mainmenu[] = {
            {1, "default"}, 
            {2, "tests >"}, 
            {3, "benchmarks >"}, 
            {999, "exit"}, 
            {0,0},
        };
        int m1;
        do {
            m1 = menu("Main menu", mainmenu, 1);
            switch (m1) {
                case 1 :
                    tests_doall();
                    payload_benchmark();
                    break;
                case 2 :
                    tests_menu();
                    break;
                case 3 :
                    payload_benchmark_menu();
                    break;
                case 999 :
                    break;
            }
        } while (m1 != 999);

    } else
#endif
    {
        /* call tests */
        tests_doall();

        /* call a payload */
        payload_benchmark();
    }



    /* all CPUs leaving the payload: go to sleep */
    stop();
}

