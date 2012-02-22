/*
 * =====================================================================================
 *
 *       Filename:  smm.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 16:06:18
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef SMM_H
#define SMM_H

#define SMM_DEFAULT 0xDEADBEEF

void smm_init(void);
uint32_t smm_deactivate(void);
uint32_t smm_restore(uint32_t value);

#endif // SMM_H
