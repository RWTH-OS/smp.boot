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

#define IA32_PERFEVTSEL_USR      16
#define IA32_PERFEVTSEL_OS       17
#define IA32_PERFEVTSEL_EN       22

void perfcount_init_l1d_miss(void) {
  uint64_t event_sel = 0x24 | (0x2 << 8) | (1 << IA32_PERFEVTSEL_OS) | (1 << IA32_PERFEVTSEL_USR);
  uint64_t global_ctrl = (1 << IA32_PERFGLOBALCTRL_PMC0);

  // Initialize PMC
  wrmsr(IA32_PERFGLOBALCTRL, global_ctrl);

  // Write but leave the counter in disabled state
  wrmsr(IA32_PERFEVTSEL0, event_sel);
  
  // Reset PMC
  perfcount_reset();
}

void perfcount_init_l2_miss(void) {
  uint64_t event_sel = 0x24 | (0x2 << 8) | (1 << IA32_PERFEVTSEL_OS) | (1 << IA32_PERFEVTSEL_USR);
  uint64_t global_ctrl = (1 << IA32_PERFGLOBALCTRL_PMC0);

  // Initialize PMC
  wrmsr(IA32_PERFGLOBALCTRL, global_ctrl);

  // Write but leave the counter in disabled state
  wrmsr(IA32_PERFEVTSEL0, event_sel);
  
  // Reset PMC
  perfcount_reset();
}

void perfcount_start(void) {
  uint64_t event_sel = rdmsr(IA32_PERFEVTSEL0);
  event_sel |= (1 << IA32_PERFEVTSEL_EN);
  wrmsr(IA32_PERFEVTSEL0, event_sel);
}

void perfcount_stop(void) {
  uint64_t event_sel = rdmsr(IA32_PERFEVTSEL0);
  event_sel &= ~(1 << IA32_PERFEVTSEL_EN);
  wrmsr(IA32_PERFEVTSEL0, event_sel);
}

void perfcount_reset(void) {
	wrmsr(IA32_PMC_0, 0);
}

uint64_t perfcount_read(void) {
	return rdmsr(IA32_PMC_0);
}

