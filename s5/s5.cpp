// S5 - a stack VM, inspired by Sandor Schneider's STABLE - https://w3group.de/stable.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "s5.h"

#define RN_SZ    2
#define BASE     &MEM[2]

byte isBye = 0, isError = 0;
char buf[100];
FILE* input_fp = NULL;
sys_t *sys;

#define T        DSTK[DSP]
#define N        DSTK[DSP-1]
#define R        RSTK[RSP]
#define L        sys->lsp
#define DROP1    pop()
#define DROP2    pop(); pop()

void push(long v) { if (DSP < SZ_STK) { DSTK[++DSP] = v; } }
long pop() { return (DSP > 0) ? DSTK[DSP--] : 0; }

void rpush(addr v) { if (RSP < SZ_STK) { RSTK[++RSP] = v; } }
addr rpop() { return (RSP > 0) ? RSTK[RSP--] : 0; }

void vmReset() {
    DSP = RSP = sys->lsp = 0;
    for (int i = 0; i < SZ_CODE; i++) { CODE[i] = 0; }
    for (int i = 0; i < SZ_MEM; i++) { MEM[i] = 0; }
    for (int i = 0; i < SZ_STK; i++) { REG[i] = 0; }
    CODE[HERE++] = ';';
}

void vmInit(sys_t *Sys) {
    sys = Sys;
    vmReset();
}

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

int hexNum(char x) {
    if (('0' <= x) && (x <= '9')) { return x - '0'; }
    if (('A' <= x) && (x <= 'F')) { return x - 'A' + 10; }
    if (('a' <= x) && (x <= 'f')) { return x - 'a' + 10; }
    return -1;
}

int regNum(char x, int isAlpha) {
    if ((isAlpha)  && ('A' <= x) && (x <= 'Z')) { return x - 'A'; }
    if ((!isAlpha) && ('0' <= x) && (x <= '9')) { return x - '0'; }
    isError = 1;
    return -1;
}

addr getRegNum(int pc, int msg) {
    int c1 = regNum(CODE[pc], 1);
    int c2 = regNum(CODE[pc+1], 0);
    if (isError) {
        if (msg) { printStringF("-%c%c:BadReg-", CODE[pc], CODE[pc+1]); }
        return 0;
    }
    addr n = (c2*26) + c1;
    if (SZ_REG <= n) {
        if (msg) { printStringF("-%d:RN_OOB-", n); }
        isError = 1;
        return 0;
    }
    return n;
}

addr doDefineQuote(addr pc) {
    int depth = 1;
    push(pc);
    while ((pc < SZ_CODE) && CODE[pc]) {
        char c = CODE[pc++];
        if (c == '{') { ++depth; }
        if (c == '}') {
            --depth;
            if (depth == 0) { return pc; }
        }
    }
    isError = 1;
    printString("-noQE-");
    return pc;
}

addr doBegin(addr pc) {
    rpush(pc);
    if (T == 0) {
        while ((pc < SZ_CODE) && (CODE[pc] != ']')) { pc++; }
    }
    return pc;
}

addr doWhile(addr pc) {
    if (T) { pc = R; }
    else { DROP1;  rpop(); }
    return pc;
}

addr doFor(addr pc) {
    if (L < 4) {
        LOOP_ENTRY_T* x = &LSTK[L];
        L++;
        x->pc = pc;
        x->to = pop();
        x->from = pop();
        if (x->to < x->from) {
            push(x->to);
            x->to = x->from;
            x->from = pop();
        }
    }
    return pc;
}

addr doNext(addr pc) {
    if (L < 1) { L = 0; }
    else {
        LOOP_ENTRY_T* x = &LSTK[L - 1];
        ++x->from;
        if (x->from <= x->to) { pc = x->pc; }
        else { L--; }
    }
    return pc;
}

