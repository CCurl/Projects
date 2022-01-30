// A simple editor
#include "R4.h"

#ifdef __EDITOR__
void doEditor(CELL b, addr x) {
    printStringF("editing block %d ... ", b);
    printString("-TODO-");
}
#else
void doEditor(CELL b, addr x) { printString("-noEdit-"); }
#endif
