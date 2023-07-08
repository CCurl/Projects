//
// heap - A Heap based memory manager
//

#include <stdlib.h>
#include "heap.h"

// heap-entries
typedef struct heap_t { 
    struct heap_t *prev, *next;
    int sz;
    char *ptr; 
} HEAP_T;

static HEAP_T *entries;
static char *heap;
static int mh, heap_sz;

static void cm_init(char *buf, int sz) {
    heap = buf;
    heap_sz = sz;
    mh = 0;
    entries = NULL;
}

static char *cm_get(sz) {
    char *ret = &heap[mh];
    mh += sz;
    return ret;
}

static HEAP_T *cm_new(int sz) {
    HEAP_T *x = (HEAP_T *)cm_get(sizeof(HEAP_T));
    x->prev = NULL;
    x->next = entries;
    x->sz = sz;
    x->ptr = cm_get(sz);
    if (entries) { entries->prev = x; }
    entries = x;
    return;
}

char *cm_alloc(int sz) {
    if (entries == NULL) {
        entries = cm_new(sz);
        return entries;
    }
}

void cm_free(char *ptr) {
    if (ptr == NULL) { return; }
    HEAP_T *x = entries;
    while (x && (x->ptr >= ptr)) { x = x->next; }
    if (x && (x->ptr == ptr)) {
        printf("free this: %p", x);
    }
}
