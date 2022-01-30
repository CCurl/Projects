// A simple editor
#include "R4.h"

#ifdef __EDITOR__
void readBlock(int b, addr to) {
    char fn[24];
    sprintf(fn, "block-%03d.R4", b);
    FILE* fp = fopen(fn, "rt");
    if (fp) {
        int n = fread(to, 1, USER_SZ, fp);
        fclose(fp);
        *(to + n) = 0;
    } else {
        *to = 0;
    }
}

void GotoXY(int x, int y) { printStringF("%c[%d;%dH", 27, y, x); }
void CLS() { printStringF("%c[2J", 27); GotoXY(1,1); }

void doEditor(CELL b, addr x) {
    printStringF("editing block %d ... ", b);
    CLS();
    readBlock(b, x);
    for (addr cp = x; *cp; cp++) {
        printChar(*cp);
    }
    printString("-TODO-");
}
#else
void doEditor(CELL b, addr x) { printString("-noEdit-"); }
#endif
