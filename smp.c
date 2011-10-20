/*
 * =====================================================================================
 *
 *       Filename:  smp.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 13:23:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "smp.h"
#include "system.h"


stack_t stack[MAX_CPU] __attribute__(( aligned(STACK_FRAMES*4096) ));

int smp_init(void)
{
    int i;
    for (i=0; i<MAX_CPU; i++) {
        stack[i].info.cpu_id = i;
    }
    return 0;
}


void smp_status(char c)
{
    status_putch(6+my_cpu_info()->cpu_id, c);
}


