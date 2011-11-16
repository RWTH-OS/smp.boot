/*
 * PIT
 */

#ifndef __PIT_H
#define __PIT_H

enum PIT {
    PIT_FREQ = 1193182, // Hz
    PIT_CHANNEL0 = 0x40,    // timer
    PIT_CHANNEL2 = 0x42,    // PC speaker
    PIT_MCR = 0x43  // mode and command register (write only, ready ignored)
};

uint64_t PIT_measure_tsc_per_sec(void);

#endif  // __PIT_H
