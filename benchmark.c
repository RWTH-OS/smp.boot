/*
 * =====================================================================================
 *
 *       Filename:  benchmark.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:19:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "stddef.h"
#include "system.h"
#include "info.h"
#include "time.h"
#include "smp.h"
#include "sync.h"
#include "benchmark.h"
#include "perfcount.h"

extern volatile unsigned cpu_online;

bench_opt_t bench_opt = {
    .cm_buffer = cm_write_back,
    .cm_contender = cm_write_back,
    .timebase = BENCH_HOURGLASS_SEC
};

void hourglass(unsigned sec)
{
    uint64_t tsc, tsc_last, tsc_start, tsc_end, diff;
    unsigned long long min = 0xFFFFFFFF, avg = 0, cnt = 0, max = 0;
    //int i = -1, j;
    //long p_min, p_max;

    /*
     * hourglass (warm-up)
     */
    tsc = rdtsc();
    tsc_end = tsc + 1 * 1000000ull * hw_info.tsc_per_usec;
    while (tsc < tsc_end) {
        tsc_last = tsc;
        tsc = rdtsc();

        diff = tsc-tsc_last;
        if (diff < min) min = diff;
        if (diff > max) max = diff;
        avg += diff;
        cnt++;
    }


    /*
     * hourglass (now counting)
     */
    min = 0xFFFFFFFF, avg = 0; cnt = 0; max = 0;
    tsc_start = tsc = rdtsc();
    tsc_end = tsc + sec * 1000000ull * hw_info.tsc_per_usec;
    while (tsc < tsc_end) {
        tsc_last = tsc;
        tsc = rdtsc();

        diff = tsc-tsc_last;
        if (diff < min) min = diff;
        if (diff > max) max = diff;
        cnt++;

#       if 0  //RECORD_GAPS
        if (diff > GAP_THRESHOLD && i < COUNT_GAPS-1) {
            benchdata.hourglass.gaps[++i].start = tsc_last;
            benchdata.hourglass.gaps[i].len = diff;
        }
#       endif  // RECORD_GAPS
    }

    avg = tsc - tsc_start;

#   if ! __x86_64__
    /*
     * in 32 bit mode:
     * shift avg (sum of gaps) to the right, until it fits in 32 bits
     */
    unsigned shift = 0;
    while (avg > 0xFFFFFFFFull) {
        avg >>= 1;
        shift++;
    }
#   endif

    /* unsigned long: 32 or 64 bits (native size, cf. ptr_t) */
    avg = (unsigned long)avg / (unsigned long)cnt;

#   if ! __x86_64__
    /*
     * in 32 bit mode:
     * we had a division by 2^shift before the division by cnt,
     * undo this by a left-shift (multiply result with 2^shift).
     * This reduces the precision, but avoids a 64 bit division in 32 bit kernel (requiring a builtin).
     */
    avg <<= shift;
#   endif

    //p_min = (1000*min/avg)-1000;
    //p_max = (1000*max/avg)-1000;
    printf("[%u] cnt : min/avg/max %8u : %u/%u/%u " /* "[%i.%i:%i.%i]" */ , 
            my_cpu_info()->cpu_id,
            (ptr_t)cnt, (ptr_t)min, (ptr_t)avg, (ptr_t)max /* , p_min/10, abs(p_min)%10, p_max/10, abs(p_max)%10 */ );
}

void load_until_flag(void *buffer, size_t size, size_t stride, flag_t *flag)
{
    typedef ptr_t mytype;
    size_t s;
    mytype *p = buffer;
    uint64_t t1, t2, cnt=0;
    //uint64_t pc_cache;

    perfcount_reset(0);
    perfcount_start(0);
    t1 = rdtsc();
    while (!flag_trywait(flag)) {
        for (s=0; s<size/sizeof(mytype); s+=(stride/sizeof(mytype))) {
            p[s]++;              /* read/write */
            cnt += sizeof(mytype);
        }
    }
    t2 = rdtsc();
    perfcount_stop(0);
    printf(" [%#uB/s %u]", (((cnt*hw_info.tsc_per_usec*1000)/(t2-t1))*1000u)&~0xFFFFF, perfcount_read(0));
    // round last 20 bit (set to zero) so that %#u can work

}

/*
 * Do range-stride benchmark for a combination of range/stride.
 * Needs a buffer, that is page-aligned and at least range bytes in size.
 */
typedef char access_type;

