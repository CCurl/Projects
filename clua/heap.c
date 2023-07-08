// heap - A heap-based memory manager

#include "heap.h"

#define NULL 0

typedef struct heap_t { 
    struct heap_t *prev, *next;
    int sz, isFree;
    char *ptr; 
} HEAP_T;

static HEAP_T *start, *end;
static char *heap, *hh, *h_end;
static int heap_sz;

void cm_init(char *buf, int sz) {
	heap = hh = buf;
	heap_sz = sz;
    h_end = heap + heap_sz;
	start = end = NULL;
}

char *heap_get(sz) {
	if (hh+sz >= h_end) { return NULL; }
	char *ret = hh;
	hh += sz;
	return ret;
}

static char *reuse(int sz) {
    HEAP_T *x = start;
    while (x) {
        if (x->isFree && (sz <= x->sz)) {
            x->isFree = 0;
	    return x->ptr;
        }
        x = x->next;
    }
    return NULL;
}

char *cm_malloc(int sz) {
    char *ptr = reuse(sz);
    if (ptr) { return ptr; }
    while (sz % 4) { ++sz; }
    HEAP_T *x = (HEAP_T *)heap_get(sizeof(HEAP_T));
    if (!x) { return NULL; }
    ptr = heap_get(sz);
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
static void cm_gc(HEAP_T *x) {
    // printf("-gc(x:%p)-", x);
    if (x->isFree == 1) { return; }
    x->isFree = 1;
    while (end && end->isFree) {
        hh = end;
        end = end->prev;
    }
    if (end == NULL) { start = NULL; }
    else { end->next = NULL; }
}

void cm_free(char *ptr) {
    if (ptr == NULL) { return; }
    HEAP_T *x = start;
    while (x) {
        if (x->ptr == ptr) { cm_gc(x); return; }
        if (x->ptr < ptr) { x = x->next; }
	else { return; }
    }
}
