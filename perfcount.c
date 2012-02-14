/*
 * =====================================================================================
 *
 *       Filename:  perfcount.c
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

#include "perfcount.h"
#include "system.h"

#define IA32_PERFEVTSEL0         ((unsigned int)0x186)
#define IA32_PERFGLOBALCTRL      ((unsigned int)0x38F)
#define IA32_PMC_0               ((unsigned int)0xC1)

#define IA32_PERFGLOBALCTRL_PMC0 (0)

#define IA32_EVENT_LLC_MISS      ((unsigned long long)0x2E)

#define IA32_PERFEVTSEL_OS       (17)
#define IA32_PERFEVTSEL_EN       (22)

static unsigned long long rdmsr(unsigned int msr) {
  unsigned int low, high;

  __asm__ __volatile__ ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((unsigned long long)high << 32) | low;
}

static void wrmsr(unsigned int msr, unsigned long long value) {
  unsigned int low = value & 0xFFFFFFFF;
  unsigned int high = value >> 32;

  __asm__ __volatile__ ("wrmsr" :: "a"(low), "c"(msr), "d"(high));
}

void perfcount_init_cache(void) {
  unsigned long event_sel = IA32_EVENT_LLC_MISS | (1 << IA32_PERFEVTSEL_OS) | (1 << IA32_PERFEVTSEL_EN);

  wrmsr(IA32_PMC_0, event_sel);
  perfcount_stop();
  perfcount_reset();
}

void perfcount_start(void) {
  unsigned long long global_ctrl = rdmsr(IA32_PERFGLOBALCTRL);
  global_ctrl |= (1 << IA32_PERFGLOBALCTRL_PMC0);
  wrmsr(IA32_PERFGLOBALCTRL, global_ctrl);
}

void perfcount_stop(void) {
  unsigned long long global_ctrl = rdmsr(IA32_PERFGLOBALCTRL);
  global_ctrl &= ~(1 << IA32_PERFGLOBALCTRL_PMC0);
  wrmsr(IA32_PERFGLOBALCTRL, global_ctrl);
}

void perfcount_reset(void) {
  wrmsr(IA32_PMC_0, 0);
}

unsigned long long perfcount_read(void) {
  return rdmsr(IA32_PMC_0);
}


