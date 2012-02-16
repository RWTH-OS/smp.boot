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
#include "cpu.h"

#define IA32_PERFEVTSEL0         0x186u
#define IA32_PERFGLOBALCTRL      0x38Fu
#define IA32_PMC_0               0x0C1u

#define IA32_PERFGLOBALCTRL_PMC0 0

#define IA32_EVENT_LLC_MISS      0x2Eull

#define IA32_PERFEVTSEL_OS       17
#define IA32_PERFEVTSEL_EN       22


void perfcount_init_cache(void) {
  uint32_t event_sel = IA32_EVENT_LLC_MISS | (1 << IA32_PERFEVTSEL_OS) | (1 << IA32_PERFEVTSEL_EN);

  perfcount_stop();
  wrmsr(IA32_PMC_0, event_sel);
  perfcount_reset();
}

void perfcount_start(void) {
  uint64_t global_ctrl = rdmsr(IA32_PERFGLOBALCTRL);
  global_ctrl |= (1 << IA32_PERFGLOBALCTRL_PMC0);
  wrmsr(IA32_PERFGLOBALCTRL, global_ctrl);
}

void perfcount_stop(void) {
  uint64_t global_ctrl = rdmsr(IA32_PERFGLOBALCTRL);
  global_ctrl &= ~(1 << IA32_PERFGLOBALCTRL_PMC0);
  wrmsr(IA32_PERFGLOBALCTRL, global_ctrl);
}

void perfcount_reset(void) {
  wrmsr(IA32_PMC_0, 0);
}

uint64_t perfcount_read(void) {
  return rdmsr(IA32_PMC_0);
}


