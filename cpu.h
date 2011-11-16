/*
 * =====================================================================================
 *
 *       Filename:  cpu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:05:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef CPU_H
#define CPU_H

void reboot();
void stop();

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


#endif //  CPU_H
