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

void hourglass(unsigned sec)
{
    uint64_t tsc, tsc_last, tsc_end, diff;
    unsigned long min = 0xFFFFFFFF, avg = 0, cnt = 0, max = 0;
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
        avg += diff;
        cnt++;
#       if 0  //RECORD_GAPS
        if (diff > GAP_THRESHOLD && i < COUNT_GAPS-1) {
            benchdata.hourglass.gaps[++i].start = tsc_last;
            benchdata.hourglass.gaps[i].len = diff;
        }
#       endif  // RECORD_GAPS
    }
    avg /= cnt;

    //p_min = (1000*min/avg)-1000;
    //p_max = (1000*max/avg)-1000;
    printf("[%u] %u sec hourglass: cnt : min/avg/max %u : %u/%u/%u " /* "[%i.%i:%i.%i]" */ "\n", 
            my_cpu_info()->cpu_id, sec, 
            cnt, min, avg, max /* , p_min/10, abs(p_min)%10, p_max/10, abs(p_max)%10 */ );
}

/*
 * Do range-stride benchmark for a combination of range/stride.
 * Needs a buffer, that is page-aligned and at least range bytes in size.
 */
typedef char access_type;
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
