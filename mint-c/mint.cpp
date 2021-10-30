// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include "mint.h"

SYS_T sys;
CELL t1;
byte isBye = 0, isError = 0;
char buf[100];

inline void push(CELL v) { if (sys.dsp < STK_SZ) { sys.dstack[++sys.dsp] = v; } }
inline CELL pop() { return (sys.dsp) ? sys.dstack[sys.dsp--] : 0; }

void rpush(addr v) { if (sys.rsp < STK_SZ) { sys.rstack[++sys.rsp] = v; } }
addr rpop() { return (sys.rsp) ? sys.rstack[sys.rsp--] : 0; }

void vmInit() {
    sys.dsp = sys.rsp = sys.lsp = 0;
    for (int i = 0; i < NUM_REGS; i++) { REG[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { USER[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { FUNC[i] = 0; }
    REG['h' - 'a'] = (CELL)&sys.user[0];
    REG['r' - 'a'] = (CELL)&sys.reg[0];
    REG['s' - 'a'] = (CELL)&sys;
    REG['u' - 'a'] = (CELL)&sys.user[0];
    REG['z' - 'a'] = USER_SZ;
    *((byte *)HERE++) = ';';
}

void setCell(byte* to, CELL val) {
    *(to++) = (val) & 0xff;
    *(to++) = (val >> 8) & 0xff;
#if CELL_SZ == 4
    *(to++) = (val >> 16) & 0xff;
    *(to) = (val >> 24) & 0xff;
#endif
}

CELL getCell(byte* from) {
    CELL val = *(from++);
    val |= (*from++) << 8;
#if CELL_SZ == 4
    val |= (*from++) << 16;
    val |= (*from) << 24;
#endif
    return val;
}

void dumpStack() {
    for (UCELL i = 1; i <= sys.dsp; i++) {
        printStringF("%s%ld", (i > 1 ? " " : ""), (long)sys.dstack[i]);
    }
}

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

addr doDefineFunction(addr pc, char nm) {
    CELL fn = nm - 'A';
    FUNC[fn] = (addr)pc;
    while (*(pc++)) { 
        if (*(pc-1) == ';') {
            HERE = (CELL) pc;
            return pc; 
        }
    }
    isError = 1;
    printString("-dfErr-");
    return 0;
}

addr doFor(addr pc) {
    CELL n = pop();
    if (n == 0) {
        while (*(pc++) != ')') {}
        return pc;
    }
    if (LSP < LSTACK_SZ) {
        LOOP_ENTRY_T* x = &sys.lstack[LSP++];
        x->start = pc;
        INDEX = x->from = 0;
        x->to = n;
        x->end = 0;
        if (x->to < x->from) {
            push(x->to);
            x->to = x->from;
            INDEX = x->from = pop();
        }
    }
    return pc;
}

addr doNext(addr pc) {
    if (LSP < 1) { LSP = 0; return pc; }
    LOOP_ENTRY_T* x = &sys.lstack[LSP - 1];
    INDEX = ++x->from;
    if (x->from < x->to) { x->end = pc; pc = x->start; }
    else { 
        LSP--;
        if (0 < LSP) { INDEX = sys.lstack[LSP - 1].from; }
    }
    return pc;
}

addr doExt(addr pc) {
    byte ir = *(pc++);
    switch (ir) {
    case '!': *(byte*)T = (byte)N; DROP2;          break;
    case '@': T = *(char*)T;                       break;
    case 'F': return doFile(pc);
    case 'P': return doPin(pc);
    default: 
        printString("-noExt-");
        isError = 1;
    }
    return pc;
}

addr run(addr pc) {
    isError = 0;
    LSP = 0;
    while (!isError && (0 < pc)) {
        byte ir = *(pc++);
        switch (ir) {
        case 0: return pc;
        case ' ': while (*(pc) == ' ') { pc++; }       break;  // 32
        case '!': setCell((byte*)T, N); DROP2;  break;  // 33
        case '"': push(T);                      break;  // 34 (DUP)
        case '#': push(N);                      break;  // 35 (OVER)
        case '$': t1 = N; N = T; T = t1;        break;  // 36 (SWAP)
        case '%': t1 = pop(); T %= t1;          break;  // 37
        case '&': t1 = pop(); T &= t1;          break;  // 38
        case '\'': DROP1;                       break;  // 39
        case '(': pc = doFor(pc);               break;  // 40
        case ')': pc = doNext(pc);              break;  // 41
        case '*': t1 = pop(); T *= t1;          break;  // 42
        case '+': t1 = pop(); T += t1;          break;  // 43
        case ',': printChar((char)pop());       break;  // 44
        case '-': t1 = pop(); T -= t1;          break;  // 45
        case '.': printStringF("%ld", (long)pop());    break;  // 46
        case '/': t1 = pop();                           // 47
            if (t1) { T /= t1; }
            else { printString("-zeroDiv-"); isError = 1; }
            break;
        case '0': case '1': case '2': case '3': case '4':     // 48-57
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            t1 = *(pc) - '0';
            while (BetweenI(t1, 0, 9)) {
                T = (T * 10) + t1;
                t1 = *(++pc) - '0';
            } break;
        case ':': ir = *(pc++);                            // 58
            if (BetweenI(ir, 'A', 'Z')) {
                pc = doDefineFunction(pc, ir);
            } else { isError = 1; }
            break;
        case ';': pc = rpop();                        break;  // 59
        case '<': t1 = pop(); T = T <  t1 ? 1 : 0;    break;  // 60
        case '=': t1 = pop(); T = T == t1 ? 1 : 0;    break;  // 61
        case '>': t1 = pop(); T = T >  t1 ? 1 : 0;    break;  // 62
        case '?': /* FREE */                          break;  // 63
        case '@': T = getCell((byte *)T);             break;  // 64
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': t1 = ir - 'A';
            if (FUNC[t1]) {
                rpush(pc);
                pc = FUNC[t1];
            } break;
        case '[':                                       break;  //  91
        case '\\': isBye = 1;                           break;  //  92
        case ']':                                       break;  //  93
        case '^': t1 = pop(); T ^= t1;                  break;  //  94
        case '_': while (*(pc) != '_') {                        // 95
            printChar(*(pc++));
        } ++pc;
            break;
        case '`': pc = doExt(pc);                       break; // 96
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z': 
            push((CELL)&REG[ir-'a']);  
            break;
        case '{':                                      break;  // 123
        case '|': t1 = pop(); T |= t1;                 break;  // 124
        case '}':                                      break;  // 125
        case '~': T = ~T;                              break;  // 126
        }
    }
    if (isError && ((CELL)pc < HERE)) { REG[4] = (CELL)pc; }
    return pc;
}
