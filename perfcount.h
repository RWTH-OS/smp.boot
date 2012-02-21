/*
 * =====================================================================================
 *
 *       Filename:  perfcount.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14.02.2012 10:02:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Christian Spoo (cs) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef PERFCOUNT_H
#define PERFCOUNT_H

#include "stddef.h"

#define perfcount_init_cache() perfcount_init_l2_miss()
#warning "perfcount compatibility hack active"

void perfcount_init_l1d_miss(void);
void perfcount_init_l2_miss(void);
void perfcount_start(void);
void perfcount_stop(void);
void perfcount_reset(void);
uint64_t perfcount_read(void);

#endif

