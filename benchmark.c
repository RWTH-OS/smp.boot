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

void hourglass(unsigned sec)
{
    uint64_t tsc, tsc_last, tsc_end, diff;
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
    tsc = rdtsc();
    tsc_end = tsc + sec * 1000000ull * hw_info.tsc_per_usec;
    while (tsc < tsc_end) {
        tsc_last = tsc;
        tsc = rdtsc();

        diff = tsc-tsc_last;
        if (diff < min) min = diff;
        if (diff > max) max = diff;
        avg += diff;        /* TODO: avg need not to be added up; it's just (tsc_end-tsc_start) ! */
        cnt++;

#       if 0  //RECORD_GAPS
        if (diff > GAP_THRESHOLD && i < COUNT_GAPS-1) {
            benchdata.hourglass.gaps[++i].start = tsc_last;
            benchdata.hourglass.gaps[i].len = diff;
        }
#       endif  // RECORD_GAPS
    }


#   if ! __x86_64__
    /*
     * in 32 bit mode:
     * shift avg (sum of gaps) to the right, until it fits in 32 bits
     */
    unsigned shift = 0;
    while (avg > 0xFFFFFFFF) {
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
    printf("[%u] cnt : min/avg/max %8u : %u/%u/%u " /* "[%i.%i:%i.%i]" */ "\n", 
            my_cpu_info()->cpu_id,
            (ptr_t)cnt, (ptr_t)min, (ptr_t)avg, (ptr_t)max /* , p_min/10, abs(p_min)%10, p_max/10, abs(p_max)%10 */ );
}

void load_until_flag(void *buffer, size_t size, size_t stride, flag_t *flag)
{
    typedef ptr_t mytype;
    size_t s;
    mytype *p = buffer;

    while (!flag_trywait(flag)) {
        for (s=0; s<size/sizeof(mytype); s+=(stride/sizeof(mytype))) {
            p[s]++;              /* read/write */
        }
    }

}

/*
 * Do range-stride benchmark for a combination of range/stride.
 * Needs a buffer, that is page-aligned and at least range bytes in size.
 */
typedef char access_type;

//#pragma GCC diagnostic ignored "-Wno-pragmas"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
uint64_t range_stride(void *buffer, size_t range, size_t stride)
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
    /* calculate count of repeats to be of constant time (decreasing with increasing number of accesses in range) */
    for (i=0; i<256*1024*1024/(range/stride); i++) {
        for (r=0; r < range; r += stride) {
            d = data[r];
        }
    }
    return (rdtsc()-tsc)/(256*1024*1024);
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

    tsc = tsc_start = rdtsc();
    tsc_end = tsc_start + sec * 1000000ull * hw_info.tsc_per_usec;

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
#   if __x86_64__
    avg = (tsc-tsc_start)/cnt;
#   else
    /*
     * no 64 bit division in 32 bit mode: shift right, divide in 32 bit, shift back left
     */ 
    unsigned shift = 0;
    avg = tsc - tsc_start;
    while (avg > 0xFFFFffff) {
        avg >>= 1;
        shift++;
    }
    avg = (unsigned long)avg/(unsigned long)cnt;
    avg >>= shift;
#   endif
    /*printf("t%ur%us%u : min/avg/max : %u/%u/%u\n", 
            (unsigned long)type, (unsigned long)range, (unsigned long)stride, 
            (unsigned long)min, (unsigned long)avg, (unsigned long)max);*/
    printf("%u/%u/%u ", (unsigned long)min, (unsigned long)avg, (unsigned long)max);
}

