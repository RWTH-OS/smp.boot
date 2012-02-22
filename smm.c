/*
 * =====================================================================================
 *
 *       Filename:  smm.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 16:05:21
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "stddef.h"
#include "system.h"
#include "config.h"
#include "cpu.h"
#include "pci.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_SMM > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_SMM > 1)

//#define IN(var, port) __asm__ volatile ("in %%dx, %%eax" : "=a" (var) : "d" (port))
//#define OUT(port, var) __asm__ volatile ("out %%eax, %%dx" :  : "a" (var), "d" (port))
#define LPC_BUS    0x00
#define LPC_DEVICE 0x1F
#define LPC_FUNC   0x00
#define LPC_OFFSET 0x40
#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC
static uint32_t bak_smi_en;
static uint32_t pmbase = 0;             

void smm_init(void)
{
    uint32_t adr;
    uint32_t smi_en;
    unsigned if_backup;

    if_backup = cli();

    
    /*
    adr = (1<<31) | (LPC_BUS << 16)  |  (LPC_DEVICE << 11)  |  (LPC_FUNC <<  8)  |  LPC_OFFSET;
    outportl(CONFIG_ADDRESS, adr);
    pmbase = inportl(CONFIG_DATA);
    */
    pmbase = pci_config_read(LPC_BUS, LPC_DEVICE, LPC_FUNC, LPC_OFFSET);
    pmbase &= 0xFF80;
    IFV printf("SMM: located pmbase : 0x%x \n", pmbase);



    if (pmbase != 0) {

        adr = pmbase+0x30;                          /* SMI_EN I/O register is at pmbase+0x30 */

        /* read SMI_EN and display */
        smi_en = inportl(adr);
        bak_smi_en = smi_en;
        IFVV printf("SMI_EN: 0x%x \n", smi_en);
    }

    if (if_backup) sti();
}

void smm_deactivate(void)
{
    uint32_t adr;
    uint32_t smi_en;
    unsigned if_backup;

    if_backup = cli();

    if (pmbase != 0) {
        adr = pmbase+0x30;                          /* SMI_EN I/O register is at pmbase+0x30 */

        /* set to 0 (only writable bits will be changed...) */
        smi_en = 0; //&= ~1;
        outportl(adr, smi_en);

        /* read again to see what has changed */
        smi_en = inportl(adr);
        IFVV printf("SMI_EN: 0x%x (after setting SMI_EN to 0; bit 0 is GBL_SMI_EN)\n", smi_en);

        /* if GBL_SMI_EN (bit #0) is 0, deactivation was successful */
        if ((smi_en & 0x01) == 0) {
            IFV printf("SMI globally disabled\n");
        } else {
            printf("Warning: SMI was not disabled!\n");
        }
    }

    if (if_backup) sti();
}

void smm_restore(void)
{
    unsigned if_backup;

    if_backup = cli();

    if (pmbase != 0) {
        outportl(pmbase+0x30, bak_smi_en);
    }

    if (if_backup) sti();
}