//#pragma GCC diagnostic ignored "-Wno-pragmas"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
uint64_t range_stride(void *buffer, size_t range, size_t stride, uint64_t *p_pc0)
{
    size_t r, i;
    volatile access_type d;
    volatile access_type *data = (volatile access_type*)buffer;
    uint64_t tsc;

    stride /= sizeof(access_type);

    /* warm-up */
    for (i=0; i<4; i++) {
        for (r=0; r < range; r += stride) {
            d = data[r];
        }
    }
    
    tsc = rdtsc();
    perfcount_reset(0);
    perfcount_start(0);
    /* calculate count of repeats to be of approx. constant time (decreasing with increasing number of accesses in range) */
    for (i=0; i<BENCH_RANGESTRIDE_REP/(range/stride); i++) {
        for (r=0; r < range; r += stride) {
            d = data[r];
        }
    }
    perfcount_stop(0);
    if (p_pc0 != NULL) *p_pc0 = perfcount_read(0)/(BENCH_RANGESTRIDE_REP/(range/stride));
    return (rdtsc()-tsc)/(BENCH_RANGESTRIDE_REP);
}

#if __x86_64__
#define RAX   "rax"
#else
#define RAX   "eax"
#endif

void worker(volatile unsigned long *p_buffer, size_t range, size_t stride, access_t type, unsigned sec)
{
    uint64_t tsc, tsc_last, tsc_start, tsc_end, diff, min = 0xFFFFffffFFFFffff, max = 0, avg, cnt = 0;
    volatile unsigned long *p = p_buffer;
    volatile unsigned long dummy;
    uint64_t pc_l2;

    tsc = tsc_start = rdtsc();
    tsc_end = tsc_start + sec * 1000000ull * hw_info.tsc_per_usec;

    perfcount_reset(0);
    perfcount_start(0);

    while (tsc < tsc_end) {
        tsc_last = tsc;
        tsc = rdtsc();

        diff = tsc - tsc_last;
        if (diff < min) min = diff;
        if (diff > max) max = diff;
        cnt++;

        //if (type == AT_WRITE) printf("|%x", p);

        if (p_buffer != NULL) {
            switch (type) {
                case AT_READ :
                    __asm__ volatile ("mov %0, %%" RAX : "=a"(dummy) : "m"(*p));
                    //dummy = *p;
                    break;
                case AT_WRITE :
                    __asm__ volatile ("mov %%" RAX ", %0" : "=m"(*p) : "a"(dummy));
                    //*p = dummy;
                    break;
                case AT_UPDATE :
                    *p += dummy;
                    break;
                case AT_ATOMIC :
                    dummy = __sync_add_and_fetch(p, 1);
                    break;
            }

            p += stride/sizeof(unsigned long);
            if ((ptr_t)p >= (ptr_t)p_buffer+range) p = p_buffer;
        }
    }

    perfcount_stop(0);

    avg = (tsc-tsc_start)/cnt;
    // with lib.c providing __udivdi3(), 64 bit division can be done in 32 bit mode.

    /*printf("t%ur%us%u : min/avg/max : %u/%u/%u\n", 
            (unsigned long)type, (unsigned long)range, (unsigned long)stride, 
            (unsigned long)min, (unsigned long)avg, (unsigned long)max);*/

    pc_l2 = perfcount_read(0);
    printf("%u/%u/%u [L2$:%u] ", (unsigned long)min, (unsigned long)avg, (unsigned long)max, pc_l2);
}





void bench_hourglass()
{

    if (CPU_ID == 0) printf("1 CPU hourglass (%u sec) ----------------------------------------------\n", bench_opt.timebase);

    barrier(&global_barrier);
    if (collective_only(0x0001)) {

        udelay(1000000);
        printf("others halt         ");
        hourglass(bench_opt.timebase);
        printf("\n");

        collective_end();
    }
}


