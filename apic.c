#include "system.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_APIC > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_APIC > 1)

static ptr_t localAPIC =   0xfee00000UL;
static ptr_t ioAPIC_base = 0xfec00000UL;

uint32_t read_localAPIC(uint32_t offset)
{
    volatile uint32_t *localAPIC_Register = (uint32_t*)(localAPIC+offset);
    return *localAPIC_Register;
}
void write_localAPIC(uint32_t offset, uint32_t value)
{
    volatile uint32_t *localAPIC_Register = (uint32_t*)(localAPIC+offset);
    *localAPIC_Register = value;
}

uint32_t read_ioAPIC(unsigned id, uint32_t offset)
{
    volatile uint32_t* ioAPIC_Adr = (uint32_t*)(ioAPIC_base + (id*4096));
    volatile uint32_t* ioAPIC_Data = (uint32_t*)(ioAPIC_base + (id*4096) + 0x10);
    *ioAPIC_Adr = offset;
    /*  TODO: wait?! */
    return *ioAPIC_Data;
}
void write_ioAPIC(unsigned id, uint32_t offset, uint32_t value)
{
    volatile uint32_t* ioAPIC_Adr = (uint32_t*)(ioAPIC_base + (id*4096));
    volatile uint32_t* ioAPIC_Data = (uint32_t*)(ioAPIC_base + (id*4096) + 0x10);
    *ioAPIC_Adr = offset;
    /*  TODO: wait?! */
    *ioAPIC_Data = value;
}

void apic_init()
{
    /* the presence of a localAPIC (CPUID(EAX=1).EDX[bit 9]) was already checked in startXX.asm */

    /* local APIC address is in hw_info */
    localAPIC = hw_info.lapic_adr;

    IFVV printf("local APIC version: 0x%x\n", read_localAPIC(0x30) & 0xFF);

    /* support only VERSION >= 0x10 */

    /* deactivate PIC */
    outportb(0xA1, 0xFF);
    outportb(0x21, 0xFF);


    /* TODO: activate local APIC */



    /* TODO: start APs */


    /* TODO: activate I/O APIC */


    /* TODO: calibrate TSC (rdtsc) and CLK (time-base of local APIC timer) */



}
