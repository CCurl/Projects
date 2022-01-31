// A simple editor
#include "R4.h"

#ifdef __EDITOR__
#define MAX_Y     25
#define MAX_X    100
int xPos, yPos;
addr lines[25];
addr cur_cp;

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

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void CursonOn() { printString("\x1B[?25h"); }
void CursonOff() { printString("\x1B[?25l"); }

void findCRs(addr cp) {
    int i = 0;
    while (*cp) {
        if (*cp == '\n') {
            lines[i++] = cp; 
            if (MAX_Y <= i) { *(cp+1) = 0; return; }
        }
        ++cp;
    }
}

void showText(addr x) {
    int xp = 1, yp = 1;
    CursonOff();
    GotoXY(1, 1);
    for (addr cp = x; *cp; cp++) {
        if (*cp == '\n') {
            lines[yp-1] = cp;
            ++yp;
            xp = 1;
            printString("\x1B[K");
            if (MAX_Y <= yp) { *(cp + 1) = 0; }
        }
        if (cp == cur_cp) { xPos = xp; yPos = yp; }
        printChar(*cp);
    }
    GotoXY(xPos, yPos);
    CursonOn();
}

void charLeft() {
    if (1 < xPos) { GotoXY(--xPos, yPos); }
}

void charRight() {
    if (xPos < MAX_X) { GotoXY(++xPos, yPos); }
}

void charUp() {
    if (1 < yPos) { GotoXY(xPos, --yPos); }
}

void charDown() {
    if (yPos < MAX_Y) { GotoXY(xPos, ++yPos); }
}

int doEditorChar(char c, addr x) {
    if (c == 'q') { return 0; }
    if (c == 'a') { charLeft(); }
    if (c == 'd') { charRight(); }
    if (c == 'w') { charUp(); }
    if (c == 's') { charDown(); }
    return 1;
}

void doEditor(CELL b, addr x) {
    cur_cp = x;
    printStringF("editing block %d ... ", b);
    CLS();
    readBlock(b, x);
    showText(x);
    while (doEditorChar(getChar(), x)) {
        // showText();
    }
}
#else
void doEditor(CELL b, addr x) { printString("-noEdit-"); }
#endif
