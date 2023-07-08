#ifndef __HEAP_H__

#define HEAP_SZ 100000
#define ALLOC_SZ 1000

char *cm_alloc(int sz);
void cm_free(char *ptr);

#endif