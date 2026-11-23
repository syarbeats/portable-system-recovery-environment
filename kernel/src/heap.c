#include <psre/heap.h>
#include <stdint.h>

#define HEAP_BYTES (256U * 1024U)
#define HEAP_MAGIC 0x50535245U
#define ALIGN_UP(v, a) (((v) + ((a)-1)) & ~((a)-1))

struct __attribute__((aligned(16))) heap_block {
    size_t size;
    struct heap_block *next;
    uint32_t magic;
    uint8_t free;
};

static uint8_t heap_area[HEAP_BYTES] __attribute__((aligned(16)));
static struct heap_block *head;
static size_t free_bytes;

bool heap_initialize(void) {
    head = (struct heap_block *)heap_area;
    head->size = HEAP_BYTES - sizeof(*head);
    head->next = 0;
    head->magic = HEAP_MAGIC;
    head->free = 1;
    free_bytes = head->size;
    return true;
}

void *kmalloc(size_t size) {
    if (!head || size == 0) return 0;
    size = ALIGN_UP(size, 16);
    for (struct heap_block *b = head; b; b = b->next) {
        if (b->magic != HEAP_MAGIC) return 0;
        if (!b->free || b->size < size) continue;
        if (b->size >= size + sizeof(struct heap_block) + 16) {
            struct heap_block *split = (struct heap_block *)((uint8_t *)(b + 1) + size);
            split->size = b->size - size - sizeof(*split);
            split->next = b->next;
            split->magic = HEAP_MAGIC;
            split->free = 1;
            b->next = split;
            b->size = size;
            free_bytes -= sizeof(struct heap_block);
        }
        b->free = 0;
        free_bytes -= b->size;
        return (void *)(b + 1);
    }
    return 0;
}

void kfree(void *ptr) {
    if (!ptr || (uint8_t *)ptr < heap_area + sizeof(struct heap_block) ||
        (uint8_t *)ptr >= heap_area + HEAP_BYTES) return;
    struct heap_block *b = ((struct heap_block *)ptr) - 1;
    if (b->magic != HEAP_MAGIC || b->free) return;
    b->free = 1;
    free_bytes += b->size;
    for (struct heap_block *cur = head; cur && cur->next;) {
        if (cur->free && cur->next->free && cur->next->magic == HEAP_MAGIC) {
            cur->size += sizeof(struct heap_block) + cur->next->size;
            cur->next = cur->next->next;
            free_bytes += sizeof(struct heap_block);
        } else cur = cur->next;
    }
}

size_t heap_get_capacity(void) { return HEAP_BYTES; }
size_t heap_get_free_bytes(void) { return free_bytes; }
