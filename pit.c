/*
 * PIT
 */

#include "system.h"
#include "pit.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_PIT > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_PIT > 1)

static uint64_t PIT_get_tsc_per_xxx(void) {
    uint16_t counter = 0x00;    // 0 = 65.536 ?
    uint64_t tsc_start, tsc_end;
    unsigned long count;

    // setup PIT
    outportb(PIT_MCR, 0x34);    // channel 0, lo/hi byte order, mode 2 (rate generator), binary counter format
    outportb(PIT_CHANNEL0, (uint8_t) (counter & 0xFF)); // low byte
    outportb(PIT_CHANNEL0, (uint8_t) ((counter >> 8) & 0xFF)); // high byte

    tsc_start = rdtsc();    // overhead not measured because very small compared to TSC difference

    while (1) {
#       if __x86_64__
        asm volatile (
                "xor %%rax, %%rax\n\t"  // set RAX to 0
                "mov $0x00, %%al\n\t" // channel 0, latch command
                "out %%al, $0x43\n\t"    // prevent the current count from being updated
                "in $0x40, %%al\n\t" // low byte of current count in AL
                "mov %%al, %%ah\n\t"
                "in $0x40, %%al\n\t"    // high byte of current count in AL
                "rol $8, %%ax\n\t"  // correct order
                : "=a" (count));
#       else
        asm volatile (
                "xor %%eax, %%eax\n\t"  // set RAX to 0
                "mov $0x00, %%al\n\t" // channel 0, latch command
                "out %%al, $0x43\n\t"    // prevent the current count from being updated
                "in $0x40, %%al\n\t" // low byte of current count in AL
                "mov %%al, %%ah\n\t"
                "in $0x40, %%al\n\t"    // high byte of current count in AL
                "rol $8, %%ax\n\t"  // correct order
                : "=a" (count));
#       endif

        if (count <= (65536 - 19549) ) {    // 1193182 * (1<<14) / 1000000 (ticks per 2^14 usec)
            tsc_end = rdtsc();
            break;
        }
    }

    IFVV printf("TSCs per hsec: %u\n", (tsc_end - tsc_start) );

    return (tsc_end - tsc_start) ;
}

/*
 * average TSC per second
 */
static void PIT_measure_tsc_per_sec(void) {
    const unsigned pot = 4;             // cycles: power-of-two
    unsigned cycles = (1 << pot);       // 2^4 = 16
    unsigned c;
    unsigned long long sum = 0;

    cli();  // no interrupts!

    if (cpuid_edx(0x80000007) & (1 << 8)) {
        IFVV printf("TSC is invariant!\n");
    } else {
        IFVV printf("TSC is variant!\n");
    }

    // TODO: use linear regression to increase accuracy
    for (c = 0; c < cycles; c++) {
        sum += PIT_get_tsc_per_xxx();
    }

    hw_info.tsc_per_usec = (sum >> (pot+14));
    IFV printf("TSC per usec: %u\n", (unsigned long)(hw_info.tsc_per_usec));
    hw_info.tsc_per_sec = hw_info.tsc_per_usec*1000000;
}

void pit_init(void)
{
    PIT_measure_tsc_per_sec();
}
