/*
 * This is 32 bit C code for help during the boot sequence.
 * It is used for the 64 bit kernel but should be suited for the 32 bit kernel, too.
 * This is called from startXX.asm in REAL MODE (physical addresses)
 */

#include "bda_struct.h"
#include "acpi_struct.h"
#include "mps_struct.h"
#include "info.h"
#include "lib.h"

#include "config.h"
#define IFV   if (VERBOSE > 0 || VERBOSE_BOOT > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_BOOT > 1)

#define MP_FLT_SIGNATURE 0x5f504d5f


extern hw_info_t hw_info; 

/* scrn.c */
void cls();
void putch(char c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
void init_video();
void itoa (char *buf, int base, long d);
void printf (const char *format, ...);


void cpu_features()
{
    uint32_t eax, ebx, ecx, edx;
    void cpuid(unsigned func)
    {
        asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(func));
    }

    /* it was checked before, that the CPUID instruction is available */
    cpuid(0);
    hw_info.cpuid_max = eax;

    cpuid(0x80000000);
    hw_info.cpuid_high_max = eax;

    cpuid(1);
    hw_info.cpuid_family = (eax>>8)&0x1F;

    /*
     * TODO : read info about manufacturer (Intel/AMD), CPU model, cache?, ...
     */

}



/*
 * Functions for ACPI tables  ================================================================================
 */

static ptr_t search_rsdp(ptr_t offset, ptr_t size) 
{
    union {
        char str[9];
        uint32_t u32[2];
    } __attribute__((packed)) acpi_sig = { "RSD PTR " };

    volatile const uint32_t *p;
    unsigned i;

    p = (uint32_t*)offset;

    for (i = 0; i < size/sizeof(uint32_t); i++) {
        if (p[i] == acpi_sig.u32[0] && p[i+1] == acpi_sig.u32[1]) {
            return (ptr_t)(void*)&p[i];
        }
    }

    return 0;
}

/* check_hdr()
 *  - checksum
 *  - revision
 *  - optionally display OEMID, ...
 */
static int check_sum(void *hdr, size_t length)
{
    uint8_t sum = 0;
    uint8_t *p = (uint8_t*)hdr;
    unsigned i;

    for (i=0; i<length; i++) {
        sum += p[i];
    }
    if (sum != 0) {
        printf("WARNING: checksum = %hhu (should be 0)\n", sum);
    }
    return sum;
}

/*
 * The MADT (Multiprocessor APIC Description Table), Signature APIC,
 * contains information about the number of enabled processors and
 * the I/O APICs.
 */
static int read_madt(ptr_t offset)
{
    madt_t *madt = (madt_t*)offset;

    if (check_sum(madt, madt->header.length) != 0) {
        printf("WARNING: checksum of MADT invalid.\n");
        return -1;
    }

    hw_info.lapic_adr = madt->lapic_adr;
    IFV printf("local APIC adr: 0x%x\n", (ptr_t)hw_info.lapic_adr);

    unsigned i = 0;
    hw_info.cpu_cnt = 0;
    hw_info.ioapic_cnt = 0;
    while (__builtin_offsetof(madt_t, apic_structs)+i < madt->header.length) {
        unsigned subtype = madt->apic_structs[i];   /* first byte is type of entry */
        unsigned sublen = madt->apic_structs[i+1];  /* second byte is size of entry */
        madt_lapic_t *lapic;
        madt_ioapic_t *ioapic;

        switch (subtype) {
            case MADT_TYPE_LAPIC :
                lapic = (madt_lapic_t*)(ptr_t)&madt->apic_structs[i];
                if (lapic->flags.enabled) {
                    hw_info.cpu[hw_info.cpu_cnt].lapic_id = lapic->apic_id;
                    hw_info.cpu_cnt++;
                }
                IFV printf("CPU id=%u  enabled: %u local APIC id: %u\n", (ptr_t)lapic->acpi_processor_id, (ptr_t)lapic->flags.enabled, (ptr_t)lapic->apic_id);
                break;
            case MADT_TYPE_IOAPIC :
                ioapic = (madt_ioapic_t*)(ptr_t)&madt->apic_structs[i];
                hw_info.ioapic[hw_info.ioapic_cnt].id = ioapic->ioapic_id;
                hw_info.ioapic[hw_info.ioapic_cnt].adr = ioapic->ioapic_adr;
                hw_info.ioapic_cnt++;
                IFV printf("I/O APIC id=%u  adr: 0x%x\n", (ptr_t)ioapic->ioapic_id, (ptr_t)ioapic->ioapic_adr);
                break;

        }
        i += sublen;
    }

    return 0;
}


