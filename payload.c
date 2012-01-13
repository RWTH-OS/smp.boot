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

extern volatile unsigned cpu_online;

/*
 * The payload is called by all CPUs after complete initialization.
 * A Barrier is executed immediately before, so they should come in shortly.
 */

static mutex_t mut = MUTEX_INITIALIZER;
static barrier_t barr = BARRIER_INITIALIZER(MAX_CPU+1); // this barrier will be set to cpu_online
static barrier_t barr2 = BARRIER_INITIALIZER(2);        // barrier for two
static flag_t flag = FLAG_INITIALIZER;

void payload_benchmark()
{
    unsigned myid = my_cpu_info()->cpu_id;
    mutex_lock(&mut);
    if (barr.max == MAX_CPU+1) {
        /* first one sets barr.max to the actual count of CPUs */
        barr.max = cpu_online;
        smm_deactivate();       // ...and (try to) deactivate SMM
    }
    mutex_unlock(&mut);

    size_t buffer_size = 16 * MB;
    static void *p_buffer = NULL;

    if (myid == 0) {
        p_buffer = heap_alloc(buffer_size / PAGE_SIZE);       // one page = 4kB
        /* no need for pre-faulting, because pages are present after head_alloc()
         * but initialize them */
        memset(p_buffer, 0, buffer_size);
    }

    size_t contender_size = 16 * MB;
    static void *p_contender = NULL;

    if (myid == 0) {
        p_contender = heap_alloc(contender_size / PAGE_SIZE);       // one page = 4kB
        //virt_to_phys(p_contender);
        //p_contender[0] = 42;
        //printf("[1] p_contender = 0x%x .. 0x%x\n", (ptr_t)p_contender, (ptr_t)p_contender+16*1024*1024);
        memset(p_contender, 0, contender_size);
    }

    barrier(&barr);

    if (myid == 0) printf("1 CPU hourglass (%u sec) ----------------------------------------------\n", BENCH_HOURGLAS_SEC);

    barrier(&barr);
    if (myid != 0) {
        smp_halt();
    } else {
        unsigned u;
        udelay(1000000);
        printf("others halt         ");
        hourglass(BENCH_HOURGLAS_SEC);
        for (u = 1; u<cpu_online; u++) {
            smp_wakeup(u);
        }
    }


#if 0
    if (cpu_online > 1) {
        if (myid == 0) printf("2 CPUs hourglass (%u sec) ---------------------------------------------\n", BENCH_HOURGLAS_SEC);
        barrier(&barr);

        if (myid <= 1) {    /* IDs 0 and 1 */
            unsigned u;

            barrier(&barr2);
            for (u=0; u<4; u++) {
                size_t size;
                if (myid == 1) {
                    switch (u) {        /* Cache Ranges valid for xaxis, Core i7 */
                        case 0: size=8*1024; break;         /* fits into L1 Cache */
                        case 1: size=128*1024; break;       /* fits into L2 Cache */
                        case 2: size=4*1024*1024; break;    /* fits into L3 Cache */
                        case 3: size=16*1024*1024; break;   /* larger than Cache */
                    }
                    printf("[1] Range %5u kB: ", size/1024);
                }
                barrier(&barr2);

                if (myid == 0) {
                    hourglass(BENCH_HOURGLAS_SEC);
                    flag_signal(&flag);
                } else {
                    load_until_flag(p_contender, size, &flag);
                }

                barrier(&barr2);
            }
            if (myid == 0) {
                for (u = 2; u<cpu_online; u++) {
                    smp_wakeup(u);
                }
            }
        } else {
            smp_halt();
        }
    }

    if (cpu_online > 4) {     /* assuming, that the upper half of cores are hyperthreads */
        if (myid == 0) printf("2 CPUs hourglass (hyper-threads) (%u sec) -----------------------------\n", BENCH_HOURGLAS_SEC);
        barrier(&barr);

        if (myid == 0) {
            unsigned u;
            hourglass(BENCH_HOURGLAS_SEC);
            for (u = 1; u<cpu_online; u++) {
                if (u != cpu_online/2) smp_wakeup(u);
            }
        } else if (myid == cpu_online/2) { 
            hourglass(BENCH_HOURGLAS_SEC);
        } else {
            smp_halt();
        }
    }
#endif

    barrier(&barr);

    if (cpu_online > 1) {
        volatile unsigned long *p_buffer = NULL;
        size_t worker_range, worker_stride, load_range, load_stride;
        unsigned load_nbr;
        access_t worker_atype;
        static barrier_t barr_dyn = BARRIER_INITIALIZER(MAX_CPU);

        if (myid == 0) printf("Worker benchmark (%u sec) -----------------------------\n", BENCH_HOURGLAS_SEC);

        /*
         * need some buffers with different cache strategies:
         *  - no cache
         *  - write-through
         *  - write-back
         * Or a way to change attributes for a range of pages...
         * (don't forget to flush the TLB after changing!)
         */

        /*
         * repeat benchmark for these dimensions:
         *   - cache (no, write-through, write-back)
         *   - range (<L1, <L2, <L3, >L3)
         *   - stride (only cache line size: 64)
         *   - access type (read, write, update, atomic)
         *   - use huge-pages (to avoid TLB effects), 4k pages (with TLB effects)
         *   - worker(s) on same/other pages as/than load
         *   - concurrent atomic access to one (or some) variables (lock-free, but contended)
         *
         * results:
         *   - min, avg, max
         */

        unsigned load_nbrs[] = {0, 1, 3};
        //size_t load_ranges[] = {4*KB, 256*KB, 4*MB, 16*MB};
        size_t load_ranges[] = {4*KB, 4*MB, 16*MB};
        size_t load_strides[]= {64};
        size_t worker_ranges[]  = {4*KB, 256*KB, 16*MB};
        size_t worker_strides[] = {64};
        //access_t worker_atypes[] = {AT_READ, AT_WRITE, AT_UPDATE, AT_ATOMIC};
        access_t worker_atypes[] = {AT_UPDATE};
        static flag_t flags[MAX_CPU];

        for (unsigned u=0; u<MAX_CPU; u++) flag_init(&flags[u]);

        foreach (load_nbr, load_nbrs) {
            if (load_nbr >= cpu_online) { break; }

            barrier(&barr);
            if (myid == 0) barr_dyn.max = load_nbr+1;
            barrier(&barr);

            if (myid > load_nbr) smp_halt();
            else {
                foreach (load_range, load_ranges) {
                    foreach (load_stride, load_strides) {
                        if (myid==0) printf("== %u load(s) on range %5u kB (stride %u) [rd, wr, upd, atomic]\n", 
                                load_nbr, load_range>>10, load_stride);
                        if (myid == 0) {
                            /*
                             * start worker (benchmark)
                             */
                            udelay(100*1000);
                            foreach (worker_range, worker_ranges) {
                                foreach (worker_stride, worker_strides) {
                                    //printf("---- bench: range: %x (stride %u) (rd, wr, upd, atomic) \n", 
                                    //              worker_range, worker_stride, worker_atype);
                                    printf("r %5u kB: ", worker_range>>10);
                                    foreach (worker_atype, worker_atypes) {
                                        /* 
                                         * start benchmark 
                                         */
                                        worker(p_buffer, worker_range, worker_stride, worker_atype, BENCH_HOURGLAS_SEC);
                                    }
                                    printf("\n");
                                }
                            }
                            for (unsigned u=1; u<=load_nbr; u++) flag_signal(&flags[u]);
                        } else {
                            /*
                             * start load
                             */
                            load_until_flag(p_contender, load_range, load_stride, &flags[myid]);
                        }
                        barrier(&barr_dyn);
                        if (load_nbr == 0) goto label_break;
                    }
                }
label_break:
                if (myid == 0) {
                    unsigned u;
                    for (u=load_nbr+1; u<cpu_online; u++) smp_wakeup(u);
                }
            }
        }



#if 0
        if (myid == 0) {
            p_buffer = heap_alloc(16*1024*1024 / PAGE_SIZE);       // one page = 4kB

            worker(p_buffer,       8*1024, 64, AT_READ, BENCH_HOURGLAS_SEC);
            worker(p_buffer,     265*1024, 64, AT_READ, BENCH_HOURGLAS_SEC);
            worker(p_buffer, 16*1024*1024, 64, AT_READ, BENCH_HOURGLAS_SEC);

            worker(p_buffer,       8*1024, 64, AT_WRITE, BENCH_HOURGLAS_SEC);
            worker(p_buffer,     265*1024, 64, AT_WRITE, BENCH_HOURGLAS_SEC);
            worker(p_buffer, 16*1024*1024, 64, AT_WRITE, BENCH_HOURGLAS_SEC);

            worker(p_buffer,       8*1024, 64, AT_UPDATE, BENCH_HOURGLAS_SEC);
            worker(p_buffer,     265*1024, 64, AT_UPDATE, BENCH_HOURGLAS_SEC);
            worker(p_buffer, 16*1024*1024, 64, AT_UPDATE, BENCH_HOURGLAS_SEC);

            worker(p_buffer,       8*1024, 64, AT_ATOMIC, BENCH_HOURGLAS_SEC);
            worker(p_buffer,     265*1024, 64, AT_ATOMIC, BENCH_HOURGLAS_SEC);
            worker(p_buffer, 16*1024*1024, 64, AT_ATOMIC, BENCH_HOURGLAS_SEC);
        }
#endif

        barrier(&barr);
    }





    /*
     * allocate Buffer for memory benchmarks
     */
    if (myid == 0) {
        size_t i, j;
        unsigned u;
        //size_t max_range = (1 << BENCH_MAX_RANGE_POW2); // 2^25 = 32 MB


        printf("Range/Stride (other CPUs in halt-state) ----------------\n");
        printf("str.|range%4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s\n", 
                "4k", "8k", "16k", "32k", "64k", "128k", "256k", "512k", "1M", "2M", "4M", "8M", "16M", "32M");
        for (i=BENCH_MIN_STRIDE_POW2; i<=BENCH_MAX_STRIDE_POW2; i++) {                      /* stride */
            printf("%3u      ", (1<<i));
            for (j=BENCH_MIN_RANGE_POW2; j<=BENCH_MAX_RANGE_POW2; j++) {                /* range */
                unsigned long ret = range_stride(p_buffer, (1<<j), (1<<i));
                printf(" %4u", ret);
            }
            printf("\n");
        }
        
        if (cpu_online > 1) {
            unsigned u;
            smp_wakeup(1);
            /*
             * redo benchmark while CPU(1) is working, too
             */

            barrier(&barr2);
            for (u=0; u<4; u++) {

                barrier(&barr2);
                for (i=BENCH_MIN_STRIDE_POW2; i<=BENCH_MAX_STRIDE_POW2; i++) {                      /* stride */
                    printf("%3u      ", (1<<i));
                    for (j=BENCH_MIN_RANGE_POW2; j<=BENCH_MAX_RANGE_POW2; j++) {                /* range */
                        unsigned long ret = range_stride(p_buffer, (1<<j), (1<<i));
                        printf(" %4u", ret);
                    }
                    printf("\n");
                }

                flag_signal(&flag);
                barrier(&barr2);
            }

        }

        for (u = 1; u<cpu_online; u++) {
            smp_wakeup(u);
        }
    } else {
        smp_halt();

        if (myid == 1) {
            /*
             * do some work on different memory ranges to spill caches
             */
            unsigned u;

            barrier(&barr2);
            for (u=0; u<4; u++) {
                size_t size;
                switch (u) {        /* Cache Ranges valid for xaxis, Core i7 */
                    case 0: size=8*1024; break;         /* fits into L1 Cache */
                    case 1: size=128*1024; break;       /* fits into L2 Cache */
                    case 2: size=4*1024*1024; break;    /* fits into L3 Cache */
                    case 3: size=16*1024*1024; break;   /* larger than Cache */
                }
                printf("Range/Stride (one CPU working on %u kB) --------------\n", size/1024);
                barrier(&barr2);

                load_until_flag(p_contender, size, 64, &flag);

                barrier(&barr2);
            }


        }
    }
    barrier(&barr);

}

void payload_demo()
{
    unsigned myid = my_cpu_info()->cpu_id;
    mutex_lock(&mut);
    if (barr.max == MAX_CPU+1) {
        /* first one sets barr.max to the actual count of CPUs */
        barr.max = cpu_online;
    }
    mutex_unlock(&mut);

    /* needs at least two CPUs */
    if (cpu_online >= 2) {
        if (myid == 0) {
            /* call Task for CPU 0 */
            barrier(&barr);

            printf("CPU 0: udelay 5 Sek.\n");
            udelay(5*1000*1000);
            printf("CPU 0: exit now\n");
        } else if (myid == 1) {
            /* call Task for CPU 1 */
            barrier(&barr);

            printf("CPU 1: udelay 10 Sek.\n");
            udelay(10*1000*1000);
            printf("CPU 1: exit now\n");
        } 
    } else {
        printf("only one CPU active, this task needs at least two.\n");
    }
}

