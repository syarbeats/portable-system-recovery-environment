#include <psre/memory_map.h>
#include <psre/serial.h>

static const struct limine_memmap_response *g_map;
static uint64_t g_hhdm;
static struct psre_memory_map_stats g_stats;

bool memory_map_initialize(void) {
    if (!psre_memmap_request.response || !psre_hhdm_request.response)
        return false;
    g_map = psre_memmap_request.response;
    g_hhdm = psre_hhdm_request.response->offset;
    if (!g_map->entries || g_map->entry_count == 0 || g_map->entry_count > 4096)
        return false;

    g_stats.entry_count = g_map->entry_count;
    for (uint64_t i = 0; i < g_map->entry_count; ++i) {
        const struct limine_memmap_entry *e = g_map->entries[i];
        if (!e) return false;
        if (e->base + e->length < e->base) return false;
        uint64_t end = e->base + e->length;
        if (end > g_stats.highest_address) g_stats.highest_address = end;
        if (e->type == LIMINE_MEMMAP_USABLE) g_stats.usable_bytes += e->length;
        else g_stats.reserved_bytes += e->length;
    }
    return true;
}

const struct limine_memmap_response *memory_map_get(void) { return g_map; }
uint64_t memory_map_hhdm_offset(void) { return g_hhdm; }
const struct psre_memory_map_stats *memory_map_get_stats(void) { return &g_stats; }
