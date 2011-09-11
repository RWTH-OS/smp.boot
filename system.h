#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stddef.h>
#include "config.h"

/* Verbosity: if not defined here, modules may define their own */
//#define VERBOSE 0


#if __x86_64__
struct regs
{
	unsigned long long cr3, gs, fs, es, ds;				/* pushed the segs last */
	unsigned long long r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, _zero, rbx, rdx, rcx, rax ;
	unsigned long long int_no, err_code;			/* our 'push byte #' and ecodes do this */
	unsigned long long rip, cs, rflags, rsp, ss;			/* pushed by the processor automatically */
} __attribute__((packed));
#else
struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cx, eflags, useresp, ss;
};
#endif

inline static uint64_t rdtsc(void)
{
	union {
		uint64_t u64;
		uint32_t u32[2];
	} x;
	asm volatile ("rdtsc" : "=a" (x.u32[0]), "=d"(x.u32[1]));
	return x.u64;
}

/* main.c */
void *memcpy(void *dest, const void *src, int count);
void *memset(void *dest, int val, int count);
unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);
int strlen(const char *str);

/* lib.c */
unsigned char inportb (unsigned short _port);
void outportb (unsigned short _port, unsigned char _data);
void udelay(unsigned long us);
void halt();


#define NULL ((void*)0)

/* idt.c */
void idt_install();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

/* isr.c */
void isr_install();

/* apic.c */
void apic_init();

/* scrn.c */
void cls();
void putch(char c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();
void itoa (char *buf, int base, long d);
void printf (const char *format, ...);


#endif
