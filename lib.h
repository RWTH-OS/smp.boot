/*
 * =====================================================================================
 *
 *       Filename:  lib.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12.09.2011 13:43:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef LIB_H
#define LIB_H

// note: var and bit are evaluated only once!
#define BIT_SET(var, bit)       ((var) |=  (1 << (bit)))
#define BIT_CLEAR(var, bit)     ((var) &= ~(1 << (bit)))
#define MASK_SET(var, mask)       ((var) |=  (mask))
#define MASK_CLEAR(var, mask)     ((var) &= ~(mask))
#define IS_BIT_SET(var, bit)    (((var) & (1 << bit)) != 0)
#define IS_BIT_CLEAR(var, bit)  (((var) & (1 << bit)) == 0)
#define IS_MASK_SET(var, mask)    (((var) & (mask)) != 0)
#define IS_MASK_CLEAR(var, mask)  (((var) & (mask)) == 0)

void *memcpy(void *dest, const void *src, int count);
void *memset(void *dest, int val, int count);
unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);
int strlen(const char *str);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);
int atoi(const char *a);

int abs(int j);
long labs(long j);


#endif // LIB_H
