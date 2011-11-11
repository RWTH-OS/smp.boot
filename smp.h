/*
 * =====================================================================================
 *
 *       Filename:  smp.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 13:23:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef SMP_H
#define SMP_H

#include "config.h"
#include "sync.h"

/*
 * per-cpu info structure
 */
typedef struct {
    unsigned cpu_id;     
    mutex_t wakelock;
} cpu_info_t;

/*
 * Stack (growing downwards with per-cpu info structure on opposite (lower) end, where the stack should never grow to)
 */
typedef union {
    unsigned stack[STACK_FRAMES * 4096 / sizeof(unsigned)];
    cpu_info_t info;
} stack_t; // __attribute__((packed));

extern stack_t stack[MAX_CPU];

int smp_init(void);

static inline cpu_info_t * my_cpu_info()
{
    void *p;
#   ifdef __x86_64__
    asm ("movq %%rsp, %%rax \n\t andq %%rdx, %%rax" : "=a"(p) : "d" ( ~(STACK_FRAMES*4096ul-1) ));
#   else
    asm ("movl %%esp, %%eax \n\t andl %%edx, %%eax" : "=a"(p) : "d" ( ~(STACK_FRAMES*4096ul-1) ));
#   endif
    return (cpu_info_t *)p;
}

void smp_status(char c);


#endif // SMP_H

