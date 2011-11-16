#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stddef.h>
#include "config.h"
#include "info.h"

/* Verbosity: if not defined here, modules may define their own */
//#define VERBOSE 0

#define PAGE_BITS    12
#define PAGE_SIZE    (1<<PAGE_BITS)
#define PAGE_MASK    (PAGE_SIZE-1)
#if __x86_64__
#   define INDEX_BITS   9
#else
#   define INDEX_BITS  10
#endif
#define INDEX_MASK    ((1<<INDEX_BITS) -1)

#define STACK_SIZE    ((ptr_t)STACK_FRAMES * PAGE_SIZE)
#define STACK_MASK    (STACK_SIZE-1)

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

inline static void sti(void) {
    asm volatile ("sti");
}

inline static void cli(void) {
    asm volatile ("cli");
}

inline static void cpuid(uint32_t func, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func));
}

inline static void cpuid2(uint32_t func, uint32_t subfunc, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    asm volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func), "c"(subfunc));
}

inline static uint32_t cpuid_eax(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return eax;
}

inline static uint32_t cpuid_ebx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return ebx;
}

inline static uint32_t cpuid_ecx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return ecx;
}

inline static uint32_t cpuid_edx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return edx;
}


/* lib.c */
#include "lib.h"

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
void status_putch(int x, int c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();
void itoa (char *buf, int base, long d);
void printf (const char *format, ...);


#endif
