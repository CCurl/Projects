#ifndef __HEAP_H__

#include <stdint.h>

// NOTE: The 'match' parameter is a number that identifies how close a match to the
//       requested size a free entry should be in order to be reused.
//   The default value for this is 50 (when 0 is specified).
//   For example, if match=20, then it will not reuse an entry unless it is <= 20 bytes
//       larger than the requested size (entry-sz - req-sz <= 20).
extern void hm_init(char *buf, uint32_t buf_sz, uint32_t match);

// This will return NULL if there is no space available.
extern char *hm_malloc(uint32_t sz);

// This will do some rudimentary garbage-collecting, by removing all freed entries at the
//       end of the list of allocated entries, completely freeing up that space.
extern void hm_free(char *ptr);

#endif
