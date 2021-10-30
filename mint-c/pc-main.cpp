// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#ifdef _WIN32

#define  _CRT_SECURE_NO_WARNINGS

#include "mint.h"

FILE* input_fp;

void printChar(const char c) { printf("%c", c); }
void printString(const char* str) { printf("%s", str); }

addr doFile(addr pc) {
    printString("-notImpl-");
    return pc;
}

addr doPin(addr pc) {
    printString("-noPin-");
    return pc;
}

void ok() {
    printString("\r\nmint:(");
    dumpStack();
    printString(")>");
}

void rtrim(char *cp) {
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

void loop() {
    char *tib = (char *)HERE;
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { ok(); }
    if (fgets(tib, 100, fp) == tib) {
        if (fp == stdin) { doHistory(tib); }
        rtrim(tib);
        run((byte *)tib);
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = NULL; // input_pop();
    }
}

int main(int argc, char** argv) {
    vmInit();
    while (!isBye) { loop(); }
    return 0;
}
#endif
