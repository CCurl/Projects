// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include "R4.h"

#ifdef __PC__

#ifdef __WINDOWS__
CELL doMillis() { return (CELL)GetTickCount(); }
CELL doMicros() { return (CELL)doMillis()*1000; }
void doDelay(CELL ms) { Sleep(ms); }
#else
CELL doMillis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (CELL)((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
}
CELL doMicros() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (CELL)(ts.tv_nsec);
}
void doDelay(CELL ms) { 
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL); 
}
#endif

static byte fdsp = 0;
static FILE* fstack[STK_SZ+1];
static char buf[256];
static CELL t1, t2;

FILE* input_fp;
void fpush(FILE* v) { if (fdsp < STK_SZ) { fstack[++fdsp] = v; } }
FILE* fpop() { return (fdsp) ? fstack[fdsp--] : 0; }

void printChar(const char c) { printf("%c", c); }
void printString(const char* str) { printf("%s", str); }
CELL getSeed() { return doMillis(); }

int getChar() { return _getch(); }
int charAvailable() { return _kbhit(); }

addr doCustom(byte ir, addr pc) {
    switch (ir) {
    case 'Q': isBye = 1;                       break;
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}

void ok() {
    printString("\r\nR4:(");
    dumpStack();
    printString(")>");
}

void rtrim(char* cp) {
    char* x = cp;
    while (*x) { ++x; }
    --x;
    while (*x && (*x < 32) && (cp <= x)) { *(x--) = 0; }
}

void loadCode(const char* src) {
    addr here = (addr)HERE;
    addr tib = here;
    while (*src) {
        *(tib++) = *(src++);
    }
    *tib = 0;
    run(here);
}

void doHistory(char* str) {
    FILE* fp = fopen("history.txt", "at");
    if (fp) {
        fputs(str, fp);
        fclose(fp);
    }
}

void loop() {
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { ok(); }
    if (fgets(buf, sizeof(buf), fp) == buf) {
        if (fp == stdin) { doHistory(buf); }
        rtrim(buf);
        loadCode(buf);
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = fpop();
    }
}

int main(int argc, char** argv) {
    vmInit();
    loadCode("0 fB");
    while (!isBye) { loop(); }
    return 0;
}

#endif
