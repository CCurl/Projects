#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "s5.h"

typedef unsigned char byte;

#define STK_SZ 31

byte code[64*1024];
byte memory[128*1024];
byte regs[260];
long dstk[STK_SZ + 1];
addr rstk[STK_SZ + 1];
sys_t sys;

// These are used only be the PC version
static HANDLE hStdOut = 0;
static char input_fn[32];
static char runMode = 'i';

// These are in the <Arduino.h> file
long millis() { return GetTickCount(); }
int analogRead(int pin) { printStringF("-AR(%d)-", pin); return 0; }
void analogWrite(int pin, int val) { printStringF("-AW(%d,%d)-", pin, val); }
int digitalRead(int pin) { printStringF("-DR(%d)-", pin); return 0; }
void digitalWrite(int pin, int val) { printStringF("-DW(%d,%d)-", pin, val); }
void pinMode(int pin, int mode) { printStringF("-pinMode(%d,%d)-", pin, mode); }
void delay(DWORD ms) { Sleep(ms); }

void printString(const char* str) {
    DWORD n = 0, l = strlen(str);
    if (l) { WriteConsoleA(hStdOut, str, l, &n, 0); }
}

void ok() {
    printString("\r\ns5:"); dumpStack(0); printString(">");
}

void doHistory(const char* txt) {
    FILE* fp = NULL;
    fopen_s(&fp, "history.txt", "at");
    if (fp) {
        fprintf(fp, "%s", txt);
        fclose(fp);
    }
}

addr strToCode(addr loc, const char *txt, int NT) {
    char c = *(txt++);
    while (c) {
        if (c == 9) { c = ' '; }
        if (' ' <= c) { setCodeByte(loc++, c); }
        c = *(txt++);
    }
    if (NT) { setCodeByte(loc++, 0); }
    return loc;
}

void loop() {
    char tib[100];
    addr nTib = sys.code_sz - 100;
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { ok(); }
    if (fgets(tib, 100, fp) == tib) {
        if (fp == stdin) { doHistory(tib); }
        strToCode(nTib, tib, 1);
        run(nTib);
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = NULL;
        if (runMode == 'n') { isBye = 1; }
    }
}

void process_arg(char* arg)
{
    if (*arg == 'n') { runMode = 'n'; }
    else if (*arg == 'i') { runMode = 'i'; }
    else if ((*arg == 'f') && (*(arg + 1) == ':')) {
        arg = arg + 2;
        strcpy_s(input_fn, sizeof(input_fn), arg);
    }
    else if (*arg == '?') {
        printString("usage s5 [arguments] [code]\n");
        printString("  -s:[source-filename]\n");
        printString("  -i  Run in interactive mode (default)\n");
        printString("  -n  Run in non-interactive mode\n");
    }
    else { printf("unknown arg '-%s' (try s5 -?)\n", arg); }
}

int main(int argc, char** argv) {
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m; GetConsoleMode(hStdOut, &m);
    SetConsoleMode(hStdOut, (m | ENABLE_VIRTUAL_TERMINAL_PROCESSING));

    sys.code = code;
    sys.mem = memory;
    sys.reg = regs;
    sys.code_sz = (addr)32*1024;
    sys.mem_sz = 128*1024;
    sys.reg_rz = 260;
    sys.dstack = dstk;
    sys.rstack = rstk;
    sys.stack_sz = STK_SZ;

    vmInit(&sys);

    input_fn[0] = 0;
    input_fp = NULL;

    for (int i = 1; i < argc; i++)
    {
        char* cp = argv[i];
        if (*cp == '-') { process_arg(++cp); }
        else { 
            doHistory(argv[i]);
            doHistory("\n");
            strToCode(500, argv[i], 1);
            run(500);
        }
    }

    if (strlen(input_fn) > 0) {
        input_fp = fopen(input_fn, "rt");
    }

    if ((!input_fp) && (runMode == 'n')) { isBye = 1; }
    while (isBye == 0) { loop(); }
}

#endif // #define _WIN32
