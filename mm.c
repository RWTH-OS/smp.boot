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
static inline ptr_t offset1G(void * adr) 
{
    return ((ptr_t)adr) & ((INDEX_MASK<<(PAGE_BITS+INDEX_BITS))|(INDEX_MASK<<PAGE_BITS)|PAGE_MASK);
}
static inline ptr_t offset2M(void * adr) 
{
    return ((ptr_t)adr) & ((INDEX_MASK<<PAGE_BITS)|PAGE_MASK);
}
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
#else   // __x86_32__
static inline ptr_t offset4M(void * adr) 
{
    return ((ptr_t)adr) & ((INDEX_MASK<<PAGE_BITS)|PAGE_MASK);
}
static inline ptr_t pd1_index(void * adr) 
{
    return (((ptr_t)adr) >> (PAGE_BITS+INDEX_BITS)) & INDEX_MASK;
}
#endif  // __x86_??__

#define pt_num_entries (0x1000 / sizeof(pt_entry_t))

static inline void *page_to_adr(page_t page)
{
    return (void*)((ptr_t)page << PAGE_BITS);
}

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
#define FRAME_TYPE_PT   1
#define FRAME_TYPE_4k   2
#if __x86_64__
#define FRAME_TYPE_2M   3
#define FRAME_TYPE_1G   4
#else
#define FRAME_TYPE_4M   5
#endif
frame_t get_free_frame(unsigned type)
{
    static frame_t last_frame_pt = 0x400; // start at 4 MB (frame << PAGE_BITS = 0x400000)
    static frame_t last_frame_4k = 0x800; // start at 8 MB (frame << PAGE_BITS = 0x800000)

    if (type == FRAME_TYPE_4k) {
        IFVV printf("get_free_frame(4k): start at frame 0x%x ", last_frame_4k);
        while (IS_BIT_CLEAR(freemap[frame_to_freemap_index(last_frame_4k)], frame_to_freemap_bit(last_frame_4k))) {
            last_frame_4k++;
            if (last_frame_4k >= (MAX_MEM / PAGE_SIZE)) {
                printf("ERROR: out of memory!\n");
                smp_status('E');
                while (1) asm volatile ("hlt");
            }
        }
        BIT_CLEAR(freemap[frame_to_freemap_index(last_frame_4k)], frame_to_freemap_bit(last_frame_4k));
        IFVV printf("return frame 0x%x\n", last_frame_4k);
        return last_frame_4k;
    } else if (type == FRAME_TYPE_PT) {
        IFVV printf("get_free_frame(pt): start at frame 0x%x ", last_frame_pt);
        while (IS_BIT_CLEAR(freemap[frame_to_freemap_index(last_frame_pt)], frame_to_freemap_bit(last_frame_pt))) {
            last_frame_pt++;
            if (last_frame_pt >= (MAX_MEM / PAGE_SIZE)) {
                printf("ERROR: out of memory!\n");
                smp_status('E');
                while (1) asm volatile ("hlt");
            }
        }
        BIT_CLEAR(freemap[frame_to_freemap_index(last_frame_pt)], frame_to_freemap_bit(last_frame_pt));
        IFVV printf("return frame 0x%x\n", last_frame_pt);
        return last_frame_pt;
    }
    printf("ERROR: type %d not supported!\n", type);
    smp_status('E');
    while (1) asm volatile ("hlt");
}

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
            if (pd2[pd2_index(adr)].dir.ps == 1) {
                /* 1 GB huge page */
                return (((ptr_t)pd2[pd2_index(adr)].page.frame1G << (PAGE_BITS+2*INDEX_BITS)) + offset1G(adr));
            } else {
                /* read pd3 */
                pd3_entry_t *pd3 = (pd3_entry_t*)((ptr_t)pd2[pd2_index(adr)].dir.frame << PAGE_BITS);
                if (pd3[pd3_index(adr)].dir.p) {
                    if (pd3[pd3_index(adr)].dir.ps == 1) {
                        /* 2 MB huge page */
                        return (((ptr_t)pd3[pd3_index(adr)].page.frame2M << (PAGE_BITS+INDEX_BITS)) + offset2M(adr));
                    } else {
                        /* read pt */
                        pt_entry_t *pt = (pt_entry_t*)((ptr_t)pd3[pd3_index(adr)].dir.frame << PAGE_BITS);
                        if (pt[pt_index(adr)].page.p) {
                            return (((ptr_t)pt[pt_index(adr)].page.frame << PAGE_BITS) + offset(adr));
                        } else {
                            return 0;
                        }
                    }
                } else {
                    return 0;
                }
            }
        } else {
            return 0;
        }
    } else {
        return 0;
    }
