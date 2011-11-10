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
#include "sync.h"
#include "multiboot_struct.h"
#include "mm_struct.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_MM > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_MM > 1)


/*
 * helper functions to extract offset and table indices from a (virtual) address
 */
static inline ptr_t offset(void * adr) 
{
    return ((ptr_t)adr) & PAGE_MASK;
}
static inline ptr_t pt_index(void * adr) 
{
    return (((ptr_t)adr) >> PAGE_BITS) & INDEX_MASK;
}
#if __x86_64__
static inline ptr_t pd3_index(void * adr) 
{
    return (((ptr_t)adr) >> (PAGE_BITS+INDEX_BITS)) & INDEX_MASK;
}
static inline ptr_t pd2_index(void * adr) 
{
    return (((ptr_t)adr) >> (PAGE_BITS+2*INDEX_BITS)) & INDEX_MASK;
}
static inline ptr_t pd1_index(void * adr) 
{
    return (((ptr_t)adr) >> (PAGE_BITS+3*INDEX_BITS)) & INDEX_MASK;
}
#else
static inline ptr_t pd1_index(void * adr) 
{
    return (((ptr_t)adr) >> (PAGE_BITS+INDEX_BITS)) & INDEX_MASK;
}
#endif

/*  --------------------------------------------------------------------------- */

/* global pointer to pd1 (1st level page directory; pml4) */
pd1_entry_t *pd1;

/*  --------------------------------------------------------------------------- */

/*
 * helper functions to convert virtual <-> physical address
 */