addr doIJK(addr pc, int mode) {
    push(0);
    if ((mode == 1) && (0 < L)) { T = LSTK[L - 1].from; }
    if ((mode == 2) && (0 < L)) { T = LSTK[L - 2].from; }
    if ((mode == 3) && (0 < L)) { T = LSTK[L - 3].from; }
    return pc;
}

void dumpCode() {
    printStringF("\r\nCODE: size: %d bytes, HERE=%d", SZ_CODE, HERE);
    if (HERE == 0) { printString("\r\n(no code defined)"); return; }
    addr x = HERE;
    int ti = 0, npl = 20;
    char txt[32];
    for (addr i = 0; i < HERE; i++) {
        if ((i % npl) == 0) {
            if (ti) { txt[ti] = 0;  printStringF(" ; %s", txt); ti = 0; }
            printStringF("\n\r%05d: ", i);
        }
        txt[ti++] = (CODE[i] < 32) ? '.' : CODE[i];
        printStringF(" %3d", CODE[i]);
    }
    while (x % npl) {
        printString("    ");
        x++;
    }
    if (ti) { txt[ti] = 0;  printStringF(" ; %s", txt); }
}

void dumpStack(int hdr) {
    if (hdr) { printStringF("\r\nSTACK: size: %d ", SZ_STK); }
    printString("(");
    for (int i = 1; i <= DSP; i++) { printStringF("%s%ld", (i > 1 ? " " : ""), DSTK[i]); }
    printString(")");
}

void dumpRegs() {
    printStringF("\r\nREGISTERS: ");
    int n = 0;
    for (int i = 0; i < SZ_REG; i++) {
        if (MEM[i] == 0) { continue; }
        if (((n++) % 5) == 0) { printString("\r\n"); }
        char r1 = (i / 26) + '0';
        char r2 = (i % 26) + 'A';
        printStringF("%c%c: %-10ld  ", r2, r1, MEM[i]);
    }
}

void dumpAll() {
    dumpStack(1);   printString("\r\n");
    dumpRegs();     printString("\r\n");
    dumpCode();     printString("\r\n");
}

addr doFile(addr pc) {
    int ir = CODE[pc++];
    switch (ir) {
#ifdef __PC__
    case 'C':
        if (T) { fclose((FILE*)T); }
        DROP1;
        break;
    case 'O': {
        byte* md = MEM + pop();
        byte* fn = MEM + T;
        T = 0;
        fopen_s((FILE**)&T, (char *)fn, (char *)md);
    }
            break;
    case 'R': if (T) {
        long n = fread_s(buf, 2, 1, 1, (FILE*)T);
        T = ((n) ? buf[0] : 0);
        push(n);
    }
            break;
    case 'W': if (T) {
        FILE* fh = (FILE*)pop();
        buf[1] = 0;
        buf[0] = (byte)pop();
        fwrite(buf, 1, 1, fh);
    }
            break;
#endif
    }
    return pc;
}

addr doPin(addr pc) {
    int ir = CODE[pc++];
    long pin = pop(), val = 0;
    switch (ir) {
    case 'I': pinMode(pin, INPUT); break;
    case 'U': pinMode(pin, INPUT_PULLUP); break;
    case 'O': pinMode(pin, OUTPUT); break;
    case 'R': ir = CODE[pc++];
        if (ir == 'D') { push(digitalRead(pin)); }
        if (ir == 'A') { push(analogRead(pin)); }
        break;
    case 'W': ir = CODE[pc++]; val = pop();
        if (ir == 'D') { digitalWrite(pin, val); }
        if (ir == 'A') { analogWrite(pin, val); }
        break;
    }
    return pc;
}

addr doExt(addr pc) {
    byte ir = CODE[pc++];
    switch (ir) {
    case 'F': pc = doFile(pc);          break;
    case 'P': pc = doPin(pc);           break;
    case 'R': vmReset();                break;
    case 'S': DSP = 0;              break;
    case 'T': isBye = 1;                break;
    case 'I': ir = CODE[pc++];
        if (ir == 'A') { dumpAll(); }
        if (ir == 'C') { dumpCode(); }
        if (ir == 'R') { dumpRegs(); }
        if (ir == 'S') { dumpStack(0); }
        break;
    }
    return pc;
}

