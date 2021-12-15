// R4 - A Minimal Interpreter

#include "R4.h"

SYS_T sys;
byte ir, isBye = 0, isError = 0;
static char buf[24];
addr pc;
CELL n1, t1;
void push(CELL v) { if (sys.dsp < STK_SZ) { sys.dstack[++sys.dsp] = v; } }
CELL pop() { return (sys.dsp) ? sys.dstack[sys.dsp--] : 0; }

inline void rpush(addr v) { if (sys.rsp < STK_SZ) { sys.rstack[++sys.rsp] = v; } }
inline addr rpop() { return (sys.rsp) ? sys.rstack[sys.rsp--] : 0; }

static float fstack[STK_SZ+1];
static int fsp = 0;

#define fT fstack[fsp]
#define fN fstack[fsp-1]
#define fDROP1 fpop()
#define fDROP2 fpop(); fpop();
static void fpush(float v) { if (fsp < STK_SZ) { fstack[++fsp] = v; } }
static float fpop() { return (fsp) ? fstack[fsp--] : 0; }

#define lAt() (&sys.lstack[LSP])
inline LOOP_ENTRY_T* lpush() { if (LSP < STK_SZ) { ++LSP; } return lAt(); }
inline LOOP_ENTRY_T *ldrop() { if (0 < LSP) { --LSP; } return lAt(); }

