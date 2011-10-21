/*
 * =====================================================================================
 *
 *       Filename:  mm.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  21.10.2011 10:27:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "system.h"
#include "mm.h"
#include "smp.h"
#include "multiboot_struct.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_MM > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_MM > 1)

typedef union {
    uint64_t u64;
    struct {
        uint64_t p     :  1;
        uint64_t rw    :  1;
        uint64_t us    :  1;
        uint64_t pwd   :  1;
        uint64_t pcd   :  1;
        uint64_t a     :  1;
        uint64_t ign   :  1;
        uint64_t mbz   :  2;
        uint64_t avl1  :  3;
        uint64_t frame : 40;
        uint64_t avl2  : 11;
        uint64_t nx    :  1;
    } bits;
} pml4_entry_t;
typedef union {
    uint64_t u64;
    struct {
        uint64_t p     :  1;
        uint64_t rw    :  1;
        uint64_t us    :  1;
        uint64_t pwd   :  1;
        uint64_t pcd   :  1;
        uint64_t a     :  1;
        uint64_t ign   :  1;
        uint64_t zero  :  1;
        uint64_t mbz   :  1;
        uint64_t avl1  :  3;
        uint64_t frame : 40;
        uint64_t avl2  : 11;
        uint64_t nx    :  1;
    } bits;
} pdpe_entry_t;
typedef union {
    uint64_t u64;
    struct {
        uint64_t p     :  1;
        uint64_t rw    :  1;
        uint64_t us    :  1;
        uint64_t pwd   :  1;
        uint64_t pcd   :  1;
        uint64_t a     :  1;
        uint64_t ign   :  1;
        uint64_t zero  :  1;
        uint64_t mbz   :  1;
        uint64_t avl1  :  3;
        uint64_t frame : 40;
        uint64_t avl2  : 11;
        uint64_t nx    :  1;
    } bits;
} pde_entry_t;
typedef union {
    uint64_t u64;
    struct {
        uint64_t p     :  1;
        uint64_t rw    :  1;
        uint64_t us    :  1;
        uint64_t pwd   :  1;
        uint64_t pcd   :  1;
        uint64_t a     :  1;
        uint64_t d     :  1;
        uint64_t pat   :  1;
        uint64_t g     :  1;
        uint64_t avl1  :  3;
        uint64_t frame : 40;
        uint64_t avl2  : 11;
        uint64_t nx    :  1;
    } bits;
} pte_entry_t;

pml4_entry_t *pml4;

uint64_t freemap[MAX_MEM / PAGE_SIZE / 64];
static inline unsigned frame_to_freemap_index(unsigned frame)
{
    return (frame >> 6);        // DIV 64
}
static inline unsigned frame_to_freemap_bit(unsigned frame)
{
    return (frame & 0x3F);      // MOD 64
}
uint64_t get_free_frame()
{
    static unsigned last_frame = 0x400; // start at 4 MB (frame << 12 = 0x400000)
    while (IS_BIT_CLEAR(freemap[frame_to_freemap_index(last_frame)], frame_to_freemap_bit(last_frame))) {
        last_frame++;
        if (last_frame >= (MAX_MEM / PAGE_SIZE)) {
            printf("ERROR: out of memory!\n");
            smp_status('E');
            while (1) asm volatile ("hlt");
        }
    }
    BIT_CLEAR(freemap[frame_to_freemap_index(last_frame)], frame_to_freemap_bit(last_frame));
    return last_frame;
}

/*
 * In 64 bit mode, paging is enabled by start64.asm and the first 2 MB are identity-mapped.
 */
int mm_init()
{
    IFV printf("mm_init() 64 bit version\n");

    /* read address of page table PML4 (first level) from register cr3 */
    asm volatile ("mov %%cr3, %%rax" : "=a"(pml4));
    IFVV printf("MM: pml4 = 0x%x\n", (ptr_t)pml4);

    /* checks of page table entries */
    IFVV printf("MM: pml4[0].p : %u\n", pml4[0].bits.p);
    IFVV printf("MM: pml4[1].p : %u\n", pml4[1].bits.p);

    pdpe_entry_t *pdpe = (pdpe_entry_t*)(ptr_t)(pml4[0].bits.frame << 12);
    IFVV printf("MM: pml4[0]->pdpe = 0x%x\n", (ptr_t)pdpe);

    pde_entry_t *pde = (pde_entry_t*)(ptr_t)(pdpe[0].bits.frame << 12);
    IFVV printf("MM: pml4[0]->pdpe[0]->pde = 0x%x\n", (ptr_t)pde);

    IFVV printf("MM: pml4[0]->pdpe[0]->pde[0].p = %d   adr = 0x%x\n", pde[0].bits.p, pde[0].bits.frame<<12);
    IFVV printf("MM: pml4[0]->pdpe[0]->pde[1].p = %d   adr = 0x%x\n", pde[1].bits.p, pde[1].bits.frame<<12);

    pte_entry_t *pte = (pte_entry_t*)(ptr_t)(pde[0].bits.frame << 12);
    IFVV printf("MM: pml4[0]->pdpe[0]->pde[0]->pte = 0x%x\n", (ptr_t)pte);

    /*
     * initialize freemap[]
     */
    memset(freemap, 0, sizeof(freemap));
    multiboot_info_t *mbi = (multiboot_info_t*)(ptr_t)hw_info.mb_adr;
    if (IS_BIT_SET(mbi->flags, 0)) {
        // mem_upper is from 1 MB up to first memory hole.
        // we start with this and add more later if we find mmap_ (flags[6])
        IFVV printf("flags[0] - mem_lower: 0x%x=%d  mem_upper: 0x%x=%d\n", mbi->mem_lower, mbi->mem_lower, mbi->mem_upper, mbi->mem_upper);
        unsigned u;     // start at 0x400000 (4 MB)
        for (u = 0x400; u < (mbi->mem_upper >> 2)+0x100; u++) {
            BIT_SET(freemap[frame_to_freemap_index(u)], frame_to_freemap_bit(u));
        }
    }

    if (IS_BIT_SET(mbi->flags, 6)) {
        char mem_type[][10] = {"mem", "other"};
        IFVV printf("flags[6] - mmap_length: %d  mmap_addr: 0x%x\n", mbi->mmap_length, mbi->mmap_addr);
        multiboot_memory_map_t* p = (multiboot_memory_map_t*)(long)mbi->mmap_addr;
        for ( ; p < (multiboot_memory_map_t*)(long)(mbi->mmap_addr+mbi->mmap_length); p = ((void*)p + p->size + 4)) {
            if (p->type == 1) {
                unsigned u;
                IFVV printf("  mmap[0x%x] - addr:0x%x  len:0x%x  type: %d (%s)\n",
                        p, (multiboot_uint32_t)(p->addr), (multiboot_uint32_t)(p->len), p->type, mem_type[p->type==1?0:1]);
                for (u = (p->addr>>12); u < ((p->addr + p->len) >> 12); u++) {
                    if (u >= 0x400)
                        BIT_SET(freemap[frame_to_freemap_index(u)], frame_to_freemap_bit(u));
                }
            }
        }
    }

    BIT_CLEAR(freemap[frame_to_freemap_index(0x201)], frame_to_freemap_bit(0x201));
    printf("get_free_frame() : %x\n", get_free_frame());
    printf("get_free_frame() : %x\n", get_free_frame());
    printf("get_free_frame() : %x\n", get_free_frame());
    

    //int *p = (int*)0x00200000-4;    // 2 MB (- 4B) : access fine; 2 MB + 4B : page fault
    //*p = 0;
    return 0;
}



