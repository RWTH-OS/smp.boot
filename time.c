/*
 * =====================================================================================
 *
 *       Filename:  time.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 13:52:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "info.h"
#include "smp.h"

void udelay(unsigned long us)
{
    uint64_t tsc_now, tsc_end;
    smp_status('u');
    tsc_now = rdtsc();
    //printf("tsc %lu\n", tsc_now.u64);
    tsc_end = tsc_now + (us * hw_info.tsc_per_usec);
    while (tsc_now < tsc_end) {
        tsc_now = rdtsc();
        //printf("tsc %lu\n", tsc_now.u64);
    }
    smp_status('.');
}

