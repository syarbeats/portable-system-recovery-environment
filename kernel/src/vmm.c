#include <psre/vmm.h>
#include <psre/memory_map.h>
#include <psre/pmm.h>
#include <psre/limine.h>
#include <stdint.h>

#define PAGE_PRESENT 0x001ULL
#define PAGE_LARGE   0x080ULL
#define PAGE_ADDR    0x000ffffffffff000ULL
static uint64_t cr3_value;
static uint64_t hhdm;

static inline uint64_t read_cr3(void) {
    uint64_t value;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(value));
    return value;
}
static inline volatile uint64_t *phys_table(uint64_t physical) {
    return (volatile uint64_t *)(uintptr_t)(hhdm + (physical & PAGE_ADDR));
}

bool vmm_initialize(void) {
    hhdm = memory_map_hhdm_offset();
    cr3_value = read_cr3() & PAGE_ADDR;
    return hhdm != 0 && cr3_value != 0;
}
uint64_t vmm_get_cr3(void) { return cr3_value; }

uint64_t vmm_translate(uint64_t va) {
    if (!cr3_value || !hhdm) return 0;
    volatile uint64_t *pml4 = phys_table(cr3_value);
    uint64_t e4 = pml4[(va >> 39) & 0x1ff];
    if (!(e4 & PAGE_PRESENT)) return 0;
    volatile uint64_t *pdpt = phys_table(e4 & PAGE_ADDR);
    uint64_t e3 = pdpt[(va >> 30) & 0x1ff];
    if (!(e3 & PAGE_PRESENT)) return 0;
    if (e3 & PAGE_LARGE) return (e3 & 0x000fffffc0000000ULL) | (va & 0x3fffffffULL);
    volatile uint64_t *pd = phys_table(e3 & PAGE_ADDR);
    uint64_t e2 = pd[(va >> 21) & 0x1ff];
    if (!(e2 & PAGE_PRESENT)) return 0;
    if (e2 & PAGE_LARGE) return (e2 & 0x000fffffffe00000ULL) | (va & 0x1fffffULL);
    volatile uint64_t *pt = phys_table(e2 & PAGE_ADDR);
    uint64_t e1 = pt[(va >> 12) & 0x1ff];
    if (!(e1 & PAGE_PRESENT)) return 0;
    return (e1 & PAGE_ADDR) | (va & 0xfffULL);
}


/* M013: guarded 4-level page-table MMIO mapper.
 * The fixed high-half window is separate from the Limine HHDM and kernel image.
 * Page tables are allocated from PMM and accessed through the established HHDM.
 */
#define MMIO_PAGE_SIZE       4096ULL
#define MMIO_PAGE_MASK       0x000FFFFFFFFFF000ULL
#define MMIO_PRESENT         0x001ULL
#define MMIO_WRITE           0x002ULL
#define MMIO_PWT             0x008ULL
#define MMIO_PCD             0x010ULL
#define MMIO_NX              (1ULL << 63)
#define MMIO_SLOT_BASE       0xFFFFC10000000000ULL
#define MMIO_SLOT_SIZE       0x10000000ULL /* 256 MiB per reservation */
#define MMIO_SLOT_COUNT      32u
#define MMIO_MAX_MAPPINGS    32u

typedef struct {
    bool active;
    void *returned_address;
    uint64_t requested_size;
    uint64_t mapped_virtual;
    uint64_t page_count;
} mmio_mapping_t;

static mmio_mapping_t mmio_mappings[MMIO_MAX_MAPPINGS];
static uint32_t mmio_active_count;

