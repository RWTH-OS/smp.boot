/*
 * =====================================================================================
 *
 *       Filename:  tests.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.11.2011 09:19:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "system.h"
#include "smp.h"
#include "sync.h"
#include "mm.h"
#include "cpu.h"
#include "keyboard.h"
#include "menu.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_TESTS > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_TESTS > 1)

extern volatile unsigned cpu_online;


void tests_barrier(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    unsigned u;
    static barrier_t barr2 = BARRIER_INITIALIZER(2);

    if (cpu_online >= 2 && myid < 2) {
        barrier(&barr2);
        for (u=0; u<20; u++) {

            if ((u % (myid+1)) == 0) udelay(1000*(u+1));

            barrier(&barr2);

        }
        barrier(&barr2);
    }

    barrier(&global_barrier);
    for (u=0; u<20; u++) {

        if ((u % (myid+1)) == 0) udelay(1000*(u+1));

        barrier(&global_barrier);
    }
    IFV printf("[%u] leaving test_barrier()\n", myid);
    barrier(&global_barrier);
}

void tests_flag(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    static flag_t flag = FLAG_INITIALIZER;
    
    barrier(&global_barrier);

    if (myid == 0) {
        udelay(1000000);
        printf("[0] signal flag\n");
        flag_signal(&flag);

        udelay(1000000);
        printf("[0] signal second flag\n");
        flag_signal(&flag);

        udelay(2000000);
        printf("[0] signal third flag\n");
        flag_signal(&flag);

    } else if (myid == 1) {
        flag_wait(&flag);
        printf("[1] detected flag\n");
        flag_wait(&flag);
        printf("[1] detected second flag\n");

        while (!flag_trywait(&flag)) {
            udelay(100000);
            printf("z");
        }
        printf("[1] detected third flag\n");

    }

}

void tests_mm(void)
{
    static barrier_t barr = BARRIER_INITIALIZER(2);
    unsigned myid = my_cpu_info()->cpu_id;
    static volatile uint32_t * volatile p_shared = NULL;
    static volatile uint32_t * volatile p_shared2 = NULL;
    static volatile uint32_t * volatile p_shared3 = NULL;
    static volatile uint32_t * volatile p_shared4 = NULL;

    if (cpu_online >= 2) {
        if (myid == 0) {
            /* call Task for CPU 0 */
            p_shared = heap_alloc(1, 0);   // one page = 4kB
            p_shared2 = heap_alloc(4, 0);   // one page = 16kB
            printf("[0] p_shared = 0x%x\n", p_shared);
            printf("[0] p_shared2 = 0x%x\n", p_shared2);
            udelay(1*1000*1000);
            barrier(&barr);         // --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 
            p_shared3 = heap_alloc(2, 0);   // two pages = 8kB
            barrier(&barr);         // --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 
            printf("[0] p_shared[1023] = 0x%x (should be 0x01010101)\n", p_shared[1023]);
            printf("[0] p_shared2[2048] = 0x%x (should be 0x22222222)\n", p_shared2[2048]);

            barrier(&barr);         // --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 
            //virt_to_phys((void*)p_shared);
            p_shared[0] = 0xdeadbeef;
            p_shared2[0] = 0xdeadbeef;
            p_shared3[0] = 0xdeadbeef;
            barrier(&barr);         // --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 
            virt_to_phys((void*)p_shared4);
            //__asm__ volatile ("invlpg %0" : : "m"(*(int*)p_shared4));
            //tlb_shootdown((void*)p_shared4, 2*4*KB);
            p_shared4[1] = 0xdeadc0de;

            printf("[0]: udelay 5 Sek.\n");
            udelay(5*1000*1000);
            printf("[0]: exit now\n");
            barrier(&barr);         // --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 
        } else if (myid == 1) {
            /* call Task for CPU 1 */
            barrier(&barr);         // --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 1 --- 
            p_shared4 = heap_alloc(2, 0);   // two pages = 8kB
            udelay(1*1000*1000);
            printf("[1] p_shared = 0x%x\n", p_shared);
            printf("[1] p_shared2 = 0x%x\n", p_shared2);
            printf("[1] p_shared3 = 0x%x\n", p_shared3);
            printf("[1] p_shared4 = 0x%x\n", p_shared4);
            virt_to_phys((void*)p_shared4);
            udelay(1*1000*1000);
            memset((void*)p_shared, 1, 4096);
            memset((void*)p_shared2, 0x22, 4*4096);
            barrier(&barr);         // --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 2 --- 
            
            barrier(&barr);         // --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 3 --- 
            p_shared4[0] = 0xdeadbeef;
            barrier(&barr);         // --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 4 --- 
            p_shared[1] = 0xdeadc0de;
            p_shared2[1] = 0xdeadc0de;
            p_shared3[1] = 0xdeadc0de;

            printf("[1]: udelay 10 Sek.\n");
            udelay(10*1000*1000);
            printf("[1]: exit now\n");
            barrier(&barr);         // --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 5 --- 
        } 
    } else {
        printf("only one CPU active, this task needs at least two.\n");
    }

}
void tests_mm_reconf()
{
    unsigned myid = my_cpu_info()->cpu_id;
    void *p_buffer = 0;
    size_t size = 16*KB;        // 4 pages, fits into L1$

    void membench() 
    {
        unsigned i, j;
        uint64_t t1, t2;
        volatile uint32_t *p = (volatile uint32_t*)p_buffer;
        t1 = rdtsc();
        for (i=0; i<4096; i++) {
            for (j=0; j<size; j += 4) {
                p[10]++;
            }
        }
        t2 = rdtsc();
        printf("membench: %u tics/access\n", (t2-t1)/4096/(size/4));
    }

    if (myid == 0) {
        udelay(100000);
        p_buffer = heap_alloc(size/PAGE_SIZE, MM_CACHE_DISABLE);
        membench();
        membench();
        heap_reconfig(p_buffer, size, MM_WRITE_THROUGH);
        membench();
        membench();
        heap_reconfig(p_buffer, size, 0);
        membench();
        membench();
        heap_reconfig(p_buffer, size, MM_CACHE_DISABLE);
        membench();
        membench();

    }


}

