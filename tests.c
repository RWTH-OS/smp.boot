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

#define IFV   if (VERBOSE > 0 || VERBOSE_TESTS > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_TESTS > 1)

extern volatile unsigned cpu_online;

static barrier_t barr_all = BARRIER_INITIALIZER(MAX_CPU+1);

void tests_barrier(void)
{
    unsigned myid = my_cpu_info()->cpu_id;
    unsigned u;
    static barrier_t barr2 = BARRIER_INITIALIZER(2);

    if (myid < 2) {
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

void tests_doall(void)
{
    unsigned myid = my_cpu_info()->cpu_id;

    IFV printf("[%u] calling test_barrier()\n", myid);
    tests_barrier();

    printf("[%u] exit tests_doall()\n", myid);
}