static void zero_page(volatile uint64_t *page) {
    for (uint32_t i=0;i<512;i++) page[i]=0;
}
static volatile uint64_t *table_from_phys(uint64_t physical) {
    if (!hhdm || (physical & (MMIO_PAGE_SIZE-1))) return 0;
    return (volatile uint64_t *)(uintptr_t)(hhdm+(physical&MMIO_PAGE_MASK));
}
static uint64_t alloc_table(void) {
    uint64_t physical=pmm_alloc_page();
    if (!physical) return 0;
    volatile uint64_t *table=table_from_phys(physical);
    if (!table) { (void)pmm_free_page(physical); return 0; }
    zero_page(table);
    return physical;
}
static volatile uint64_t *walk_pte(uint64_t va,bool create) {
    if (!cr3_value || !hhdm) return 0;
    volatile uint64_t *table=table_from_phys(cr3_value);
    if (!table)return 0;
    const uint16_t index[4]={
        (uint16_t)((va>>39)&0x1FFu),
        (uint16_t)((va>>30)&0x1FFu),
        (uint16_t)((va>>21)&0x1FFu),
        (uint16_t)((va>>12)&0x1FFu)
    };
    for (unsigned level=0;level<3;level++) {
        uint64_t entry=table[index[level]];
        if (!(entry&MMIO_PRESENT)) {
            if (!create)return 0;
            uint64_t child=alloc_table();
            if (!child)return 0;
            entry=child|MMIO_PRESENT|MMIO_WRITE;
            table[index[level]]=entry;
        } else {
            /* Do not split large mappings or broaden existing page permissions. */
            if ((entry&(1ULL<<7)) || !(entry&MMIO_WRITE)) return 0;
        }
        table=table_from_phys(entry&MMIO_PAGE_MASK);
        if(!table)return 0;
    }
    return &table[index[3]];
}
static bool virtual_page_has_mapping(uint64_t va) {
    if(!cr3_value || !hhdm)return false;
    volatile uint64_t *table=table_from_phys(cr3_value);
    if(!table)return false;
    const uint16_t index[4]={
        (uint16_t)((va>>39)&0x1FFu),
        (uint16_t)((va>>30)&0x1FFu),
        (uint16_t)((va>>21)&0x1FFu),
        (uint16_t)((va>>12)&0x1FFu)
    };
    for(unsigned level=0;level<3;level++) {
        uint64_t entry=table[index[level]];
        if(!(entry&MMIO_PRESENT))return false;
        if(level>0 && (entry&(1ULL<<7)))return true;
        table=table_from_phys(entry&MMIO_PAGE_MASK);
        if(!table)return true; /* Treat malformed/large mappings as occupied. */
    }
    return (table[index[3]]&MMIO_PRESENT)!=0;
}
static void invalidate_page(uint64_t va) {
    __asm__ volatile("invlpg (%0)"::"r"(va):"memory");
}
static bool canonical_kernel_address(uint64_t va) {
    /* x86-64 48-bit canonical high-half address. */
    return (va>>48)==0xFFFFu && (va&(1ULL<<47))!=0;
}
static bool nx_enabled(void) {
    uint32_t lo,hi;
    __asm__ volatile("rdmsr":"=a"(lo),"=d"(hi):"c"(0xC0000080u));
    (void)lo;
    return (hi&(1u<<11))!=0;
}

