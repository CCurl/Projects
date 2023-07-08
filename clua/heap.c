// heap - A heap-based memory manager

#include "heap.h"

#define NULL 0

typedef struct heap_t { 
    struct heap_t *prev, *next;
    int sz, isFree;
    char *ptr; 
} HEAP_T;

static HEAP_T *start, *end;
static char *heap;
static int hh, heap_sz;

void cm_init(char *buf, int sz) {
	heap = buf;
	heap_sz = sz;
	hh = 0;
	start = end = NULL;
}

char *heap_get(sz) {
	if (hh+sz >= heap_sz) { return NULL; }
	char *ret = &heap[hh];
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
// Merge with next and/or prev if it is also free
static void cm_gc(HEAP_T *x) {
    // printf("-gc(x:%p)-", x);
    if (x->isFree == 1) { return; }
    HEAP_T *p = x->prev, *n = x->next;
    // collect n into x?
    if (n && n->isFree) {
        // printf("-n:%p into x-", n);
	if (end == n) { end = x; }
        x->sz += n->sz + sizeof(HEAP_T);
	x->next = n->next;
	n = n->next;
        // printf("-x new sz=%d-", x->sz);
	if (n) { n->prev = x; }
    }
    // collect x into p?
    if (p && p->isFree) {
        // printf("-x into p:%p-", p);
	if (end == x) { end = p; }
        p->sz += x->sz + sizeof(HEAP_T);
	p->next = n;
        // printf("-p new sz=%d-", p->sz);
	if (n) { n->prev = p; }
    }
    x->isFree = 1;
    // printf("\n");
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
