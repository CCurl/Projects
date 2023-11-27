// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include "mint.h"

SYS_T sys;
byte ir, isBye = 0, isError = 0;
static char buf[100];
addr pc;
CELL t1;

inline void push(CELL v) { if (sys.dsp < STK_SZ) { sys.dstack[++sys.dsp] = v; } }
inline CELL pop() { return (sys.dsp) ? sys.dstack[sys.dsp--] : 0; }

void rpush(addr v) { if (sys.rsp < STK_SZ) { sys.rstack[++sys.rsp] = v; } }
addr rpop() { return (sys.rsp) ? sys.rstack[sys.rsp--] : 0; }

void vmInit() {
    sys.dsp = sys.rsp = sys.lsp = 0;
    for (int i = 0; i < NUM_REGS; i++) { REG[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { USER[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { FUNC[i] = 0; }
    HERE = (CELL)&sys.user[0];
    VHERE = (CELL)&sys.user[VARS_OFFSET];
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

void skipTo(byte to) {
    while (*pc) {
        char c = *(pc++);
        if (c == to) { return; }
        if (c == '`') { skipTo('`'); }
        if (c == '(') { skipTo(')'); }
        if (c == '[') { skipTo(']'); }
    }
    isError = 1;
}

void doDefineFunction() {
    CELL fn = ir - 'A';
    FUNC[fn] = pc;
    skipTo(';');
    HERE = (CELL)(pc);
    if (isError) { printString("-dfErr-"); }
}

void doFor() {
    CELL n = pop();
    if (n == 0) {
        skipTo(')');
        return;
    }
    if (LSP < LSTACK_SZ) {
        LOOP_ENTRY_T* x = &sys.lstack[LSP++];
        x->start = pc;
        x->from = 0;
        x->to = n;
        x->end = 0;
        if (x->to < x->from) {
            push(x->to);
            x->to = x->from;
            x->from = pop();
        }
    }
}

addr doNext(addr pc) {
    if (LSP < 1) { LSP = 0; return pc; }
    LOOP_ENTRY_T* x = &sys.lstack[LSP - 1];
    ++x->from;
    if (x->from < x->to) { x->end = pc; pc = x->start; }
    else { LSP--; }
    return pc;
}

void doExt() {
    ir = *(pc++);
    switch (ir) {
    case '0': push((CELL)&sys.dstack[0]);
    RCASE '1': push((CELL)HERE);
    RCASE '2': push((CELL)&sys.user[0]);
    RCASE '3': push((CELL)&sys.reg[0]);
    RCASE '4': push((CELL)0);
    RCASE '5': push((CELL)&sys.func[0]);
    RCASE '6': push((CELL)&sys.user[VARS_OFFSET]);
    RCASE 'Q': isBye = 1;
    RCASE 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    case 'g': case 'h': case 'k': case 'l':
    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
    case 'y': case 'z': t1 = 26 + (ir - 'a');
        push((CELL)&REG[t1]);
    RCASE 'i': case 'j': t1 = LSP - ((ir == 'i') ? 1 : 2);
        if (t1 < 0) { isError = 1; }
        else { push((CELL)&sys.lstack[t1].from); }
        return;
    default:
        pc = doCustom(ir, pc);
    }
}

addr run(addr start) {
    isError = 0;
    pc = start;
    LSP = 0;
    next:
    if (isError && ((CELL)pc < HERE)) { REG[4] = (CELL)pc; }
    if (isError || (!pc)) { return pc; }
    ir = *(pc++);
    switch (ir) {
        case 0: return pc;
        NCASE ' ': while (*(pc) == ' ') { pc++; }
        NCASE '!': setCell((byte*)T, N); DROP2;
        NCASE '"': push(T);
        NCASE '#':  /* pc = doHex() */
        NCASE '$': t1 = N; N = T; T = t1;
        NCASE '%': push(N);
        NCASE '&': t1 = pop(); T &= t1;
        NCASE '\'': DROP1;
        NCASE '(': doFor();
        NCASE ')': pc = doNext(pc);
        NCASE '*': t1 = pop(); T *= t1;
        NCASE '+': t1 = pop(); T += t1;
        NCASE ',': printStringF("%lx", pop());
        NCASE '-': t1 = pop(); T -= t1;
        NCASE '.': printStringF("%ld", pop());
        NCASE '/': t1 = pop();
            if (t1) { T /= t1; }
            else { printString("-zeroDiv-"); isError = 1; }
        NCASE '0': case '1': case '2': case '3': case '4':
        case  '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            while (BetweenI(*pc, '0', '9')) {
                T = (T * 10) + *(pc++)-'0';
            }
        NCASE ':': ir = *(pc++);
            if (BetweenI(ir, 'A', 'Z')) {
                doDefineFunction();
            } else { isError = 1; }
        NCASE ';': pc = rpop();
        NCASE '<': t1 = pop(); T = T <  t1 ? 1 : 0;
        NCASE '=': t1 = pop(); T = T == t1 ? 1 : 0;
        NCASE '>': t1 = pop(); T = T >  t1 ? 1 : 0;
        NCASE '?': /* FREE */
        NCASE '@': T = getCell((byte *)T);
        NCASE 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': t1 = ir - 'A';
            if (FUNC[t1]) {
                if (*pc != ';') { rpush(pc); }
                pc = FUNC[t1];
            }
        NCASE '[': /*pc = doArray(pc); */
        NCASE '\\': doExt();
        NCASE ']': /* close of array */
        NCASE '^': t1 = pop(); T ^= t1;
        NCASE '_': T = -T;
        NCASE '`': while (*pc  && *pc != ir) {
            printChar(*(pc++));
        } pc++;
        NCASE 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z': push((CELL)&REG[ir-'a']);
        NCASE '{': T *= 2;
        NCASE '|': t1 = pop(); T |= t1;
        NCASE '}': T /= 2;
        NCASE '~': T = ~T;
        NEXT;
        default:
            printStringF("unknown ir: (%d)", ir);
    }
    return pc;
}
