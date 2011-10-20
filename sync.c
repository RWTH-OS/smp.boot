/*
 * =====================================================================================
 *
 *       Filename:  sync.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 16:11:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "sync.h"
#include "smp.h"




void mutex_init(mutex_t *m)
{
    *m = MUTEX_INITIALIZER;
} 

void mutex_lock(mutex_t *m)
{
    smp_status('m');
    while (!__sync_bool_compare_and_swap(m, 1, 0)) {};
    smp_status('.');
}

int mutex_trylock(mutex_t *m)
{
    return (__sync_bool_compare_and_swap(m, 1, 0));
}

void mutex_unlock(mutex_t *m)
{
    *m = 1;
}



void barrier_init(barrier_t *b, int max)
{
    *b = BARRIER_INITIALIZER(max);
}

void barrier(barrier_t *b)
{
    unsigned e = b->epoch;
    /* epoch e ends, enter into episode. As no participant can leave the episode, every member gets the same e here */
    unsigned c = __sync_add_and_fetch(&(b->cnt), 1); 
    smp_status('b');
    /* every participant increments the counter and gets a unique ID into c */
    if (c == b->max) {
        /* last: become master for this episode: */
        /* reset counter (all others have already their ID and wait in the episode for the epoch to change)  */
        b->cnt = 0;
        /* and release the others by incrementing the epoch (this end the episode and start a new epoch) */
        b->epoch++;     /* overflow is no problem, because the others wait while the epoch is equal */
    } else {
        /* not last:  */
        /* wait for epoch to be incremented */
        while (e == b->epoch) {};
    }
    /* episode ends, next epoch (e+1) begins */
    smp_status('.');
}


void flag_init(flag_t *flag)
{
    *flag = FLAG_INITIALIZER;
}

void flag_signal(flag_t *flag)
{
    __sync_add_and_fetch(&flag->flag, 1);
}
      
void flag_wait(flag_t *flag)
{
    unsigned n = flag->next + 1;
    smp_status('f');
    while (flag->flag < n) {};
    __sync_add_and_fetch(&flag->next, 1);
    smp_status('.');
}

int flag_trywait(flag_t *flag)
{
    unsigned n = flag->next + 1;
    if (flag->flag < n) {
        return 0;
    } else {
        __sync_add_and_fetch(&flag->next, 1);
        return 1;
    }
}
      
