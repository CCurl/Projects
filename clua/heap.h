#ifndef __HEAP_H__

extern void cm_init(char *buf, int sz);
extern char *cm_malloc(int sz);
extern void cm_free(char *ptr);
extern char *heap_get(int sz);

#endif
