#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stddef.h>

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


/* main.c */
void *memcpy(void *dest, const void *src, int count);
void *memset(void *dest, int val, int count);
unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);
int strlen(const char *str);

/* lib.c */
unsigned char inportb (unsigned short _port);
void outportb (unsigned short _port, unsigned char _data);
void halt();


#define NULL ((void*)0)

/* idt.c */
void idt_install();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

/* isr.c */
void isr_install();

/* apic.c */
void apic_init();

/* scrn.h */
void cls();
void putch(char c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();
void itoa (char *buf, int base, unsigned long d);
void printf (const char *format, ...);

#endif
