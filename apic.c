#include "system.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_APIC > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_APIC > 1)

#define LAPIC_REG_ID        0x0020
#define LAPIC_REG_VERSION   0x0030
#define LAPIC_REG_SPURIOUS  0x00F0
#define LAPIC_ICR_LOW       0x0300
#define LAPIC_ICR_HIGH      0x0310
#define LAPIC_LVT_TIMER     0x0320
#define LAPIC_LVT_ERROR     0x0370

static ptr_t localAPIC =   0xfee00000UL;
static ptr_t ioAPIC_base = 0xfec00000UL;

volatile unsigned cpu_online = 0;

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
void set_localAPIC(uint32_t offset, uint32_t mask, uint32_t value)
{
    volatile uint32_t *localAPIC_Register = (uint32_t*)(localAPIC+offset);
    uint32_t reg = *localAPIC_Register;
    reg &= ~mask;
    reg |= value;
    *localAPIC_Register = reg;
}

uint32_t read_ioAPIC(unsigned id, uint32_t offset)
{
    // TODO : store IF, CLI
    volatile uint32_t* ioAPIC_Adr = (uint32_t*)(ioAPIC_base + (id*4096));
    volatile uint32_t* ioAPIC_Data = (uint32_t*)(ioAPIC_base + (id*4096) + 0x10);
    *ioAPIC_Adr = offset;
    /*  TODO: wait?! */
    return *ioAPIC_Data;
    // TODO : restore IF to previous value
}
void write_ioAPIC(unsigned id, uint32_t offset, uint32_t value)
{
    // TODO : store IF, CLI
    volatile uint32_t* ioAPIC_Adr = (uint32_t*)(ioAPIC_base + (id*4096));
    volatile uint32_t* ioAPIC_Data = (uint32_t*)(ioAPIC_base + (id*4096) + 0x10);
    *ioAPIC_Adr = offset;
    /*  TODO: wait?! */
    *ioAPIC_Data = value;
    // TODO : restore IF to previous value
}

void apic_init()
{
    uint16_t u;
    /* the presence of a localAPIC (CPUID(EAX=1).EDX[bit 9]) was already checked in startXX.asm */

    /* local APIC address is in hw_info */
    localAPIC = hw_info.lapic_adr;


    /* support only VERSION >= 0x10 */

    /* deactivate PIC */
    outportb(0xA1, 0xFF);
    outportb(0x21, 0xFF);


    /* 
     * activate local APIC 
     */

    /* Presence: CPUID.01h:EDX[bit 9] (checked already in start.asm) */

    /* To initialise the BSP's local APIC, set the enable bit in the spurious
     * interrupt vector register and set the error interrupt vector in the
     * local vector table.  */
    set_localAPIC(LAPIC_REG_SPURIOUS, (1<<8), (1<<8));

    IFVV printf("local APIC version: 0x%x  max LVT entry: %u\n", 
            read_localAPIC(LAPIC_REG_VERSION) && 0xFF, 
            ((read_localAPIC(LAPIC_REG_VERSION)>>16) && 0xFF)+1);

    /*
     * according to http://www.osdever.net/tutorials/view/multiprocessing-support-for-hobby-oses-explained
     * the spurios int vector can be ignored (use 0x1F for now...)
     * and the lowest 4 bits are hardwired to 1 (only 0x?F can be used)
     */
    write_localAPIC(LAPIC_LVT_ERROR, 0x1F);       /* 0x1F: temporary vector (all other bits: 0) */


    /* 
     * start APs 
     */

    /* install initial code to a physical page below 640 kB */

    uint8_t *ptr = (uint8_t*)(ptr_t)(SMP_FRAME << 12);

    extern uint8_t smp_start[];
    extern uint16_t smp_apid;
    extern uint8_t smp_end;
    uint16_t size = (uint16_t)((ptr_t)&smp_end - (ptr_t)&smp_start);

    /* pointer to the variable smp_apid in that page */
    volatile uint16_t *ptr_apid = (void*)ptr + ((ptr_t)&smp_apid - (ptr_t)&smp_start);

    IFVV printf("smp_start = 0x%x  smp_end = 0x%x  size = %u\n", (ptr_t)&smp_start, (ptr_t)&smp_end, size);

    if (size > PAGE_SIZE) {
        printf("WARNING: SMP start code larger than one page!\n");
        return; 
    }

    /* copy code byte-wise (TODO: why not use memcpy?) */
    for (u=0; u<size; u++) {
        ptr[u] = smp_start[u];
    }
    

    /* set up status monitor for APs */
    status_putch(6, '[');
    status_putch(6+hw_info.cpu_cnt, ']');

    /* now send IPIs to the APs */
    for (u = 1; u < hw_info.cpu_cnt; u++) {
        *ptr_apid = u;
        IFV printf("SMP: try to wake up AP#%u\n", u);
        IFVV printf("  #%u: send INIT IPI\n", u);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[u].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x5 << 8)|SMP_FRAME);

        udelay(10*1000); /* 10 ms */
        
        IFVV printf("  #%u: send first SIPI\n", u);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[u].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x6 << 8)|SMP_FRAME);

        udelay(200);  /* 200 us */
        
        IFVV printf("  #%u: send second SIPI\n", u);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[u].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x6 << 8)|SMP_FRAME);

        udelay(100 * 1000); /* 100 ms */
        // TODO: check, if CPU is up.
        //   ...else: what ?!
        
    }

    udelay(100*1000);
    IFV printf("all %u APs called, %u up\n", hw_info.cpu_cnt-1, cpu_online);


    /* TODO: activate I/O APIC */


    /* TODO: calibrate TSC (rdtsc) and CLK (time-base of local APIC timer) */



}
