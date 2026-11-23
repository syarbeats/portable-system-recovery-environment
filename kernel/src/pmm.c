#include <psre/pmm.h>
#include <psre/memory_map.h>
#include <psre/limine.h>

/* Manage up to 64 GiB of physical address space. Bits set in used[] are unavailable. */
#define PMM_MAX_BYTES (64ULL * 1024ULL * 1024ULL * 1024ULL)
#define PMM_MAX_PAGES (PMM_MAX_BYTES / PSRE_PAGE_SIZE)
#define PMM_BITMAP_BYTES (PMM_MAX_PAGES / 8ULL)
#define PMM_RESERVED_LOW_BYTES (1024ULL * 1024ULL)

static uint8_t used[PMM_BITMAP_BYTES];
static uint8_t allocated[PMM_BITMAP_BYTES];
static struct psre_pmm_stats stats;
static uint64_t search_hint;

static inline bool bit_get(const uint8_t *map, uint64_t n) {
    return (map[n >> 3] & (uint8_t)(1u << (n & 7))) != 0;
}
static inline void bit_set(uint8_t *map, uint64_t n) {
    map[n >> 3] |= (uint8_t)(1u << (n & 7));
}
static inline void bit_clear(uint8_t *map, uint64_t n) {
    map[n >> 3] &= (uint8_t)~(1u << (n & 7));
}

static void mark_free_range(uint64_t base, uint64_t length) {
    uint64_t start = base / PSRE_PAGE_SIZE + ((base % PSRE_PAGE_SIZE) != 0);
    uint64_t end = (base + length) / PSRE_PAGE_SIZE;
    if (start >= PMM_MAX_PAGES) return;
    if (end > PMM_MAX_PAGES) end = PMM_MAX_PAGES;
    for (uint64_t page = start; page < end; ++page) {
        if (bit_get(used, page)) {
            bit_clear(used, page);
            stats.free_pages++;
        }
    }
}

bool pmm_initialize(void) {
    const struct limine_memmap_response *map = memory_map_get();
    if (!map) return false;
    for (uint64_t i = 0; i < PMM_BITMAP_BYTES; ++i) {
        used[i] = 0xff;
        allocated[i] = 0;
    }
    stats.total_pages = PMM_MAX_PAGES;
    stats.free_pages = 0;
    stats.allocated_pages = 0;
    stats.managed_limit_bytes = PMM_MAX_BYTES;

    for (uint64_t i = 0; i < map->entry_count; ++i) {
        const struct limine_memmap_entry *e = map->entries[i];
        if (e && e->type == LIMINE_MEMMAP_USABLE)
            mark_free_range(e->base, e->length);
    }

    /* Keep low physical memory unavailable for firmware/legacy structures. */
    uint64_t low_pages = PMM_RESERVED_LOW_BYTES / PSRE_PAGE_SIZE;
    for (uint64_t page = 0; page < low_pages; ++page) {
        if (!bit_get(used, page)) { bit_set(used, page); stats.free_pages--; }
    }
    search_hint = low_pages;
    return stats.free_pages > 0;
}

uint64_t pmm_alloc_page(void) {
    if (stats.free_pages == 0) return 0;
    for (uint64_t pass = 0; pass < 2; ++pass) {
        uint64_t begin = pass == 0 ? search_hint : 1;
        uint64_t end = pass == 0 ? PMM_MAX_PAGES : search_hint;
        for (uint64_t page = begin; page < end; ++page) {
            if (!bit_get(used, page)) {
                bit_set(used, page);
                bit_set(allocated, page);
                stats.free_pages--;
                stats.allocated_pages++;
                search_hint = page + 1;
                if (search_hint >= PMM_MAX_PAGES) search_hint = 1;
                return page * PSRE_PAGE_SIZE;
            }
        }
    }
    return 0;
}

bool pmm_free_page(uint64_t address) {
    if ((address & (PSRE_PAGE_SIZE - 1)) != 0) return false;
    uint64_t page = address / PSRE_PAGE_SIZE;
    if (page >= PMM_MAX_PAGES || !bit_get(allocated, page)) return false;
    bit_clear(allocated, page);
    bit_clear(used, page);
    stats.free_pages++;
    if (stats.allocated_pages) stats.allocated_pages--;
    if (page < search_hint) search_hint = page;
    return true;
}

const struct psre_pmm_stats *pmm_get_stats(void) { return &stats; }


/* M011: contiguous DMA-capable page allocation, with an inclusive address ceiling. */
uint64_t pmm_alloc_contiguous_pages(uint64_t count, uint64_t max_address) {
    if (!count || count > PMM_MAX_PAGES || !stats.free_pages) return 0;
    uint64_t max_page = max_address / PSRE_PAGE_SIZE;
    if (max_page >= PMM_MAX_PAGES) max_page = PMM_MAX_PAGES - 1;
    if (max_page + 1 < count) return 0;
    uint64_t start = search_hint;
    if (start < 1) start = 1;
    for (unsigned pass=0; pass<2; ++pass) {
        uint64_t begin = pass == 0 ? start : 1;
        uint64_t end = pass == 0 ? max_page + 1 : (start < max_page + 1 ? start : max_page + 1);
        if (begin >= end) continue;
        for (uint64_t page=begin; page + count <= end; ++page) {
            uint64_t n=0;
            while (n<count && !bit_get(used,page+n)) ++n;
            if (n != count) { page += n; continue; }
            for (uint64_t i=0;i<count;i++) {
                bit_set(used,page+i); bit_set(allocated,page+i);
            }
            stats.free_pages-=count; stats.allocated_pages+=count;
            search_hint=page+count;
            if(search_hint>=PMM_MAX_PAGES)search_hint=1;
            return page*PSRE_PAGE_SIZE;
        }
    }
    return 0;
}
bool pmm_free_contiguous_pages(uint64_t address, uint64_t count) {
    if (!count || (address & (PSRE_PAGE_SIZE-1))) return false;
    uint64_t first=address/PSRE_PAGE_SIZE;
    if(first>=PMM_MAX_PAGES || count>PMM_MAX_PAGES-first)return false;
    for(uint64_t i=0;i<count;i++)
        if(!bit_get(allocated,first+i))return false;
    for(uint64_t i=0;i<count;i++){
        bit_clear(allocated,first+i);bit_clear(used,first+i);
    }
    stats.free_pages+=count;
    stats.allocated_pages-=count;
    if(first<search_hint)search_hint=first;
    return true;
}