/*
 * functions for Multiprocessor Specification ================================================================
 */

static ptr_t search_fp(ptr_t offset, ptr_t size) 
{
    union {
        char str[5];
        uint32_t u32;
    } __attribute__((packed)) fp_sig = { "_MP_" };

    volatile const uint32_t *p;
    unsigned i;

    p = (uint32_t*)offset;

    for (i = 0; i < size/sizeof(uint32_t); i++) {
        if (p[i] == fp_sig.u32) {
            return (ptr_t)(void*)&p[i];
        }
    }

    return 0;
}







/* ===========================================================================================================
 * get_info()
 *
 * This function is called from the 32 bit startup code in real-mode to collect
 * information about the hardware from
 *  - BIOS Data Area (BDA) / Extended BDA (EBDA)
 *  - ACPI (if available) (1)
 *  - Multiprocessor Specification tables (if available)
 * The collected information is stored in internal data structures that are
 * shared with the protected mode kernel (32- or 64-bit).
 *
 *
 *  (1) note: this OS does not support ACPI Power Management but only reads
 *      information about available CPUs and APICs from the ACPI tables.
 */

void get_info()
{
	//unsigned long addr;
	unsigned i;//, count;

    init_video();
    IFVV printf("get_info()\n");

    /* initialize hw_info to all 0's */
    memset(&hw_info, 0, sizeof(hw_info));


    /*
     * check, what CPUID functions are available
     */
    cpu_features();



    /*
     * first, read BIOS Data Area
     */
    bda_t *bda = (bda_t*)0x400;

    /* get EBDA Segment 16-bit real-mode segment, i.e. shift left by 4 bits to get physical address */
    ebda_t *ebda = (ebda_t*)((ptr_t)bda->segm_ebda << 4);
    hw_info.ebda_adr = bda->segm_ebda << 4;
    hw_info.ebda_size = ebda->size;
    
    /*
     * search for ACPI tables 
     *  - EBDA
     *  - physical address range 0xE0000 .. 0xFFFFF
     */
    rsdp_t *rsdp = (rsdp_t*)search_rsdp(hw_info.ebda_adr, hw_info.ebda_size);
    if (rsdp == 0) {
        rsdp = (rsdp_t*)search_rsdp(0xE0000, 0x20000);
    }
    if (rsdp == 0) {
        printf("no RSDP found.\n");
        goto skip_acpi;
    }

    if (check_sum(rsdp, 20) != 0) {
        printf("WARNING: checksum of ACPI RSDP is incorrect.");
        goto skip_acpi;
    }

    rsdt_t *rsdt = (rsdt_t*)(ptr_t)rsdp->rsdt_adr;

    if (check_sum(rsdt, rsdt->header.length) != 0) {
        printf("WARNING: checksum of ACPI RSDT is incorrect.");
        goto skip_acpi;
    }

    for (i=0; i<(rsdt->header.length-sizeof(desc_hdr_t))/sizeof(uint32_t); i++) {
        desc_hdr_t *hdr = (desc_hdr_t*)(ptr_t)rsdt->entry[i];

        //char str[5] = { 0 };
        //memcpy(str, &hdr->signature, 4);
        //printf("RSDT Entry[%u]: 0x%x %s (size: %u)\t", i, rsdt->entry[i], str, len);

        /* signature to table translation: see APICspec40a.pdf, Table 5-5, p. 114 */
        switch (hdr->signature) {
            //case FADT_SIGNATURE :  /* FACP -> FADT  */
                //printf("Fixed ACPI Descr. Table (FADT)\n");
                //break;
            case MADT_SIGNATURE :  /* APIC -> MADT  */
                //printf("Multiple APIC Descr. T. (MADT)\n");
                read_madt(rsdt->entry[i]);
                break;
            //default :
                //printf("not supported, yet\n");

        };
    }
    

    if (hw_info.lapic_adr != 0) {
        /* we have, what we need (skip multiprocessor specification tables) */
        goto skip_mps;
    }
        

skip_acpi:
    0;

/*
 * now read multiprocessor tables
 */
    /* 1. In the first kilobyte of the Extended BIOS Data Area (EBDA) */
    mps_fp_t *fp;
    fp = (mps_fp_t*)search_fp(hw_info.ebda_adr, 1024);
    /* 2. Within the last kilobyte of system base memory if EBDA is undefined */
    if (fp == 0) {
        fp = (mps_fp_t*)search_fp(639*1024, 1024);
    }
    /* 3. At the top of system physical memory */
    // ?!?
    
    /* 4. In the BIOS read-only memory space between 0xE0000 and 0xFFFFF */
    if (fp == 0) {
        fp = (mps_fp_t*)search_fp(0xE0000, 0x20000);
    }
    if (fp == 0) {
        printf("no FP found.\n");
        goto skip_mps;
    }

    if (check_sum(fp, fp->length*16) != 0) {
        printf("WARNING: checksum of MPS FP is incorrect.\n");
        goto skip_mps;
    }

    if (fp->spec_rev != 4 || fp->feature[0] != 0 || fp->offset_config == 0) {
        printf("WARNING: MPS revision or features not supported.\n");
        goto skip_mps;
    }

    mps_config_t *config = (mps_config_t*)(ptr_t)fp->offset_config;
    IFVV printf("config_t = 0x%x\n", config);

    if (check_sum(config, config->base_length) != 0) {
        printf("WARNING: checksum of MPS base CONFIG is incorrect.\n");
        goto skip_mps;
    }

    hw_info.lapic_adr = config->adr_lapic;

    if (check_sum(config, config->extd_len) != 0) {
        printf("WARNING: checksum of MPS extended CONFIG is incorrect.\n");
        goto skip_mps;
    }

    hw_info.cpu_cnt = 0;
    hw_info.ioapic_cnt = 0;
    uint8_t *type = (uint8_t*)((ptr_t)config + sizeof(mps_config_t));
    for (i=0; i < config->cnt_oem; i++) {
        mps_conf_processor_t *processor;
        mps_conf_ioapic_t *ioapic;
        switch (*type) {
            case 0 :
                processor = (mps_conf_processor_t*)type;
                IFVV printf("CPU %u\n", processor->lapic_id);
                if (processor->flags.en) {
                    hw_info.cpu[hw_info.cpu_cnt].lapic_id = processor->lapic_id;
                    hw_info.cpu_cnt++;
                }
                type += sizeof(mps_conf_processor_t);
                break;
            case 2 :
                ioapic = (mps_conf_ioapic_t*)type;
                IFVV printf("I/O APIC %u  0x%x\n", ioapic->ioapic_id, ioapic->adr_ioapic);
                if (ioapic->flags.en) {
                    hw_info.ioapic[hw_info.ioapic_cnt].id = ioapic->ioapic_id;
                    hw_info.ioapic[hw_info.ioapic_cnt].adr = ioapic->adr_ioapic;
                    hw_info.ioapic_cnt++;
                }
                type += sizeof(mps_conf_ioapic_t);
                break;
            default :
                type += 8;  /* all entries except processor are 8 bytes long */

        }
    }

skip_mps:

    if (hw_info.lapic_adr == 0 || hw_info.cpu_cnt == 0 || hw_info.ioapic_cnt == 0) {
        printf("ERROR: neither ACPI nor MP gave me information about CPUs and APICs. Halt.\n");
        halt();
    }
    IFV printf("found %d CPUs and %d I/O APICs\n", (ptr_t)hw_info.cpu_cnt, (ptr_t)hw_info.ioapic_cnt);


    return;
}