ptr_t virt_to_phys(void * adr)
{
#   if __x86_64__
    if (pd1[pd1_index(adr)].dir.p) {
        pd2_entry_t *pd2 = (pd2_entry_t*)((ptr_t)pd1[pd1_index(adr)].dir.frame << PAGE_BITS);
        if (pd2[pd2_index(adr)].dir.p) {
            pd3_entry_t *pd3 = (pd3_entry_t*)((ptr_t)pd2[pd2_index(adr)].dir.frame << PAGE_BITS);
            if (pd3[pd3_index(adr)].dir.p) {
                pt_entry_t *pt = (pt_entry_t*)((ptr_t)pd3[pd3_index(adr)].dir.frame << PAGE_BITS);
                if (pt[pt_index(adr)].page.p) {
                    return (((ptr_t)pt[pt_index(adr)].page.frame << PAGE_BITS) + offset(adr));
                } else {
                    return 0;
                }
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    } else {
        return 0;
    }
#   else
    if (pd1[pd1_index(adr)].dir.p) {
        pt_entry_t *pt = (pt_entry_t*)((ptr_t)pd1[pd1_index(adr)].dir.frame << PAGE_BITS);
        if (pt[pt_index(adr)].page.p) {
            return (((ptr_t)pt[pt_index(adr)].page.frame << PAGE_BITS) + offset(adr));
        } else {
            return 0;
        }
    } else {
        return 0;
    }
#   endif
    return 0;
}

/*
 * phys_to_virt only possible by a tree-search through the page directories...
 */







/*  --------------------------------------------------------------------------- */


unsigned long freemap[MAX_MEM / PAGE_SIZE / (sizeof(unsigned long)*8)];     // currently 2GB / 4kB / 8bits = 64kB

static inline unsigned frame_to_freemap_index(frame_t frame)
{
    return (frame / (sizeof(unsigned long)*8));        // DIV 64
}
static inline unsigned frame_to_freemap_bit(frame_t frame)
{
    return (frame % (sizeof(unsigned long)*8));      // MOD 64
}
frame_t get_free_frame()
{
    static frame_t last_frame = 0x400; // start at 4 MB (frame << PAGE_BITS = 0x400000)
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

/*  --------------------------------------------------------------------------- */

void map_free_frame_to_page(page_t page)
{
    // get free frame from get_free_frame()
    // walk through page tables (creating them as neccessary)
    // and enter the new frame to the given adr
    unsigned frame = get_free_frame();
    frame += page;
}

/*  --------------------------------------------------------------------------- */

mutex_t pt_mutex = MUTEX_INITIALIZER_LOCKED;

/*
 * In 64 bit mode, paging is enabled by start64.asm and the first 2 MB are identity-mapped.
 */
int mm_init()
{
    IFV printf("mm_init() 64 bit version\n");

    /* read address of page table PML4 (first level) from register cr3 */
#   if __x86_64__
    asm volatile ("mov %%cr3, %%rax" : "=a"(pd1));
#   else
    asm volatile ("mov %%cr3, %%eax" : "=a"(pd1));
#   endif
    IFVV printf("MM: pd1 = 0x%x\n", (ptr_t)pd1);

#   if __x86_64__
    /* checks of page table entries */
    IFVV printf("MM: pd1[0].p : %u\n", pd1[0].dir.p);
    IFVV printf("MM: pd1[1].p : %u\n", pd1[1].dir.p);

    pd2_entry_t *pd2 = (pd2_entry_t*)(ptr_t)(pd1[0].dir.frame << PAGE_BITS);
    IFVV printf("MM: pd1[0]->pd2 = 0x%x\n", (ptr_t)pd2);

    pd3_entry_t *pd3 = (pd3_entry_t*)(ptr_t)(pd2[0].dir.frame << PAGE_BITS);
    IFVV printf("MM: pd1[0]->pd2[0]->pd3 = 0x%x\n", (ptr_t)pd3);

    IFVV printf("MM: pd1[0]->pd2[0]->pd3[0].p = %d   adr = 0x%x\n", pd3[0].dir.p, pd3[0].dir.frame<<PAGE_BITS);
    IFVV printf("MM: pd1[0]->pd2[0]->pd3[1].p = %d   adr = 0x%x\n", pd3[1].dir.p, pd3[1].dir.frame<<PAGE_BITS);

    pt_entry_t *pt = (pt_entry_t*)(ptr_t)(pd3[0].dir.frame << PAGE_BITS);
    IFVV printf("MM: pd1[0]->pd2[0]->pd3[0]->pt = 0x%x\n", (ptr_t)pt);

    /*
     * initialize freemap[]
     */
    unsigned count = 0;
    memset(freemap, 0, sizeof(freemap));
    multiboot_info_t *mbi = (multiboot_info_t*)(ptr_t)hw_info.mb_adr;
    if (IS_BIT_SET(mbi->flags, 0)) {
        // mem_upper is from 1 MB up to first memory hole.
        // we start with this and add more later if we find mmap_ (flags[6])
        IFVV printf("flags[0] - mem_lower: 0x%x=%d  mem_upper: 0x%x=%d\n", mbi->mem_lower, mbi->mem_lower, mbi->mem_upper, mbi->mem_upper);
        unsigned u;     // start at 0x400000 (4 MB)
        for (u = 0x400; u < (mbi->mem_upper >> 2)+0x100; u++) {
            count++;
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
                for (u = (p->addr>>PAGE_BITS); u < ((p->addr + p->len) >> PAGE_BITS); u++) {
                    if (u >= 0x400) {
                        if (IS_BIT_CLEAR(freemap[frame_to_freemap_index(u)], frame_to_freemap_bit(u))) {
                            count++;
                            BIT_SET(freemap[frame_to_freemap_index(u)], frame_to_freemap_bit(u));
                        }
                    }
                }
            }
        }
    }
    IFV printf("MM: registered %u=0x%x free pages (%u MB)\n", count, count, count>>8);



    

    //int *p = (int*)0x00200000-4;    // 2 MB (- 4B) : access fine; 2 MB + 4B : page fault
    //*p = 0;
    
    mutex_unlock(&pt_mutex);    // pt_mutex was initialized in locked state, from now on, the mm is usable
#   endif
    return 0;
}

static unsigned next_virt_page = 0x400;

void *heap_alloc(unsigned nbr_pages)
{
    unsigned res;
    unsigned u;

    mutex_lock(&pt_mutex);

    res = next_virt_page;
    
    for (u = res; u < res+nbr_pages; u++) {
        map_free_frame_to_page(u);
    }
    next_virt_page += nbr_pages;

    mutex_unlock(&pt_mutex);
    return ((void*)(ptr_t)(res << PAGE_BITS));
}

