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


stack_t stack[MAX_CPU] __attribute__(( aligned(STACK_SIZE) ));

int smp_init(void)
{
    /* this is run before any other CPU (AP) is called */
    unsigned u;
    for (u=0; u<MAX_CPU; u++) {
        stack[u].info.cpu_id = u;
        //if (u<4) printf("stack[%u].info.cpu_id at 0x%x value: %u\n", u, &(stack[u].info.cpu_id), stack[u].info.cpu_id);
        mutex_init(&(stack[u].info.wakelock));  // state: unlocked
    }
    return 0;
}


void smp_status(char c)
{
    status_putch(6+my_cpu_info()->cpu_id, c);
}


