#include "cf.h"

char *pc, *here;
char user[USER_SZ];
CELL stk[16];
char dsp = 0, rsp = 0;
void push(CELL v) { dsp = (dsp<15) ? dsp+1 : 0; stk[dsp] = v; }
CELL pop() { char x = dsp; dsp = (dsp==0) ? 15 : 0; return stk[x]; }

int charAvailable() { return _kbhit(); }
int getChar() { return _getch(); }
void printString(const char *s) { printf("%s", s); }
void printChar(char c) { printf("%c", c); }

void printStringF(const char* fmt, ...) {
    char buf[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

int main(int argc, char **argv) {
    here = user;
    Color(RED, 0);
    printString("hi ");
    Color(BLUE, 0);
    printString("there ");
    Color(WHITE, 0);
    doEditor();
    doCompiler(theBlock);
    return 1;
}