void bench_hourglass_worker(void *p_contender)
{
    static barrier_t barr2 = BARRIER_INITIALIZER(2);        // barrier for two
    static flag_t flag = FLAG_INITIALIZER;

    if (cpu_online > 1) {
        if (CPU_ID == 0) printf("2 CPUs hourglass (%u sec) ---------------------------------------------\n", bench_opt.timebase);
        barrier(&global_barrier);

        if (collective_only(0x0003)) {   /* IDs 0 and 1 */

            unsigned u;

            barrier(&barr2);
            for (u=0; u<12; u++) {
                size_t size = 1;
                if (CPU_ID == 1) {
                    switch (u) {        /* Cache Ranges valid for xaxis, Core i7 */
                        case  0: size=16*1024; break;        /* fits into L1 Cache */
                        case  1: size=128*1024; break;       /* fits into L2 Cache */
                        case  2: size=256*1024; break;       /* fits into L2 Cache */
                        case  3: size=512*1024; break;       /* fits into L2 Cache */
                        case  4: size=1*1024*1024; break;    /* fits into L3 Cache */
                        case  5: size=2*1024*1024; break;    /* fits into L3 Cache */
                        case  6: size=3*1024*1024; break;    /* fits into L3 Cache */
                        case  7: size=4*1024*1024; break;    /* fits into L3 Cache */
                        case  8: size=5*1024*1024; break;    /* fits into L3 Cache */
                        case  9: size=6*1024*1024; break;    /* fits into L3 Cache */
                        case 10: size=8*1024*1024; break;    /* fits into L3 Cache */
                        case 11: size=16*1024*1024; break;   /* larger than Cache */
                    }
                    printf("[1] Range %#uB: ", size);
                }
                barrier(&barr2);

                if (CPU_ID == 0) {
                    hourglass(bench_opt.timebase);
                    flag_signal(&flag);
                } else {
                    load_until_flag(p_contender, size, 32, &flag);
                    printf("\n");
                }

                barrier(&barr2);
            }

            collective_end();
        }
    }
}

void bench_hourglass_hyperthread()
{

    if (cpu_online > 4) {     /* assuming, that the upper half of cores are hyperthreads */
        if (CPU_ID == 0) printf("2 CPUs hourglass (hyper-threads) (%u sec) -----------------------------\n", bench_opt.timebase);
        barrier(&global_barrier);

        if (collective_only(0x0001 | (1 << (cpu_online/2)))) {
            if (CPU_ID == 0) {
                hourglass(bench_opt.timebase);
                printf("\n");
            } else { 
                hourglass(bench_opt.timebase);
                printf("\n");
            }
            collective_end();
        }
    }
}

