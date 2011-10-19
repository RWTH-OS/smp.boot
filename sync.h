/*
 * =====================================================================================
 *
 *       Filename:  sync.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 16:11:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef SYNC_H
#define SYNC_H

typedef volatile int mutex_t;
#define MUTEX_INITIALIZER   (mutex_t)1
void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
int mutex_trylock(mutex_t *m);
void mutex_unlock(mutex_t *m);

typedef struct {
    volatile unsigned cnt;
    volatile unsigned epoch;
    unsigned max;
} barrier_t;
#define BARRIER_INITIALIZER(max)  (barrier_t){ .cnt=0, .epoch=0, .max=max };
void barrier_init(barrier_t *b, int max);
void barrier(barrier_t *b);



#endif  // SYNC_H

