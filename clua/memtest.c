#include <stdio.h>
#include "heap.h"

char buf[1000];

int main() {
	cm_init(buf, sizeof(buf));
	char *x1 = cm_malloc(10); printf("x1: %p\n", x1);
	char *x2 = cm_malloc(10); printf("x2: %p\n", x2);
	cm_free(x2);
	char *x3 = heap_get(10); printf("x3: %p\n", x3);
	char *x4 = cm_malloc(10); printf("x4: %p\n", x4);
	cm_free(x1);
	cm_free(x2);
	cm_free(x3);
	cm_free(x4);
	x4 = cm_malloc(20); printf("x4: %p\n", x4);
	cm_free(0);
	printf("\n");
	return 0;
}

