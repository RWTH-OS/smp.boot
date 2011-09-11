#include "system.h"
#include "apic.h"

#ifndef VERBOSE
#define VERBOSE 1
#endif

static ptr_t localAPIC =   0xfee00000UL;
static ptr_t ioAPIC_base = 0xfec00000UL;
static uint32_t ioAPIC_count = 0;
//extern uint32_t global_mp;

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

    /*  TODO: read local APIC address from MSR */

    if (VERBOSE) printf("local APIC version: %x\n", read_localAPIC(0x30) & 0xFF);

    /* support only VERSION >= 0x10 */

    /* TODO: deactivate PIC, activate APIC */



/*

    / * show some infos from the Multiprocessor Table global_mp * /
    printf("mp floating pointer structure: %x\n", (ptr_t)global_mp);
    apic_mp_t *mp = (apic_mp_t*)(unsigned long)global_mp;
    printf("mp spec version:               %x\n", mp->version);

    k/printf("MP System Config Type:         %x\n", (long)mp->features[0]);
    if (mp->features[0] != 0) {
        printf("ERROR: no config table. Default config currently not supported.\n");
        halt();
    }
    //printf("mp IMCR (1: IMCR present; 0: PIC Mode): %x\n", (long)(mp->features[2] >> 7));

    apic_config_table_t *act = (apic_config_table_t*)(unsigned long)mp->mp_config;
    printf("mp config table:               %x\n", (ptr_t)act);

    localAPIC = (ptr_t)act->lapic;
    printf("lapic:                         %x\n", localAPIC);

    / *
    char string[32] = {0};
    memcpy(&string[0], act->oem_id, 8);
    string[8] = '/';
    memcpy(&string[9], act->product_id, 12);
    printf("oem_id/product_id:             %s\n", string);
    * /

    printf("apic entry count               %u\n", act->entry_count);
    unsigned i;
    uint8_t *type = (uint8_t*)((unsigned long)act + sizeof(apic_config_table_t));
    for (i=0; i<act->entry_count; i++) {
        if (*type == 0) {
            / * Processor entry * /
            apic_processor_entry_t *entry = (apic_processor_entry_t*)type;

            printf("CPU %d en:%d bp:%d sig:%x feat:%x\n", 
                    (ptr_t)entry->id, (ptr_t)entry->cpu_flags&1, (ptr_t)entry->cpu_flags&2, 
                    (ptr_t)entry->cpu_signature, (ptr_t)entry->cpu_feature);

            type += sizeof(apic_processor_entry_t);
        } else if (*type == 1) {
            / * Bus entry * /
            apic_bus_entry_t *entry = (apic_bus_entry_t*)type;
            char string[7] = {0};
            memcpy(string, &entry->name[0], 6);

            printf("BUS %d %s\n", entry->bus_id, string);

            type += sizeof(apic_bus_entry_t);
        } else if (*type == 2) {
            / * I/O APIC entry * /
            apic_io_entry_t *entry = (apic_io_entry_t*)type;

            printf("I/O APIC %d ver=%x en:%d addr=%x\n", 
                    entry->id, entry->version, entry->enabled&1, entry->addr);

            ioAPIC_base = (ptr_t)entry->addr;
            ioAPIC_count++;

            type += sizeof(apic_io_entry_t);
        } else if (*type == 3) {
            / * I/O Interrupt Asignment entry * /
            apic_ioirq_entry_t *entry = (apic_ioirq_entry_t*)type;

            printf("I/O Interrupt type=%d bus=%d irq=%d dest=%d/%d\n", 
                    entry->itype, entry->src_bus, entry->src_irq, entry->dest_apic, entry->dest_intin);

            type += sizeof(apic_ioirq_entry_t);
        } else if (*type == 4) {
            / * Local Interrupt Assignment entry * /
            apic_ioirq_entry_t *entry = (apic_ioirq_entry_t*)type;

            printf("local Interrupt type=%d bus=%d irq=%d dest=%d/%d\n", 
                    entry->itype, entry->src_bus, entry->src_irq, entry->dest_apic, entry->dest_intin);

            type += sizeof(apic_ioirq_entry_t);
        } else {
            //printf("WARNING: found unknown entry type: %d\n", *type);
        }
        if (VERBOSE && i%10==0) udelay(10000000);
    }
    halt();
*/



    /* TODO: calibrate TSC (rdtsc) and CLK (time-base of local APIC timer) */



}
