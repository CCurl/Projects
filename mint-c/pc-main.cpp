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
    char buf[128];
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { ok(); }
    if (fgets(buf, 100, fp) == buf) {
        if (fp == stdin) { doHistory(buf); }
        rtrim(buf);
        loadCode(buf);
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = NULL; // input_pop();
    }
}

int main(int argc, char** argv) {
    vmInit();
    loadCode(":B32,;:N13,10,;");
    loadCode(":R26(i@97+,Bi@4*r@+@.N);");
    loadCode(":Ch@u@-\"._ bytes_(i@u@+`@\"58=(N),);");
    while (!isBye) { loop(); }
    return 0;
}
#endif
