// heap - A simple heap-based memory manager

#include "heap.h"

typedef struct heap_t { 
    struct heap_t *prev, *next;
    uint32_t sz, isFree;
    char *ptr; 
} HEAP_T;

static HEAP_T *start, *end;
static char *heap, *hh, *h_end;
static int heap_sz, sz_match;

void hm_init(char *buf, uint32_t buf_sz, uint32_t match) {
    heap = hh = buf;
    heap_sz = buf_sz;
    h_end = heap + heap_sz;
    start = end = NULL;
    sz_match = match ? match : 50;
}

static char *get_mem(sz) {
    if (hh+sz >= h_end) { return NULL; }
    char *ret = hh;
    hh += sz;
    return ret;
}

// Reuse a free entry that is "close enough" to the requested size,
// reserving larger entries for later requests that are larger.
static char *reuse(uint32_t sz) {
    HEAP_T *x = start;
    HEAP_T *best = NULL;
    while (x) {
        if (x->isFree && (sz <= x->sz)) {
            x->isFree = 0;
            #pragma warning( disable : 4018 )
            if ((x->sz - sz) <= sz_match) { return x->ptr; }
        }
        x = x->next;
    }
    return NULL;
}

char *hm_malloc(uint32_t sz) {
    char *ptr = reuse(sz);
    if (ptr) { return ptr; }
    while (sz & 0x03) { ++sz; }
    HEAP_T *x = (HEAP_T *)get_mem(sizeof(HEAP_T));
    if (!x) { return NULL; }
    ptr = get_mem(sz);
    if (!ptr) { hh -= sizeof(HEAP_T); return NULL; }
    x->prev = end;
    x->next = NULL;
    x->sz = sz;
    x->ptr = ptr;
    x->isFree = 0;
    if (!start) { start = x; }
    if (end) { end->next = x; }
    end = x;
    return ptr;
}

// Garbage collect one entry 'x'
static void do_gc(HEAP_T *x) {
    if (x->isFree) { return; }
    x->isFree = 1;
    while (end && end->isFree) {
        hh = (char*)end;
        end = end->prev;
    }
    if (end == NULL) { start = NULL; }
    else { end->next = NULL; }
}

void hm_free(char *ptr) {
    if (ptr == NULL) { return; }
    HEAP_T *x = start;
    while (x) {
        if (x->ptr == ptr) { do_gc(x); return; }
        if (x->ptr < ptr) { x = x->next; }
    else { return; }
    }
}
