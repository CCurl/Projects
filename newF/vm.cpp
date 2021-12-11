// vm - A minimal forth VM

#include "newF.h"

SYS_T sys;
byte ir, isBye = 0, isError = 0;
static char buf[96];
addr pc;
CELL t, n, INDEX;

void push(CELL v) { if (sys.dsp < STK_SZ) { sys.dstack[++sys.dsp] = v; } }
CELL pop() { return (sys.dsp) ? sys.dstack[sys.dsp--] : 0; }

inline void rpush(addr v) { if (sys.rsp < STK_SZ) { sys.rstack[++sys.rsp] = v; } }
inline addr rpop() { return (sys.rsp) ? sys.rstack[sys.rsp--] : 0; }

#define lAt() (&sys.lstack[LSP])
inline LOOP_ENTRY_T* lpush() { if (LSP < STK_SZ) { ++LSP; } return lAt(); }
inline LOOP_ENTRY_T *ldrop() { if (0 < LSP) { --LSP; } return lAt(); }

void vmInit() {
    sys.dsp = sys.rsp = sys.lsp = 0;
    for (int i = 0; i < USER_SZ; i++) { USER[i] = 0; }
}

void setCell(addr to, CELL val) {
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

CELL getCell(addr from) {
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

void setWord(addr to, CELL val) {
    *(to+1) = (byte)(val >> 8);
    *(to) = (byte)(val);
}

CELL getWord(addr from) {
    return (*(from+1) << 8) | (*from);
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

CELL doRand() {
    static CELL seed = 0;
    if (seed == 0) { seed = getSeed(); }
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return (seed < 0) ? -seed : seed;
}

void doExt() {
    ir = *(pc++);
    switch (ir) {
    case 'C': rpush(pc);                        // fall thru to 'J'
    case 'J': pc = (addr)pop();                    return;
    case 'R': vmInit();                            return;
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
        case 1: push(*(pc++));                          break;
        case 2: push(getWord(pc)); pc += 2;             break;
        case 4: push(getCell(pc)); pc += CELL_SZ;       break;
        case 5: rpush(pc + CELL_SZ);                      // CALL - NO BREAK!
        case 6: pc = (addr)getCell(pc);                 break;  // JUMP
        case 7: ir = *(pc++); if (pop()) { pc += ir; }  break;  // 0BRANCH
            break;
        case ' ': while (*(pc) == ' ') { pc++; }        break;  // 32
        case '!': setCell((byte*)T, N); DROP2;          break;  // 33
        case '"': while (*(pc) != ir) { printChar(*(pc++)); };  // 34
                ++pc; break;
        case '#': push(T);                              break;  // 35 (DUP)
        case '$': t = N; N = T; T = t;                  break;  // 36 (SWAP)
        case '%': push(N);                              break;  // 37 (OVER)
        case '&': t = pop(); T &= t;                    break;  // 38
        case '\'': push(*(pc++));                       break;  // 39
        case '(': doIf();                               break;  // 40
        case ')': /* endIf() */                         break;  // 41
        case '*': t = pop(); T *= t;                    break;  // 42
        case '+': t = pop(); T += t;                    break;  // 43
        case ',': printChar((char)pop());               break;  // 44
        case '-': t = pop(); T -= t;                    break;  // 45
        case '.': printStringF("%ld", (CELL)pop());     break;  // 46
        case '/': if (T) { N /= T; DROP1; }                     // 47
                else { isError = 1;  printString("-0div-"); }
                break;
        case ':': 
            break;
        case ';': pc = rpop();                          break;  // 59
        case '<': t = pop(); T = (T  < t) ? 1 : 0;      break;  // 60
        case '=': t = pop(); T = (T == t) ? 1 : 0;      break;  // 61
        case '>': t = pop(); T = (T  > t) ? 1 : 0;      break;  // 62
        case '?': break;                                        // 63
        case '@': T = getCell((byte*)T);                break;  // 64
        case 'A': break;
        case 'B': break;
        case 'C': *(byte *)T = (byte)N; DROP2;          break;
        case 'D': if (T) { N = N % T; DROP1; }                  // MODULO
            else { printString("-0div-"); isError= 1; } break;
        case 'E': break;
        case 'F': break;
        case 'G': break;
        case 'H': break;
        case 'I': push((CELL)&INDEX);                   break; // I
        case 'J': break;
        case 'K': break;
        case 'L': t = pop(); T = (T << t);              break;
        case 'M': --T;                                  break;
        case 'N': T = -T;                               break;
        case 'O': break;
        case 'P': ++T;                                  break;
        case 'Q': break;
        case 'R': t = pop(); T = (T >> t);              break;
        case 'S': t = T; n = N;                                 // /MOD
            if (t == 0) { isError = 1; printString("-0div-"); }
            else { N = (n / t); T = (n % t); }          break;
        case 'T': break;
        case 'U': T = (T < 0) ? -T : T;                 break;
        case 'V': break;
        case 'W': break;
        case 'X': if (LSP) {
            t = (CELL)sys.lstack[--LSP].end;
            if (t) { pc = (addr)t; }
            else { skipTo('}'); }
        } break;
        case 'Y': break;
        case 'Z': break;
        case '[': doFor();                              break;  // 91
        case '\\': DROP1;                               break;  // 92
        case ']': doNext();                             break;  // 93
        case '^': t = pop(); T ^= t;                    break;  // 94
        case '_': T = (T) ? 0 : 1;                      break;  // 95
        case '`': doExt();                              break;  // 96
        case 'a': break;
        case 'b': break;
        case 'c': T = *(byte *)T;                       break;
        case 'd': break;
        case 'e': break;
        case 'f': break;
        case 'g': break;
        case 'h': break;
        case 'i': break;
        case 'j': break;
        case 'k': break;
        case 'l': break;
        case 'm': break;
        case 'n': break;
        case 'o': break;
        case 'p': break;
        case 'q': break;
        case 'r': push(doRand()); break;
        case 's': break;
        case 't': break;
        case 'u': break;
        case 'v': break;
        case 'w': break;
        case 'x': break;
        case 'y': break;
        case 'z': break;
        case '{': lpush()->start = pc;                  break;
        case '|': t = pop(); T |= t;                    break;  // 124
        case '}': if (pop() == 0) { ldrop(); }                  // 125
                else { lAt()->end = pc; pc = lAt()->start; }
            break;
        case '~': T = ~T;                               break;  // 126
        default: printStringF("-inv:%d-", ir); isError = 1;
        }
    }
    return pc;
}
