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

#define IFV   if (VERBOSE > 0 || VERBOSE_TESTS > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_TESTS > 1)

extern volatile unsigned cpu_online;

static barrier_t barr_all = BARRIER_INITIALIZER(MAX_CPU+1);

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

    if (myid == 0) {
        barr_all.max = cpu_online;
        /* wait until all others are in the following barrier */
        while (barr_all.cnt < (barr_all.max-1)) { };
    }
    barrier(&barr_all);
    for (u=0; u<20; u++) {

        if ((u % (myid+1)) == 0) udelay(1000*(u+1));

        barrier(&barr_all);
    }
    IFV printf("[%u] leaving test_barrier()\n", myid);
    barrier(&barr_all);
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
            p_shared = heap_alloc(1);   // one page = 4kB
            p_shared2 = heap_alloc(4);   // one page = 16kB
            printf("[0] p_shared = 0x%x\n", p_shared);
            printf("[0] p_shared2 = 0x%x\n", p_shared2);
            udelay(1*1000*1000);
            barrier(&barr);
            p_shared3 = heap_alloc(2);   // two pages = 8kB
            barrier(&barr);
            printf("p_shared[1023] = 0x%x (should be 0x01010101)\n", p_shared[1023]);
            printf("p_shared2[2048] = 0x%x (should be 0x22222222)\n", p_shared2[2048]);


            printf("CPU 0: udelay 5 Sek.\n");
            udelay(5*1000*1000);
            printf("CPU 0: exit now\n");
        } else if (myid == 1) {
            /* call Task for CPU 1 */
            barrier(&barr);
            p_shared4 = heap_alloc(2);   // two pages = 8kB
            udelay(1*1000*1000);
            printf("[1] p_shared = 0x%x\n", p_shared);
            printf("[1] p_shared2 = 0x%x\n", p_shared2);
            printf("[1] p_shared3 = 0x%x\n", p_shared3);
            printf("[1] p_shared4 = 0x%x\n", p_shared4);
            udelay(1*1000*1000);
            memset((void*)p_shared, 1, 4096);
            memset((void*)p_shared2, 0x22, 4*4096);
            barrier(&barr);

            printf("CPU 1: udelay 10 Sek.\n");
            udelay(10*1000*1000);
            printf("CPU 1: exit now\n");
        } 
    } else {
        printf("only one CPU active, this task needs at least two.\n");
    }

}

void tests_ipi(void)
{
    unsigned myid = my_cpu_info()->cpu_id;

    sti();
    if (cpu_online > 1) {

        if (myid == 0) {
            unsigned u;

            //asm volatile ("int $31");
            
            IFVV printf("issue INT 128...\n");
            asm volatile ("int $128");
            udelay(2000000);
            
            IFVV printf("send IPI vector 128 to self\n");
            send_ipi(0, 0x80);

            for (u = 1; u < cpu_online; u++) {
                udelay(1000000);
                IFVV printf("send IPI vector %u to CPU %u \n", 0x80, u);
                send_ipi(u, 0x80);
            }

            for (u = 1; u < cpu_online; u++) {
                udelay(1000000);
                IFVV printf("send IPI vector %u to CPU %u \n", 0x80, u);
                smp_wakeup(u);
            }
        } else {

            if (myid == 1) {
                udelay(1000000);
                IFVV printf("issue INT 128 on CPU 1\n");
                asm volatile ("int $128");
            }

            smp_status('H');
            asm volatile ("hlt");   // should be waken up by IPI, but apparantly, IS NOT.
            smp_status('.');
            
            smp_halt();
        }

    } else {
        printf("tests_ipi: can only be executed with more than one CPU\n");
    }
    barrier(&barr_all);
}

void tests_doall(void)
{
    unsigned myid = my_cpu_info()->cpu_id;

    IFV printf("[%u] calling test_barrier()\n", myid);
    tests_barrier();

    //tests_mm();

    tests_ipi();

    printf("[%u] exit tests_doall()\n", myid);
}
