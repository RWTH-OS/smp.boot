#include "system.h"

/*
 * This file is used in 32-Bit Boot Code (REAL MODE) called from startXX.asm and boot32.c
 * as well as in the final kernel (main.c etc.)
 */

void *memcpy(void *dest, const void *src, int count)
{
    /* copy 'count' bytes of data from 'src' to 'dest', finally return 'dest' */
    char *dp = (char *)dest;
    char *sp = (char *)src;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void *memset(void *dest, int val, int count)
{
    /* set 'count' bytes in 'dest' to 'val'.  Again, return 'dest' */
    char *temp = (char *)dest;
    for( ; count != 0; count--) *temp++ = (char)val;
    return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, int count)
{
    /* Same as above, but this time, we're working with a 16-bit
    *  'val' and dest pointer. Your code can be an exact copy of
    *  the above, provided that your local variables if any, are
    *  unsigned short */
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

int strlen(const char *str)
{
    /* This loops through character array 'str', returning how
    *  many characters it needs to check before it finds a 0.
    *  In simple words, it returns the length in bytes of a string */
    int retval;
    for(retval = 0; *str != '\0'; str++) retval++;
    return retval;
}

int strcmp(const char *a, const char *b) 
{
    while (*a != 0 && *b != 0 && (*a == *b)) {
        a++;
        b++;
    }
    return (*a - *b);
}
int strncmp(const char *a, const char *b, int n) 
{
    while (n > 0 && *a != 0 && *b != 0 && (*a == *b)) {
        n--;
        a++;
        b++;
    }
    return n==0?0:(*a - *b);
}

/*
 * can handle decimal (not starting with 0), octal (starting with zero), hex (starting with 0x) and negative values (starting with -).
 */
int atoi(const char *a)
{
    int i = 0;
    int s = 1;
    int base = 10;
    if (*a == '-') {
        s = -1;
        a++;
    }
    if (*a == '0') {
        base = 8;
        a++;
        if (*a == 'x' || *a == 'X') {
            base = 16;
            a++;
        }
    }
    while ((*a >= '0' && *a <= '0'+(base>10?10:base)-1) || (base>10 && ((*a >= 'A' && *a <= 'A'+base-11) || (*a >= 'a' && *a <= 'a'+base-11)))) {
        i *= base;
        if (*a <= '9')
            i += *a - '0';
        else if (*a >= 'a')
            i += *a - 'a'+10;
        else
            i += *a - 'A'+10;
        a++;
    }
    i *= s;
    return i;
}

/* We will use this later on for reading from the I/O ports to get data
*  from devices such as the keyboard. We are using what is called
*  'inline assembly' in these routines to actually do the work */
unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}



void halt()
{
    printf("System halted.");
    while (1) {
        asm volatile ("hlt");
    }
}