void vmInit() {
    sys.dsp = sys.rsp = sys.lsp = fsp = 0;
    for (int i = 0; i < NUM_REGS; i++) { REG[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { USER[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { FUNC[i] = 0; }
    REG[6] = NUM_REGS;
    REG[7] = (CELL)&sys.user[0];
    REG[13] = NUM_FUNCS;
    REG[19] = (CELL)&sys;
    REG[20] = (CELL)&sys.user[0];
    REG[25] = USER_SZ;
}

void setCell(byte* to, CELL val) {
#ifdef _NEEDS_ALIGN_
    *(to++) = (byte)val; 
    for (int i = 1; i < CELL_SZ; i++) {
        val = (val >> 8);
        *(to++) = (byte)val;
    }
#else
    * ((CELL *)to) = val;
#endif
}

CELL getCell(byte* from) {
    CELL val = 0;
#ifdef _NEEDS_ALIGN_
    from += (CELL_SZ - 1);
    for (int i = 0; i < CELL_SZ; i++) {
        val = (val << 8) + *(from--);
    }
#else
    val = *((CELL*)from);
#endif
    return val;
}

void dumpStack() {
    for (UCELL i = 1; i <= sys.dsp; i++) {
        printStringF("%s%ld", (i > 1 ? " " : ""), (CELL)sys.dstack[i]);
    }
}

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

void skipTo(byte to) {
    while (*pc) {
        ir = *(pc++);
        if ((to == '"') && (ir != to)) { continue; }
        if (ir == to) { return; }
        if (ir == '\'') { ++pc; continue; }
        if (ir == '(') { skipTo(')'); continue; }
        if (ir == '[') { skipTo(']'); continue; }
        if (ir == '"') { skipTo('"'); continue; }
    }
    isError = 1;
}

void doIf() {
    CELL n = pop();
    if (n == 0) {
        skipTo(')');
        return;
    }
}

void doFor() {
    CELL t = (N < T) ? T : N;
    CELL f = (N < T) ? N : T;
    DROP2;
    LOOP_ENTRY_T *x = lpush();
    x->start = pc;
    INDEX = x->from = f;
    x->to = t;
    x->end = 0;
}

void doNext() {
    LOOP_ENTRY_T* x = lAt();
    x->from = ++INDEX;
    if (x->from <= x->to) {
        x->end = pc;
        pc = x->start;
    } else {
        INDEX = ldrop()->from;
    }
}

void loopExit(char c) {
    if (!LSP) { isError = 1; return; }
    LOOP_ENTRY_T* x = lAt();
    ldrop();
    if (x->end) { pc = x->end; }
    else { skipTo(c); }
}

int isHexNum(char a) {
    if (BetweenI(a, '0', '9')) { return a - '0'; }
    if (BetweenI(a, 'A', 'F')) { return a - 'A' + 10; }
    return -1;
}

int isOk(int exp, const char* msg) {
    isError = (exp == 0); if (isError) { printString(msg); }
    return (isError == 0);
}

void doFloat() {
    addr x;
    switch (*(pc++)) {
    case '<': fpush((float)pop());                              return;
    case '>': x = (addr)&fT; push(*(CELL*)x); fpop();           return;
    case '+': fN += fT; fDROP1;                                 return;
    case '-': fN -= fT; fDROP1;                                 return;
    case '*': fN *= fT; fDROP1;                                 return;
    case '/': if (isOk(fT!=0, "-0div-")) { fN /= fT; fDROP1; }  return;
    case '.': printStringF("%g", fpop());                       return;
    default:
        isError = 1;
        printStringF("-flt(%d)-", sizeof(fT));
    }
}

void doExt() {
    ir = *(pc++);
    switch (ir) {
    case 'r': push(rand());                        return;
    case 'X': if (*pc == 'R') { ++pc; vmInit(); }  return;
    default:
        pc = doCustom(ir, pc);
    }
}

addr run(addr start) {
    isError = 0;
    pc = start;
    LSP = 0;
    while (!isError && pc) {
        ir = *(pc++);
        switch (ir) {
        case 0:                                                    return pc;
        case ' ': while (*(pc) == ' ') { pc++; }                   break;  // 32
        case '!': setCell((byte*)T, N); DROP2;                     break;  // 33
        case '"': while (*(pc)!=ir) { printChar(*(pc++)); }; ++pc; break;  // 34
        case '#': push(T);                                         break;  // 35 (DUP)
        case '$': t1 = T; T = N; N = t1;                           break;  // 36 (SWAP)
        case '%': push(N);                                         break;  // 37 (OVER)
        case '&': t1 = pop(); T &= t1;                             break;  // 38
        case '\'': push(*(pc++));                                  break;  // 39
        case '(': doIf();                                          break;  // 40
        case ')': /* endIf() */                                    break;  // 41
        case '*': t1 = pop(); T *= t1;                             break;  // 42
        case '+': t1 = pop(); T += t1;                             break;  // 43
        case ',': printChar((char)pop());                          break;  // 44
        case '-': t1 = pop(); T -= t1;                             break;  // 45
        case '.': printStringF("%ld", (CELL)pop());                break;  // 46
        case '/': if (isOk(T, "-0div-")) { N /= T; DROP1; }        break;  // 47
        case '0': case '1': case '2': case '3': case '4':                  // 48-57
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0'); ir = *(pc);
            while (BetweenI(ir, '0', '9')) {
                T = (T * 10) + (ir - '0');
                ir = *(++pc);
            } break;
        case ':': if (BetweenI(T, 0, NUM_FUNCS - 1)) {
                FUNC[pop()] = pc;
                skipTo(';');
                HERE = (CELL)pc;
            } else { isError = 1; printString("-func#-"); }
            break;
        case ';': pc = rpop();                                     break;  // 59
        case '<': t1 = pop(); T = T < t1 ? 1 : 0;                  break;  // 60
        case '=': t1 = pop(); T = T == t1 ? 1 : 0;                 break;  // 61
        case '>': t1 = pop(); T = T > t1 ? 1 : 0;                  break;  // 62
        case '?': /* FREE */                                       break;  // 63
        case '@': T = getCell((byte*)T);                           break;  // 64
        case 'A': if (T < 0) { T = -T; }                           break;  // ABS
        case 'B': printChar(' ');                                  break;
        case 'C': t1 = pop();
            if (BetweenI(t1, 0, NUM_FUNCS - 1) && FUNC[t1]) {
                if (*pc != ';') { rpush(pc); }
                pc = FUNC[t1];
            } break;
        case 'D': --T;                                             break;
        case 'E': printString("\r\n");                             break;
        case 'F': doFloat();                                       break;
        case 'G':                                                  break;
        case 'H':                                                  break;
        case 'I': push(INDEX);                                     break;
        case 'J':                                                  break;
        case 'K':                                                  break;
        case 'L': t1 = pop(); T = (T << t1);                       break;
        case 'M': if (isOk(T, "-0div-")) { t1 = pop(); T %= t1; }  break;
        case 'N': T = -T;                                          break;
        case 'O':                                                  break;
        case 'P': ++T;                                             break;
        case 'Q':                                                  break;
        case 'R': t1 = pop(); T = (T >> t1);                       break;
        case 'S': if (T) { t1 = T; T = N % t1; N /= t1; }        // /MOD
                else { isError = 1; printString("-0div-"); }       break;
        case 'T':                                                  break;
        case 'U':                                                  break;
        case 'V':                                                  break;
        case 'W':                                                  break;
        case 'X':                                                  break;
        case 'Y':                                                  break;
        case 'Z':                                                  break;
        case '[': doFor();                                         break;  // 91
        case '\\': pop();                                          break;  // 92
        case ']': doNext();                                        break;  // 93
        case '^': t1 = pop(); T ^= t1;                             break;  // 94
        case '_': T = (T) ? 0 : 1;                                 break;  // 95
        case '`':                                                  break;  // 96
        case 'a':                                                  break;
        case 'b':                                                  break;
        case 'c': ir = *(pc++);
            if (ir == '@') { T = *(byte*)T; }
            if (ir == '!') { *(byte*)T = (byte)N; DROP2; }
            break;
        case 'd': setCell((addr)T, getCell((addr)T) - 1); DROP1;   break;
        case 'e':                                                  break;
        case 'f': ir = *(pc++);
            if (ir == '@') { T = (CELL)FUNC[T]; }
            if (ir == '!') { FUNC[T] = (addr)N; DROP2; }
            break;
        case 'g':                                                  break;
        case 'h': push(0); while (1) {
                t1 = isHexNum(*(pc));
                if (t1 < 0) { break; }
                T = (T * 16) + t1; ++pc;
            } break;
        case 'i':                                                  break;
        case 'j':                                                  break;
        case 'k':                                                  break;
        case 'l': loopExit(']');                                   break;
        case 'm':                                                  break;
        case 'n':                                                  break;
        case 'o':                                                  break;
        case 'p': setCell((addr)T, getCell((addr)T) + 1); DROP1;   break;
        case 'q':                                                  break;
        case 'r': if (isOk(isReg(T), "-reg-")) { T = (CELL)&REG[T]; } break;
        case 's':                                                  break;
        case 't':                                                  break;
        case 'u':                                                  break;
        case 'v': if (isOk(isReg(T), "-reg-")) { T = REG[T]; }     break;
        case 'w': loopExit('}');                                   break;
        case 'x': doExt();                                         break;
        case 'y':                                                  break;
        case 'z':                                                  break;
        case '{': { LOOP_ENTRY_T *x = lpush();                             // 123
                x->start = pc; x->end = 0;
                if (!T) { skipTo('}'); }
            } break;
        case '|': t1 = pop(); T |= t1;                             break;  // 124
        case '}': if (!T) { ldrop(); DROP1; }                              // 125
            else { lAt()->end = pc; pc = lAt()->start; }
            break;
        case '~': T = ~T;                                          break;  // 126
        }
    }
    return pc;
}