void *vmm_map_mmio(uint64_t physical_address,uint64_t size) {
    if(!cr3_value || !hhdm || !physical_address || !size)return 0;
    uint64_t offset=physical_address&(MMIO_PAGE_SIZE-1);
    uint64_t physical_base=physical_address&~(MMIO_PAGE_SIZE-1);
    if(size>UINT64_MAX-offset)return 0;
    uint64_t span=size+offset;
    if(span>MMIO_SLOT_SIZE)return 0;
    if(physical_base>MMIO_PAGE_MASK ||
       span-1>MMIO_PAGE_MASK-physical_base)return 0;
    uint64_t pages=(span+MMIO_PAGE_SIZE-1)/MMIO_PAGE_SIZE;
    if(!pages || pages>MMIO_SLOT_SIZE/MMIO_PAGE_SIZE)return 0;
    uint64_t max_pages=((MMIO_PAGE_MASK-physical_base)/MMIO_PAGE_SIZE)+1;
    if(pages>max_pages)return 0;

    /* Fail closed if the requested device window overlaps usable system RAM. */
    const struct limine_memmap_response *memmap=memory_map_get();
    if(!memmap || !memmap->entries)return 0;
    uint64_t map_end=physical_base+pages*MMIO_PAGE_SIZE;
    for(uint64_t i=0;i<memmap->entry_count;i++) {
        const struct limine_memmap_entry *entry=memmap->entries[i];
        if(!entry || entry->type!=LIMINE_MEMMAP_USABLE)continue;
        if(entry->base+entry->length<entry->base)return 0;
        uint64_t entry_end=entry->base+entry->length;
        if(physical_base<entry_end && entry->base<map_end)return 0;
    }

    uint32_t record=MMIO_MAX_MAPPINGS;
    for(uint32_t i=0;i<MMIO_MAX_MAPPINGS;i++)
        if(!mmio_mappings[i].active){record=i;break;}
    if(record==MMIO_MAX_MAPPINGS)return 0;

    uint64_t chosen=0;
    for(uint32_t slot=0;slot<MMIO_SLOT_COUNT;slot++) {
        uint64_t candidate=MMIO_SLOT_BASE+(uint64_t)slot*MMIO_SLOT_SIZE;
        if(!canonical_kernel_address(candidate) ||
           !canonical_kernel_address(candidate+MMIO_SLOT_SIZE-1))continue;
        bool free=true;
        for(uint64_t p=0;p<pages;p++) {
            if(virtual_page_has_mapping(candidate+p*MMIO_PAGE_SIZE)){
                free=false;break;
            }
        }
        if(free){chosen=candidate;break;}
    }
    if(!chosen)return 0;

    uint64_t flags=MMIO_PRESENT|MMIO_WRITE|MMIO_PCD|MMIO_PWT;
    if(nx_enabled())flags|=MMIO_NX;
    uint64_t mapped=0;
    for(;mapped<pages;mapped++) {
        volatile uint64_t *pte=walk_pte(chosen+mapped*MMIO_PAGE_SIZE,true);
        if(!pte || (*pte&MMIO_PRESENT))break;
        *pte=(physical_base+mapped*MMIO_PAGE_SIZE)|flags;
        invalidate_page(chosen+mapped*MMIO_PAGE_SIZE);
    }
    if(mapped!=pages) {
        for(uint64_t p=0;p<mapped;p++) {
            volatile uint64_t *pte=walk_pte(chosen+p*MMIO_PAGE_SIZE,false);
            if(pte)*pte=0;
            invalidate_page(chosen+p*MMIO_PAGE_SIZE);
        }
        return 0;
    }

    void *result=(void *)(uintptr_t)(chosen+offset);
    mmio_mappings[record]=(mmio_mapping_t){
        .active=true,.returned_address=result,.requested_size=size,
        .mapped_virtual=chosen,.page_count=pages
    };
    mmio_active_count++;
    return result;
}

bool vmm_unmap_mmio(void *virtual_address,uint64_t size) {
    if(!virtual_address || !size)return false;
    for(uint32_t i=0;i<MMIO_MAX_MAPPINGS;i++) {
        mmio_mapping_t *m=&mmio_mappings[i];
        if(!m->active || m->returned_address!=virtual_address ||
           m->requested_size!=size)continue;
        for(uint64_t p=0;p<m->page_count;p++) {
            uint64_t va=m->mapped_virtual+p*MMIO_PAGE_SIZE;
            volatile uint64_t *pte=walk_pte(va,false);
            if(!pte || !(*pte&MMIO_PRESENT))return false;
        }
        for(uint64_t p=0;p<m->page_count;p++) {
            uint64_t va=m->mapped_virtual+p*MMIO_PAGE_SIZE;
            volatile uint64_t *pte=walk_pte(va,false);
            *pte=0;
            invalidate_page(va);
        }
        m->active=false;
        m->returned_address=0;
        m->requested_size=0;
        m->mapped_virtual=0;
        m->page_count=0;
        if(mmio_active_count)mmio_active_count--;
        /* Intermediate page-table pages are retained for safe reuse. */
        return true;
    }
    return false;
}
uint32_t vmm_mmio_active_mappings(void){return mmio_active_count;}
