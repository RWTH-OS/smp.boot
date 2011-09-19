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
    unsigned i;
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

#define SMP_PAGE  0x88
    uint8_t *ptr = (uint8_t*)(ptr_t)(SMP_PAGE << 12);

    extern uint8_t smp_start[];
    extern uint16_t smp_var;
    extern uint8_t smp_end;
    ptr_t size = (ptr_t)&smp_end - (ptr_t)&smp_start;

    uint16_t *ptr_var = (void*)ptr + ((ptr_t)&smp_var - (ptr_t)&smp_start);

    printf("smp_start = 0x%x  \n", (ptr_t)&smp_start);
    printf("smp_end   = 0x%x  \n", (ptr_t)&smp_end);
    printf("smp size  = %u  \n", size);


    ptr_t u;
    for (u=0; u<size; u++) {
        ptr[u] = smp_start[u];
    }
    

    *ptr_var = 0x000e;

    status_putch(6, '[');
    status_putch(6+hw_info.cpu_cnt, ']');
    /* now send IPIs to the APs */
    for (i = 1; i < hw_info.cpu_cnt; i++) {
        IFV printf("SMP: try to wake up AP#%u\n", i);
        IFVV printf("  #%u: send INIT IPI\n", i);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[i].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x5 << 8)|SMP_PAGE);

        udelay(10*1000);
        
        IFVV printf("  #%u: send first SIPI\n", i);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[i].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x6 << 8)|SMP_PAGE);

        udelay(200);
        
        IFVV printf("  #%u: send second SIPI\n", i);
        write_localAPIC(LAPIC_ICR_HIGH, (uint32_t)hw_info.cpu[i].lapic_id<<24);
        write_localAPIC(LAPIC_ICR_LOW,  (uint32_t)   (0x6 << 8)|SMP_PAGE);

        udelay(100000);
        // TODO: check, if CPU is up.
        
        //ptr[8] += 2;       // TODO : next CPU in next position of status monitor
        *ptr_var += 2;
    }


    /* TODO: activate I/O APIC */


    /* TODO: calibrate TSC (rdtsc) and CLK (time-base of local APIC timer) */



}
