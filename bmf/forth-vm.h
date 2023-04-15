#include "Shared.h"

// ------------------------------------------------------------------------------------------
// The VM
// ------------------------------------------------------------------------------------------

extern bool isEmbedded;
extern bool isBYE;
extern BYTE the_memory[];

// ------------------------------------------------------------------------------------------
extern void push(CELL);
// extern CELL pop();
// extern void rpush(CELL);
// extern CELL rpop();
extern void init_vm();
extern void cpu_loop(ADDR);