#   else
    if (pd1[pd1_index(adr)].dir.p) {
        if (pd1[pd1_index(adr)].dir.ps == 1) {
            /* 4 MB huge page */
            return (((ptr_t)pd1[pd1_index(adr)].page.frame4M << (PAGE_BITS+INDEX_BITS)) + offset4M(adr));
        } else {
            pt_entry_t *pt = (pt_entry_t*)((ptr_t)pd1[pd1_index(adr)].dir.frame << PAGE_BITS);
            if (pt[pt_index(adr)].page.p) {
                return (((ptr_t)pt[pt_index(adr)].page.frame << PAGE_BITS) + offset(adr));
            } else {
                return 0;
            }
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

static void *map_temporary(frame_t frame) 
{
    const page_t tmp_page = 0x1FF;                              /* this page is mapped initially in 32 AND 64 bit mode */
    void * const tmp_map = (void*)(tmp_page << PAGE_BITS);      /* just below 2 MB */
#   if __x86_64__
        static pt_entry_t * const  pt = (pt_entry_t*)0x5000;    /* initialized there in start64.asm */
#   else
        static pt_entry_t * const  pt = (pt_entry_t*)0x5000;    /* ATTN: not initialized in 32 bit mode, yet! */
#   endif

    IFVV printf("map_temporary: frame 0x%x to adr 0x%x\n", frame, tmp_map);
    pt[tmp_page].page.frame = frame;
    pt[tmp_page].page.rw = 1;
    pt[tmp_page].page.p = 1;
    /* invalidate TLB for page containing the address tmp_map */
    asm volatile ("invlpg %0" : : "m"(*(int*)tmp_map));
    return tmp_map;
}

#if __x86_64__
#   define MAP_HUGE_2M     1
#   define MAP_HUGE_1G     2
#else
#   define MAP_HUGE_4M     4
#endif

static void map_frame_to_adr(frame_t frame, void *adr, unsigned flags)
{
    if (flags != 0) {
        printf("ERROR (map_frame_to_adr): flags %d not supported, yet!\n", flags);
        smp_status('E');
        while (1) asm volatile ("hlt");
    }
    IFV printf("map_frame_to_adr(frame=0x%x, adr=0x%x, flags=0x%x)\n", frame, adr, flags);

#   if __x86_64__
    unsigned ipd1 = pd1_index(adr);
    IFVV printf("map: pd1=0x%x ipd1=%u\n", pd1, ipd1);

    pd2_entry_t *pd2;
    if (pd1[ipd1].dir.p == 0) {
        /* we need a new pd2 */
        frame_t new_frame = get_free_frame(FRAME_TYPE_PT);
        IFVV printf("map: new pd2: 0x%x\n", new_frame);
        pd1[ipd1].dir.p = 1;
        pd1[ipd1].dir.rw = 1;
        pd1[ipd1].dir.frame = new_frame;
        pd2 = (pd2_entry_t*)map_temporary(pd1[ipd1].dir.frame);
        memset(pd2, 0, 4096);
    } else {
        pd2 = (pd2_entry_t*)map_temporary(pd1[ipd1].dir.frame);
    }
    /* pd1 now contains an entry for pd2 */

    unsigned ipd2 = pd2_index(adr);
    IFVV printf("map: pd2=0x%x ipd2=%u\n", pd2, ipd2);

    pd3_entry_t *pd3;
    if (pd2[ipd2].dir.p == 0) {
        /* we need a new pd3 */
        frame_t new_frame = get_free_frame(FRAME_TYPE_PT);
        IFVV printf("map: new pd3: 0x%x\n", new_frame);
        pd2[ipd2].dir.p = 1;
        pd2[ipd2].dir.rw = 1;
        pd2[ipd2].dir.frame = new_frame;
        pd3 = (pd3_entry_t*)map_temporary(pd2[ipd2].dir.frame);
        memset(pd3, 0, 4096);
    } else {
        pd3 = (pd3_entry_t*)map_temporary(pd2[ipd2].dir.frame);
    }
    /* pd2 now contains an entry for pd3 */
    
    unsigned ipd3 = pd3_index(adr);
    IFVV printf("map: pd3=0x%x ipd3=%u\n", pd3, ipd3);

    pt_entry_t *pt;
    if (pd3[ipd3].dir.p == 0) {
        /* we nedd a new pt */
        frame_t new_frame = get_free_frame(FRAME_TYPE_PT);
        IFVV printf("map: new pt: 0x%x\n", new_frame);
        pd3[ipd3].dir.frame = new_frame;
        pd3[ipd3].dir.rw = 1;
        pd3[ipd3].dir.p = 1;
        pt = (pt_entry_t*)map_temporary(pd3[ipd3].dir.frame);
        memset(pt, 0, 4096);
    } else {
        pt = (pt_entry_t*)map_temporary(pd3[ipd3].dir.frame);
    }
    /* pd3 now contains an entry for pt */

    unsigned ipt = pt_index(adr);
    IFVV printf("map: dt=0x%x ipt=%u\n", pt, ipt);
    if (pt[ipt].page.p == 0) {
        /* the page was not mapped before */
        IFVV printf("map: new page: 0x%x\n", frame);
        pt[ipt].page.frame = frame;
        pt[ipt].page.rw = 1;
        pt[ipt].page.p = 1;
        /* invalidate TLB for page containing the address just mapped */
        asm volatile ("invlpg %0" : : "m"(*((int*)((ptr_t)frame<<PAGE_BITS))));
    }
#   else
    unsigned ipd1 = pd1_index(adr);

    pt_entry_t *pt;
    if (pd1[ipd1].dir.p == 0) {
        /* we need a new pd2 */
        frame_t new_frame = get_free_frame(FRAME_TYPE_PT);
        IFVV printf("map_frame_to_adr: new pt: 0x%x\n", new_frame);
        pd1[ipd1].dir.frame = new_frame;
        pd1[ipd1].dir.rw = 1;
        pd1[ipd1].dir.p = 1;
        pt = (pt_entry_t*)map_temporary(pd1[ipd1].dir.frame);
    } else {
        pt = (pt_entry_t*)map_temporary(pd1[ipd1].dir.frame);
    }
    /* pd1 now contains an entry for pt */

    unsigned ipt = pt_index(adr);
    IFVV printf("map_frame_to_adr: ipt = %u\n", ipt);
    if (pt[ipt].page.p == 0) {
        /* the page was not mapped before */
        IFVV printf("map_frame_to_adr: new page: 0x%x\n", frame);
        pt[ipt].page.frame = frame;
        pt[ipt].page.rw = 1;
        pt[ipt].page.p = 1;
        /* invalidate TLB for page containing the address just mapped */
        asm volatile ("invlpg %0" : : "m"(*((int*)((ptr_t)frame<<PAGE_BITS))));
    }
#   endif
    IFVV printf("map_frame_to_adr: done\n");
}





/*  --------------------------------------------------------------------------- */


/*  --------------------------------------------------------------------------- */

mutex_t pt_mutex = MUTEX_INITIALIZER_LOCKED;

/*
 * In 64 bit mode, paging is enabled by start64.asm and the first 2 MB are identity-mapped.
 * In 32 bit mode, the paging not activated, yet.
 */
int mm_init()
{
    IFV printf("mm_init() \n");


#   if __x86_64__
    /* read address of page table PML4 (first level) from register cr3 */
    asm volatile ("mov %%cr3, %%rax" : "=a"(pd1));
#   else    /* 32 bit */

    pd1 = (pd1_entry_t*)0x1000;
    memset(pd1, 0, PAGE_SIZE);

    pt_entry_t* pt = (pt_entry_t*)0x2000;
    memset(pt, 0, PAGE_SIZE);
    pd1[0].dir.frame = ((ptr_t)pt >> PAGE_BITS);
    pd1[0].dir.rw = 1;
    pd1[0].dir.p = 1;

    unsigned u;
    for (u = 0; u < pt_num_entries; u++) {
        pt[u].page.frame = u;        /* identity paging for the first 4 MB */
        pt[u].page.rw = 1;
        pt[u].page.p = 1;
    }

    /* map APICs 0xfee00000 and 0xfec00000 */
    /* 0xfee00-000    
     * 0b1111 1110 11-10 0000 0000 - 0000 0000 0000
     * 0xfec00-000
     * 0b1111 1110 11-00 0000 0000 - 0000 0000 0000
     */
    pt = (pt_entry_t*)0x3000;
    pd1[1023-4].dir.frame = ((ptr_t)pt >> PAGE_BITS);
    pd1[1023-4].dir.rw = 1;
    pd1[1023-4].dir.p = 1;
    pt[0].page.frame = 0xfec00;
    pt[0].page.rw = 1;
    pt[0].page.p = 1;
    pt[512].page.frame = 0xfee00;
    pt[512].page.rw = 1;
    pt[512].page.p = 1;

    asm volatile ("mov %%eax, %%cr3" : : "a"(pd1));     /* set cr3 to page-directory */
    asm volatile ("mov %%cr0, %%eax "
            "\n\t or $0x80000000, %%eax "
            "\n\t mov %%eax, %%cr0" ::: "eax");          /*  activate paging with cr0[31] */
#   endif   /*  64/32 bit */

    IFVV printf("MM: pd1 = 0x%x\n", (ptr_t)pd1);

    /* checks of page table entries */
    IFVV printf("MM: pd1[0].p : %u\n", pd1[0].dir.p);
    IFVV printf("MM: pd1[1].p : %u\n", pd1[1].dir.p);

#   if __x86_64__
    pd2_entry_t *pd2 = (pd2_entry_t*)(ptr_t)(pd1[0].dir.frame << PAGE_BITS);
    IFVV printf("MM: pd1[0]->pd2 = 0x%x\n", (ptr_t)pd2);

    pd3_entry_t *pd3 = (pd3_entry_t*)(ptr_t)(pd2[0].dir.frame << PAGE_BITS);
    IFVV printf("MM: pd1[0]->pd2[0]->pd3 = 0x%x\n", (ptr_t)pd3);

    IFVV printf("MM: pd1[0]->pd2[0]->pd3[0].p = %d   adr = 0x%x\n", pd3[0].dir.p, pd3[0].dir.frame<<PAGE_BITS);
    IFVV printf("MM: pd1[0]->pd2[0]->pd3[1].p = %d   adr = 0x%x\n", pd3[1].dir.p, pd3[1].dir.frame<<PAGE_BITS);

    pt_entry_t *pt = (pt_entry_t*)(ptr_t)(pd3[0].dir.frame << PAGE_BITS);
#   else
    pt = (pt_entry_t*)(ptr_t)(pd1[0].dir.frame << PAGE_BITS);
#   endif

        IFVV printf("MM: pt = 0x%x\n", (ptr_t)pt);

    /*
     * initialize freemap[]
     */
    unsigned count = 0;
    memset(freemap, 0, sizeof(freemap));
    IFVV printf("freemap at 0x%x, sizeof(freemap): 0x%x\n", freemap, sizeof(freemap));

    multiboot_info_t *mbi = (multiboot_info_t*)(ptr_t)hw_info.mb_adr;
    if (IS_BIT_SET(mbi->flags, 0)) {
        // mem_upper is from 1 MB up to first memory hole.
        // we start with this and add more later if we find mmap_ (flags[6])
        // mem_lower and mem_upper are in kB
        IFVV printf("flags[0] - mem_lower: 0x%x=%d  mem_upper: 0x%x=%d\n", mbi->mem_lower, mbi->mem_lower, mbi->mem_upper, mbi->mem_upper);
        frame_t u;     // start at 0x400000 (4 MB)
        frame_t limit = mbi->mem_upper;
        limit = limit >> 2;
        if (limit > (MAX_MEM >> PAGE_BITS)) limit = MAX_MEM >> PAGE_BITS;
        for (u = 0x400; u < limit; u++) {
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
                    if (u >= 0x400 && u < (MAX_MEM >> PAGE_BITS)) {
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
    return 0;
}

static page_t next_virt_page = 0x400;

void *heap_alloc(unsigned nbr_pages)
{
    unsigned i;
    void *res;
    frame_t frame;

    mutex_lock(&pt_mutex);

    res = page_to_adr(next_virt_page);
    for (i = 0; i < nbr_pages; i++) {
        frame = get_free_frame(FRAME_TYPE_4k);
        map_frame_to_adr(frame, page_to_adr(next_virt_page), 0);
        next_virt_page++;
        printf("next_virt_page=0x%x\n", next_virt_page);
    }

    mutex_unlock(&pt_mutex);
    return res;
}

