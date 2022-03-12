// S4 - A Minimal Interpreter

#include "s3.h"

#if __BOARD__ == PC

#ifdef __WINDOWS__
CELL millis() {
    return (CELL)GetTickCount();
}
CELL micros() {
    return (CELL)millis()*1000;
}
void delay(UCELL ms) { 
    Sleep((DWORD)ms);
}

int charAvailable() { return _kbhit(); }
int getChar() { return _getch(); }

#else
CELL millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (CELL)((ts.tv_sec * 1000) + (ts.tv_nsec / 1000000));
}
CELL micros() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (CELL)(ts.tv_nsec);
}
void delay(UCELL ms) { 
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL); 
}
#endif

static char buf[256];
static CELL t1, t2;

void printChar(const char c) { printf("%c", c); }
void printString(const char* str) { printf("%s", str); }


CELL getSeed() { return millis(); }

addr doCustom(byte ir, addr pc) {
    switch (ir) {
    case 'N': push(micros());          break;
    case 'T': push(millis());          break;
    case 'W': delay(pop());            break;
    case 'Q': isBye = 1;               break;
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}

void ok() {
    printString("\r\nS3:");
    dumpStack();
    printString(">");
}

void rtrim(char* cp) {
    char* x = cp;
    while (*x) { ++x; }
    --x;
    while (*x && (*x < 32) && (cp <= x)) { *(x--) = 0; }
}

void doHistory(char* str) {
    FILE* fp = fopen("history.txt", "at");
    if (fp) {
        fputs(str, fp);
        fclose(fp);
    }
}

int loop(FILE *fp) {
    if (!fp) { fp = stdin; }
    if (fgets(buf, sizeof(buf), fp) == buf) {
        if (fp == stdin) { doHistory(buf); }
        rtrim(buf);
        run((addr)buf);
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    vmInit();
    if (1 < argc) {
        FILE* fp = fopen(argv[1], "rt");
        while ((isBye == 0) && loop(fp)) {}
        fclose(fp);
    }
    while (!isBye) { ok(); loop(NULL); }
    return 0;
}

#endif