addr run(addr pc) {
    long t1, t2, t3;
    byte* bp;
    isError = 0;
    while (!isError && (0 < pc)) {
        byte ir = CODE[pc++];
        switch (ir) {
        case 0: RSP = 0; return -1;
        case ' ': while (CODE[pc] == ' ') { pc++; }         // 32
                break;
        case '!': if ((0 <= T) && (T < SZ_MEM)) { MEM[T] = N; }
                DROP2;    break;
        case '"': buf[1] = 0;                               // 34
            while ((pc < SZ_CODE) && (CODE[pc] != '"')) {
                buf[0] = CODE[pc++];
                printString(buf);
            }
            ++pc; break;
        case '#': push(T);               break;             // 35 (DUP)
        case '$': t1 = N; N = T; T = t1; break;             // 36 (SWAP)
        case '%': push(N);               break;             // 37 (OVER)
        case '&': t1 = pop(); T &= t1;   break;             // 38
        case '\'': push(CODE[pc++]);     break;             // 39
        case '(': pc = doBegin(pc);      break;             // 40
        case ')': pc = doWhile(pc);      break;             // 41
        case '*': t1 = pop(); T *= t1;   break;             // 42
        case '+': t1 = pop(); T += t1;   break;             // 43
        case '-': t1 = pop(); T -= t1;   break;             // 45
        case '/': t1 = pop(); 
            if (t1) { T /= t1; }
            else { isError = 1; }
            break;  // 47
        case ',': printStringF("%c", (char)pop());  break;  // 44
        case '.': printStringF("%ld", pop());       break;  // 46
        case '0': case '1': case '2': case '3': case '4':   // 48-57
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            t1 = CODE[pc] - '0';
            while ((0 <= t1) && (t1 <= 9)) {
                T = (T * 10) + t1;
                t1 = CODE[++pc] - '0';
            }
            break;
        case ':': /* FREE */         break;
        case ';': if (RSP == 0) { return pc; }
            pc = rpop();                             break;  // 59
        case '<': t1 = pop(); T = T < t1  ? 1 : 0;   break;  // 60
        case '=': t1 = pop(); T = T == t1 ? 1 : 0;   break;  // 61
        case '>': t1 = pop(); T = T > t1  ? 1 : 0;   break;  // 62
        case '?': t2 = pop(); t1 = pop(); t3 = pop();
            if ( t3 && t1) { rpush(pc); pc = (addr)t1; } // TRUE case
            if (!t3 && t2) { rpush(pc); pc = (addr)t2; } // FALSE case
            break;                                       // 63
        case '@': if ((0 <= T) && (T < SZ_MEM)) { T = MEM[T]; }
            break;
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': ir -= 'A';
            t1 = getRegNum(pc-1, 1);
            pc += 1;
            push(MEM[t1]); ir = CODE[pc];
            if (ir == '+') { ++pc; ++MEM[t1]; }
            if (ir == '-') { ++pc; --MEM[t1]; }
            if (ir == ':') { DROP1; ++pc; MEM[t1] = pop(); }
            break;
        case '[': pc = doFor(pc);               break;       // 91
        case '\\': DROP1;                       break;       // 92
        case ']': pc = doNext(pc);              break;       // 93
        case '^': rpush(pc); pc = (addr)pop();  break;       // 94
        case '_': push(T);                                   // 95
            while (CODE[pc] && (CODE[pc] != '_')) { MEM[T++] = CODE[pc++]; }
            ++pc; MEM[T++] = 0;
            break;
        case '`': t1 = HERE;                                // 96
            while (CODE[pc] && (CODE[pc] != '`')) {
                CODE[HERE++] = CODE[pc++];
            }
            if (CODE[pc]) { push(t1); ++pc; }
            break;
        case 'a': ir = CODE[pc++];
            bp = (byte*)T;
            if (ir == '@') { T = *bp; }
            if (ir == '!') { *bp = N & 0xff; DROP2; }
            break;
        case 'b': printString(" ");         break;
        case 'c': ir = CODE[pc++];
            bp = &MEM[T];
            if ((0 <= T) && (T < SZ_MEM)) {
                if (ir == '@') { T = *bp; }
                if (ir == '!') { *bp = N & 0xff; DROP2; }
            } break;
        case 'd': ir = CODE[pc++];
            bp = &CODE[T];
            if ((0 <= T) && (T < SZ_CODE)) {
                if (ir == '@') { T = *bp; }
                if (ir == '!') { *bp = N & 0xff; DROP2; }
            } break;
        case 'e': pc = (addr)pop();       break;
        case 'f': T = ~T;                 break;
        case 'g': /* FREE */              break;
        case 'h': push(0);
            t1 = hexNum(CODE[pc]);
            while (0 <= t1) {
                T = (T * 0x10) + t1;
                t1 = hexNum(CODE[++pc]);
            } break;
        case 'i': doIJK(pc, 1);           break;
        case 'j': doIJK(pc, 2);           break;
        case 'k': T *= 1000;              break;
        case 'l': t1 = pop();  // LOAD
#ifdef __PC__
            if (input_fp) { fclose(input_fp); }
            sprintf_s(buf, sizeof(buf), "block.%03ld", t1);
            fopen_s(&input_fp, buf, "rt");
#else
            printString("-l:pc only-");
#endif
            break;
        case 'm': ir = CODE[pc++];
            bp = (byte*)T;
            if (ir == '@') { T = *bp; }
            if (ir == '!') {
                t1 = N; DROP2;
                *(bp++) = ((t1) & 0xff);
                *(bp++) = ((t1 >> 8) & 0xff);
                *(bp++) = ((t1 >> 16) & 0xff);
                *(bp++) = ((t1 >> 24) & 0xff);
            } break;          // 97
        case 'n': printString("\r\n");          break;
        case 'o': T = -T;                       break;
        case 'p': T++;                          break;
        case 'q': T--;                          break;
        case 'r': N = N >> T; DROP1;            break;
        case 's': t2 = N; t1 = T;  // /MOD
            if (t1 == 0) { isError = 1; }
            else { N = (t2 / t1); T = (t2 % t1); }
            break;
        case 't': push(millis());               break;
        case 'u': if (T < 0) { T = -T; }        break;
        case 'v': N = N << T; DROP1;            break;
        case 'w': delay(pop());                 break;
        case 'x': pc = doExt(pc);               break;
        case 'y': /* FREE */                    break;
        case 'z':  if ((0 <= T) && (T < SZ_MEM)) { 
            bp = &MEM[pop()];
            printString((char*)bp); }
            break;
        case '{': pc = doDefineQuote(pc);    break;    // 123
        case '|': t1 = pop(); T |= t1;       break;    // 124
        case '}': if (0 < RSP) { pc = rpop(); }    // 125
                else { RSP = 0; return pc; }
            break;
        case '~': T = (T) ? 0 : 1;           break;    // 126
        }
    }
    return 0;
}

void setCodeByte(addr loc, char ch) {
    if ((0 <= loc) && (loc < SZ_CODE)) { CODE[loc] = ch; }
}

long registerVal(int reg) {
    if ((0 <= 'A') && (reg <= 'Z')) { return MEM[reg - 'A']; }
    if ((0 <= 'a') && (reg <= 'z')) { return MEM[reg - 'a']; }
    return 0;
}

addr functionAddress(const char *fn) {
    CODE[HERE+0] = fn[0];
    CODE[HERE+1] = fn[1];
    CODE[HERE+2] = fn[2];
    return getRegNum(HERE, 0);
}