void tests_ipi(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    unsigned if_backup;

    if_backup = sti();
    barrier(&global_barrier);
    if (cpu_online > 1) {

        if (myid == 0) {
            unsigned u;

            //__asm__ volatile ("int $31");
            
            IFV printf("[0] issue INT 128...\n");
            __asm__ volatile ("int $128");
            udelay(1000000);
            
            IFV printf("[0] send IPI vector 128 to self\n");
            send_ipi(0, 0x80);
            udelay(5000000);

            IFV printf("[0] send IPI vector %u to all CPUs\n", 0x80);
            for (u = 1; u < cpu_online; u++) {
                udelay(1000000);
                IFVV printf("  IPI to %u\n", u);
                send_ipi(u, 0x80);
            }

            barrier(&global_barrier);

            udelay(5000000);
            IFV printf("[0] call cmp_wakeup() for all CPUs\n");
            for (u = 1; u < cpu_online; u++) {
                udelay(1000000);
                IFVV printf("  cmp_wakeup(%u)\n", u);
                smp_wakeup(u);
            }
        } else {

            if (myid == 1) {
                udelay(1000000);
                IFV printf("[1] issue INT 128 on CPU 1\n");
                __asm__ volatile ("int $128");
            }


            smp_status('H');
            __asm__ volatile ("hlt");   // should be waken up by IPI (TODO: works in 64 bit mode, but not in 32 bit...)
            smp_status(STATUS_RUNNING);

            udelay(1000000);
            barrier(&global_barrier);
            
            smp_halt();
        }

    } else {
        printf("[%u] tests_ipi: can only be executed with more than one CPU (cpu_online=%u)\n", myid, cpu_online);
    }
    if (!if_backup) cli();  // restore previous state of interrupt flag
    barrier(&global_barrier);
}

void tests_printf(void)
{
    if (CPU_ID == 0) {
        unsigned b, f;
        printf("     ");
        for (b = 0; b<8; b++) {
            printf(" %#x", b);
        }
        printf("\n");
        for (f = 0; f<16; f++) {
            settextcolor(COLOR_FG_WHITE, COLOR_BG_BLACK);
            printf("%#x ", f);
            for (b = 0; b<8; b++) {
                settextcolor(f, b);
                printf("abcd ", b);
            }
            printf("\n");
        }
        settextcolor(COLOR_FG_WHITE, COLOR_BG_BLACK);

        printf("\n");
        printf("u:'%u', x:'%x', k:'%#u', u:'%u', k:'%#u', u:'%u'\n", 123, 123, 123, 123, 123, 123);
        printf("u:'%u', x:'%x', k:'%#u', #x:'%#x', k:'%#u', u:'%u'\n", 123, 123, 123, 123, 123, 123);
        printf("u:'%u', x:'%x', k:'%#10u', #8x:'%#8x', k:'%#u', u:'%u'\n", 123, 123, 123, 123, 123, 123);
        for (unsigned u=512; u<128*MB; u*=8) printf("%10u: %#uB\n", u, u);
    }
    barrier(&global_barrier);
}
void tests_keyboard(void)
{
    if (CPU_ID == 0) {
        unsigned u;
        unsigned scancode, keycode;
        printf("polling ~30 secs for keyboard scan codes (press some keys...)\n");
        for (u=0; u<64; u++) {
            scancode = keyboard_get_scancode();
            printf("%x ", scancode);
            udelay(500*1000);
        }
        printf("\n");

        printf("polling ~30 secs for keyboard input (press some keys...)\n");
        for (u=0; u<64; u++) {
            scancode = keyboard_get_scancode();
            keycode = scancode_to_keycode(scancode);
            if (keycode) putch(keycode); 
            printf(".");
            udelay(500*1000);
        }
        printf("\n");

        //printf("press any key...");
        //keycode = wait_for_key();
        //putch((uint8_t)keycode);
        //printf("\n");
    }
    barrier(&global_barrier);
}

void tests_doall(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    barrier(&global_barrier);

    IFV printf("[%u] calling test_barrier()\n", myid);
    tests_barrier();
    tests_flag();

    tests_mm();
    tests_mm_reconf();

    tests_ipi();

    tests_printf();
    tests_keyboard();
    
    printf("[%u] exit tests_doall()\n", myid);
}

void tests_menu(void)
{
    int t = 0xFFFF;
    do {
        menu_entry_t testmenu[] = {
            {0xFFFF, "<all>"},
            {1 << 0, "Barrier, Flag"},
            {1 << 1, "Memory-Management"},
            {1 << 2, "Interprocessor Interrupts (IPI)"},
            {1 << 3, "printf"},
            {1 << 4, "keyboard"},
            {0x10000, "return"},
            {0,0}
        };
        t = menu("Tests", testmenu, t);
        if (t & (1 << 0)) {
            tests_barrier();
            tests_flag();
        }
        if (t & (1 << 1)) {
            tests_mm();
            tests_mm_reconf();
        }
        if (t & (1 << 2)) {
            tests_ipi();
        }
        if (t & (1 << 3)) {
            tests_printf();
        }
        if (t & (1 << 4)) {
            tests_keyboard();
        }
    } while (t != 0x10000);

}

