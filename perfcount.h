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

void perfcount_init_cache(void);
void perfcount_start(void);
void perfcount_stop(void);
void perfcount_reset(void);
unsigned long long perfcount_read(void);

#endif

