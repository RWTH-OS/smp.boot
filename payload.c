/*
 * =====================================================================================
 *
 *       Filename:  payload.c
 *
 *    Description:  This file contains some payloads to execute after the kernel has booted
 *
 *        Version:  1.0
 *        Created:  20.10.2011 09:21:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "sync.h"
#include "system.h"
#include "smp.h"
#include "mm.h"
#include "benchmark.h"
#include "smm.h"
#include "menu.h"

extern volatile unsigned cpu_online;

/*
 * The payload is called by all CPUs after complete initialization.
 * A Barrier is executed immediately before, so they should come in shortly.
 */


static size_t buffer_size = 16 * MB;
static void *p_buffer = NULL;
static size_t contender_size = 16 * MB;
static void *p_contender = NULL;

static void init_buffers()
{
    if (CPU_ID == 0) {

        if (p_buffer == NULL) {
            p_buffer = heap_alloc(buffer_size / PAGE_SIZE, BENCH_WORK_FLAGS);       // one page = 4kB
            /* no need for pre-faulting, because pages are present after heap_alloc()
             * (we don't have demand paging)
             * but initialize them */
            memset(p_buffer, 0, buffer_size);
        }


        if (p_contender == NULL) {
            p_contender = heap_alloc(contender_size / PAGE_SIZE, BENCH_LOAD_FLAGS);       // one page = 4kB
            //virt_to_phys(p_contender);
            //p_contender[0] = 42;
            //printf("[1] p_contender = 0x%x .. 0x%x\n", (ptr_t)p_contender, (ptr_t)p_contender+16*1024*1024);
            memset(p_contender, 0, contender_size);
        }
    }

}

void payload_benchmark()
{

    /*
     * count and collect all processors (collective barrier)
     */
    if (CPU_ID == 0) smm_deactivate();       // ...and (try to) deactivate SMM


    /*
     * Memory allocation
     */
    init_buffers();

    barrier(&global_barrier);

    /*
     *   Benchmarks
     */

    bench_hourglass();
    bench_hourglass_worker(p_contender);
    bench_hourglass_hyperthread();

    barrier(&global_barrier);

    bench_worker(p_buffer, p_contender);
    bench_worker_cut(p_buffer, p_contender, 16*KB);
    
    
    if (CPU_ID == 0) {
        heap_reconfig(p_buffer, buffer_size, 0);
        heap_reconfig(p_contender, contender_size, MM_CACHE_DISABLE);
        barrier(&global_barrier);
        printf("========  Benchmark: WB / Load: CD ===================================\n");
    } else {
        barrier(&global_barrier);
        tlb_shootdown(p_buffer, buffer_size);
        tlb_shootdown(p_contender, contender_size);
    }
    barrier(&global_barrier);
    

    bench_worker_cut(p_buffer, p_contender, 16*KB);
    bench_worker_cut(p_buffer, p_contender, 128*KB);

    
    if (CPU_ID == 0) {
        heap_reconfig(p_buffer, buffer_size, 0);
        heap_reconfig(p_contender, contender_size, MM_WRITE_THROUGH);
        barrier(&global_barrier);
        printf("========  Benchmark: WB / Load: WT ===================================\n");
    } else {
        barrier(&global_barrier);
        tlb_shootdown(p_buffer, buffer_size);
        tlb_shootdown(p_contender, contender_size);
    }
    barrier(&global_barrier);
    

    bench_worker_cut(p_buffer, p_contender, 16*KB);
    bench_worker_cut(p_buffer, p_contender, 128*KB);

    barrier(&global_barrier);

    bench_mem(p_buffer, p_contender);
    bench_rangestride(p_buffer);
}

void payload_benchmark_menu()
{
    int t, r;
    unsigned flag;

    menu_entry_t testmenu[] = {
        {1, "reconfig p_buffer"},
        {2, "reconfig p_contender"},
        {3, "hourglass"},
        {4, "bench_worker"},
        {5, "bench_worker_cut(16 kB)"},
        {6, "bench_mem"},
        {7, "bench_rangestride"},
        {999, "return"},
        {0,0}
    };
    menu_entry_t reconfmenu[] = {
        {1, "cache disable"},
        {2, "write back"},
        {3, "write through"},
        {999, "abort"},
        {0,0}
    };

    /*
     * Memory allocation
     */
    init_buffers();
    barrier(&global_barrier);

    do {
        t = menu("Benchmarks", testmenu);
        switch (t) {
            case 1 :
                r = menu("p_buffer", reconfmenu);
                switch (r) {
                    case 1 : flag = MM_CACHE_DISABLE; break;
                    case 2 : flag = MM_WRITE_THROUGH; break;
                    case 3 : flag = 0; break;
                }
                heap_reconfig(p_buffer, buffer_size, flag);
                break;
            case 2 :
                r = menu("p_contender", reconfmenu);
                switch (r) {
                    case 1 : flag = MM_CACHE_DISABLE; break;
                    case 2 : flag = MM_WRITE_THROUGH; break;
                    case 3 : flag = 0; break;
                }
                heap_reconfig(p_contender, contender_size, flag);
                break;
            case 3 :
                bench_hourglass();
                break;
            case 4 :
                bench_worker(p_buffer, p_contender);
                break;
            case 5 :
                bench_worker_cut(p_buffer, p_contender, 16*KB);
                break;
            case 6 : 
                bench_mem(p_buffer, p_contender);
                break;
            case 7 : 
                bench_rangestride(p_buffer);
                break;
        }
    } while (t != 999);

}

void payload_demo()
{
    barrier(&global_barrier);

    /* needs at least two CPUs */
    if (cpu_online >= 2) {
        if (CPU_ID == 0) {
            /* call Task for CPU 0 */
            barrier(&global_barrier);

            printf("CPU 0: udelay 5 Sek.\n");
            udelay(5*1000*1000);
            printf("CPU 0: exit now\n");
        } else if (CPU_ID == 1) {
            /* call Task for CPU 1 */
            barrier(&global_barrier);

            printf("CPU 1: udelay 10 Sek.\n");
            udelay(10*1000*1000);
            printf("CPU 1: exit now\n");
        } 
    } else {
        printf("only one CPU active, this task needs at least two.\n");
    }
}