void bench_worker(void *p_buffer, void *p_contender)
{
    unsigned myid = my_cpu_info()->cpu_id;

    if (cpu_online > 1) {
        //volatile unsigned long *p_buffer = p_buf;
        size_t worker_range, worker_stride, load_range, load_stride;
        unsigned load_nbr;
        access_t worker_atype;
        static barrier_t barr_dyn = BARRIER_INITIALIZER(MAX_CPU);

        if (myid == 0) printf("Worker benchmark (%u sec) -----------------------------\n", bench_opt.timebase);

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


        unsigned load_nbrs[] = {0, 1, 3, 7};
        size_t load_ranges[] = {16*KB, 128*KB, 4*MB, 16*MB};
        size_t load_strides[]= {64};
        size_t worker_ranges[]  = {16*KB, 128*KB, 4*MB, 16*MB};
        size_t worker_strides[] = {64};
        access_t worker_atypes[] = {AT_READ, AT_WRITE, AT_UPDATE, AT_ATOMIC};
        static flag_t flags[MAX_CPU];

        for (unsigned u=0; u<MAX_CPU; u++) flag_init(&flags[u]);

        foreach (load_nbr, load_nbrs) {
            if (load_nbr >= cpu_online) { break; }

            barrier(&global_barrier);
            if (CPU_ID == 0) barr_dyn.max = load_nbr+1;
            barrier(&global_barrier);

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
                                    printf("r %#uB: ", worker_range);
                                    foreach (worker_atype, worker_atypes) {
                                        /* 
                                         * start benchmark 
                                         */
                                        worker(p_buffer, worker_range, worker_stride, worker_atype, bench_opt.timebase);
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

        barrier(&global_barrier);
    }


}


void bench_worker_cut(void *p_buffer, void *p_contender, size_t worker_size)
{
    unsigned myid = my_cpu_info()->cpu_id;
    static barrier_t barr2 = BARRIER_INITIALIZER(2);        // barrier for two

    /*
     * similar worker-benchmark, but different cuts through the parameter dimensions.
     */
    if (cpu_online > 1) {
        if (myid==0)  {
            printf("1 worker on range %#uB, load on range 16 kB .. 16 MB -------------------\n", worker_size);
        } else {
            perfcount_init(0, PERFCOUNT_L2); 
        }

        if (collective_only(0x0003)) {
            unsigned r;
            static flag_t flag = FLAG_INITIALIZER;

            if (myid == 0) {
                perfcount_init(0, PERFCOUNT_L1DATA); 
                printf("warm-up      : ");
                worker(p_buffer, worker_size, 32, AT_UPDATE, 1);
                printf("\n");
            }
            for (r = 16*KB; r <= 16*MB; r = (r>=512*KB && r< 8*MB)?r+512*KB:r*2) {
                barrier(&barr2);
                if (myid == 0) {
                    /* benchmark/worker */
                    udelay(10*1000);

                    printf("load %#uB : ", r);
                    worker(p_buffer, worker_size, 32, AT_UPDATE, bench_opt.timebase);

                    flag_signal(&flag);
                } else {
                    /* load */
                    load_until_flag(p_contender, r, 32, &flag);
                    printf("\n");
                }
            }

            collective_end();
        }

    }
    barrier(&global_barrier);
}

void bench_rangestride(void *p_buffer)
{

    /*
     * memory benchmark
     */
    if (CPU_ID == 0) {
        size_t range, stride;
        unsigned u;
        uint64_t pc0;
        //size_t max_range = (1 << BENCH_MAX_RANGE_POW2); // 2^25 = 32 MB


        printf("Range/Stride (other CPUs in halt-state) ----------------\n");

        static const size_t ranges[]  = {16*KB, 32*KB, 64*KB, 128*KB, 256*KB, 512*KB, 1*MB, 2*MB, 3*MB, 4*MB, 6*MB, 8*MB, 12*MB, 16*MB};
        static const size_t strides[] = {32, 64, 128, 1*KB, 4*KB};

        perfcount_init(0, PERFCOUNT_L1DATA); 

        printf("       strides| ");
        foreach (stride, strides) {
            printf(" %#uB ", stride);
        } 
        printf("\n");
        printf("--------------|-");
        foreach (stride, strides) {
            printf("---------");
        } 
        printf("\n");
        foreach (range, ranges) {
            printf("range: %#uB|", range);
            foreach (stride, strides) {
                unsigned long ret = range_stride(p_buffer, range, stride, &pc0);
                printf(" %u %u", ret, pc0);
            }
            printf("\n");
        }

        for (u = 1; u<cpu_online; u++) {
            smp_wakeup(u);
        }
    } else {
        smp_halt();
    }
    barrier(&global_barrier);


}

void bench_mem(void *p_buffer, void *p_contender)
{
    static barrier_t barr2 = BARRIER_INITIALIZER(2);        // barrier for two
    static flag_t flag = FLAG_INITIALIZER;

    /*
     * memory benchmark
     */
    if (CPU_ID == 0) {
        size_t i, j;
        unsigned u;
        //size_t max_range = (1 << BENCH_MAX_RANGE_POW2); // 2^25 = 32 MB


        printf("Range/Stride (other CPUs in halt-state) ----------------\n");
        printf("str.|range%4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s %4s\n", 
                "4k", "8k", "16k", "32k", "64k", "128k", "256k", "512k", "1M", "2M", "4M", "8M", "16M", "32M");
        for (i=BENCH_MIN_STRIDE_POW2; i<=BENCH_MAX_STRIDE_POW2; i++) {                      /* stride */
            printf("%3u      ", (1<<i));
            for (j=BENCH_MIN_RANGE_POW2; j<=BENCH_MAX_RANGE_POW2; j++) {                /* range */
                unsigned long ret = range_stride(p_buffer, (1<<j), (1<<i), NULL);
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
                        unsigned long ret = range_stride(p_buffer, (1<<j), (1<<i), NULL);
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

        if (CPU_ID == 1) {
            /*
             * do some work on different memory ranges to spill caches
             */
            unsigned u;

            barrier(&barr2);
            for (u=0; u<4; u++) {
                size_t size = 16*KB;
                switch (u) {        /* Cache Ranges valid for xaxis, Core i7 */
                    case 0: size=16*1024; break;        /* fits into L1 Cache (32 kB per core) */
                    case 1: size=128*1024; break;       /* fits into L2 Cache (256 kB per core) */
                    case 2: size=4*1024*1024; break;    /* fits into L3 Cache (8 MB shared) */
                    case 3: size=16*1024*1024; break;   /* larger than Cache */
                }
                printf("Range/Stride (one CPU working on %#uB) --------------\n", size);
                barrier(&barr2);

                load_until_flag(p_contender, size, 64, &flag);

                barrier(&barr2);
            }


        }
    }
    barrier(&global_barrier);


}

