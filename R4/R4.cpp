// R4 - A Minimal Interpreter

#include "R4.h"

byte ir, isBye = 0, isError = 0;
static char buf[24];
addr pc, HERE;
CELL n1, t1;
ushort dsp, rsp, lsp, locStart;
CELL   dstack[STK_SZ + 1];
addr   rstack[STK_SZ + 1];
LOOP_ENTRY_T lstack[LSTACK_SZ + 1];
addr   func[NUM_FUNCS];
CELL   reg[NUM_REGS];
byte   user[USER_SZ];
CELL locals[STK_SZ * 10];

void push(CELL v) { if (dsp < STK_SZ) { dstack[++dsp] = v; } }
CELL pop() { return (dsp) ? dstack[dsp--] : 0; }

inline void rpush(addr v) { if (rsp < STK_SZ) { rstack[++rsp] = v; } }
inline addr rpop() { return (rsp) ? rstack[rsp--] : 0; }

static float fstack[STK_SZ+1];
static int fsp = 0;

#define fT fstack[fsp]
#define fN fstack[fsp-1]
#define fDROP1 fpop()
#define fDROP2 fpop(); fpop();
static void fpush(float v) { if (fsp < STK_SZ) { fstack[++fsp] = v; } }
static float fpop() { return (fsp) ? fstack[fsp--] : 0; }

#define lAt() (&lstack[lsp])
inline LOOP_ENTRY_T* lpush() { if (lsp < STK_SZ) { ++lsp; } return lAt(); }
inline LOOP_ENTRY_T *ldrop() { if (0 < lsp) { --lsp; } return lAt(); }

void vmInit() {
    dsp = rsp = lsp = fsp = locStart = 0;
    for (int i = 0; i < NUM_REGS; i++) { reg[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { user[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { func[i] = 0; }
    HERE = &user[0];
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
    for (UCELL i = 1; i <= dsp; i++) {
        printStringF("%s%ld", (i > 1 ? " " : ""), (CELL)dstack[i]);
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
    if (!lsp) { isError = 1; return; }
    LOOP_ENTRY_T* x = lAt();
    ldrop();
    if (x->end) { pc = x->end; }
    else { skipTo(c); }
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

int getRFnum(int isReg) {
    push(0);
    while (*pc) {
        int d = BetweenI(*pc, 'A', 'Z') ? (*pc) - 'A' : -1;
        if (d < 0) { break; }
        T = (T * 26) + d;
        ++pc;
    }
    if ((isReg == 1) && (NUM_REGS <= T)) { isError = 1; printString("-#reg-"); }
    if ((isReg == 0) && (NUM_FUNCS <= T)) { isError = 1; printString("-#func-"); }
    if (isError) { DROP1; }
    return isError == 0;
}

CELL getDecimal() {
    CELL x = 0;
    while (*pc) {
        int d = BetweenI(*pc, '0', '9') ? (*pc) - '0' : -1;
        if (0 <= d) { x = (x * 10) + d; ++pc; }
        else { return x; }
    }
    return x;
}

void doRand() {
    static CELL seed = 0;
    if (!seed) { seed = getSeed(); }
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed <<  5);
    T = (T) ? ABS(seed) % T : seed;
}

void doExt() {
    ir = *(pc++);
    switch (ir) {
    case 'I': ir = *(pc++);
        if (ir == 'A') { 
            ir = *(pc++);
            if (ir == 'F') { push((CELL)&func[0]); };
            if (ir == 'H') { push((CELL)&HERE); };
            if (ir == 'R') { push((CELL)&reg[0]); };
            if (ir == 'U') { push((CELL)&user[0]); };
            return;
        };
        if (ir == 'C') { push(CELL_SZ); };
        if (ir == 'F') { push(NUM_FUNCS); };
        if (ir == 'H') { push((CELL)HERE); };
        if (ir == 'R') { push(NUM_REGS); };
        if (ir == 'U') { push(USER_SZ); };
        return;
    case 'S': if (*pc == 'R') { ++pc; vmInit(); }  return;
    case 'r': doRand();                            return;
    default:
        pc = doCustom(ir, pc);
    }
}

addr run(addr start) {
    pc = start;
    isError = 0;
    rsp = lsp = 0;
    while (!isError && pc) {
        ir = *(pc++);
        switch (ir) {
        case 0:                                                    return pc;
        case ' ': while (*(pc) == ' ') { pc++; }                   break;  // 32
        case '!': setCell((byte*)T, N); DROP2;                     break;  // 33 STORE
        case '"': while (*(pc)!=ir) { printChar(*(pc++)); }; ++pc; break;  // 34 PRINT
        case '#': push(T);                                         break;  // 35 DUP
        case '$': t1 = T; T = N; N = t1;                           break;  // 36 SWAP
        case '%': push(N);                                         break;  // 37 OVER
        case '&': ir = (*pc++)-'0';                                        // 38 LOCAL-VAL
            if (BetweenI(ir,0,9)) { push(locals[ir+locStart]); }   break;
        case '\'': push(*(pc++));                                  break;  // 39 CHAR-LIT
        case '(': doIf();                                          break;  // 40 IF
        case ')': /* endIf() */                                    break;  // 41 ENDIF
        case '*': t1 = pop(); T *= t1;                             break;  // 42 MULT
        case '+': t1 = pop(); T += t1;                             break;  // 43 ADD
        case ',': printChar((char)pop());                          break;  // 44 EMIT
        case '-': t1 = pop(); T -= t1;                             break;  // 45 SUBTRACT
        case '.': printStringF("%ld", (CELL)pop());                break;  // 46 DOT
        case '/': if (isOk(T, "-0div-")) { N /= T; DROP1; }        break;  // 47 DIVIDE
        case '0': case '1': case '2': case '3': case '4':                  // 48-57 NUMBER
        case '5': case '6': case '7': case '8': case '9':
            pc--; push(getDecimal());                              break;
        case ':': if (getRFnum(0)) {                                       // CREATE
                func[pop()] = pc;
                skipTo(';');
                HERE = pc;
            } break;
        case ';': pc = rpop(); locStart -= (locStart) ? 10 : 0;    break;  // 59 RETURN
        case '<': t1 = pop(); T = (T <  t1) ? 1 : 0;               break;  // 60
        case '=': t1 = pop(); T = (T == t1) ? 1 : 0;               break;  // 61
        case '>': t1 = pop(); T = (T >  t1) ? 1 : 0;               break;  // 62
        case '?':                                                  break;  // 63
        case '@': T = getCell((byte*)T);                           break;  // 64 FETCH
        case 'A': if (T < 0) { T = -T; }                           break;  // ABS
        case 'B': printChar(' ');                                  break;  // BLANK
        case 'C':                                                  break;
        case 'D': --T;                                             break;  // DEC
        case 'E': printString("\r\n");                             break;  // EOL
        case 'F': doFloat();                                       break;  // FLOAT ops
        case 'G':                                                  break;
        case 'H':                                                  break;
        case 'I':                                                  break;
        case 'J':                                                  break;
        case 'K': ir = *(pc++);
            if (ir == '?') { push(charAvailable()); }
            if (ir == '@') { push(getChar()); }
            break;
        case 'L': t1 = pop(); T = (T << t1);                       break;  // LEFT-SHIFT
        case 'M': if (isOk(T, "-0div-")) { t1 = pop(); T %= t1; }  break;  // MOD
        case 'N': T = -T;                                          break;  // NEGATE
        case 'O':                                                  break;
        case 'P': ++T;                                             break;  // TOS++
        case 'Q':                                                  break;
        case 'R': t1 = pop(); T = (T >> t1);                       break;  // RIGHT-SHIFT
        case 'S': if (T) { t1 = T; T = N % t1; N /= t1; }                  // /MOD
                else { isError = 1; printString("-0div-"); }       break;
        case 'T':                                                  break;
        case 'U':                                                  break;
        case 'V':                                                  break;
        case 'W':                                                  break;
        case 'X': if (T) { rpush(pc); pc = (addr)T; } pop();       break;  // eXecute
        case 'Y':                                                  break;
        case 'Z':                                                  break;
        case '[': doFor();                                         break;  // 91 FOR
        case '\\': pop();                                          break;  // 92 DROP
        case ']': doNext();                                        break;  // 93 NEXT
        case '^': if (getRFnum(0) && func[T]) {                            // FUNCTION CALL
            if (*pc != ';') { rpush(pc); locStart += 10; }
            pc = func[T];
        } DROP1; break;
        case '_': T = (T) ? 0 : 1;                                 break;  // 95 NOT (LOGICAL)
        case '`': /*FREE*/                                                 break;  // 96
        case 'a': if (isReg(T)) { T = (CELL)&reg[T]; }             break;  // REGISTER ADDRESS
        case 'b': ir = *(pc++);                                            // BIT operations
            if (ir == '&') { N &= T; DROP1; }      // AND
            if (ir == '|') { N |= T; DROP1; }      // OR
            if (ir == '^') { N ^= T; DROP1; }      // XOR
            if (ir == '~') { T = ~T; }             // NOT (COMPLEMENT)
            break;
        case 'c': ir = *(pc++);                                            // c@, c!
            if (ir == '@') { T = *(byte*)T; }
            if (ir == '!') { *(byte*)T = (byte)N; DROP2; }
            break;
        case 'd': if (getRFnum(1)) { --reg[pop()]; }               break;  // REG DECREMENT
        case 'e': if (getRFnum(0) && func[T]) {                            // FUNCTION CALL
            if (*pc != ';') { rpush(pc); locStart += 10; }
            pc = func[T];
        } DROP1; break;
        case 'f':                                                  break;
        case 'g': if (T) { pc = (addr)T; } pop();                  break;
        case 'h': push(0); while (1) {                                     // HEX number
                t1 = BetweenI(*pc,'0','9') ? (*pc)-'0' : -1;
                t1 = BetweenI(*pc,'A','F') ? (*pc)-'A'+10 : t1;
                if (t1 < 0) { break; }
                T = (T * 16) + t1; ++pc;
            } break;
        case 'i': if (getRFnum(1)) { ++reg[pop()]; }               break; // REG INCREMENT
        case 'j':                                                  break;
        case 'k':                                                  break;
        case 'l': loopExit(']');                                   break;
        case 'm': /*FREE*/                                         break;
        case 'n': /*FREE*/                                         break;
        case 'o': /*FREE*/                                         break;
        case 'p': /*FREE*/                                         break;
        case 'q': /*FREE*/                                         break;
        case 'r': if (getRFnum(1)) { T = reg[T]; }                 break;  // REGISTER
        case 's': if (getRFnum(1)) { reg[T] = N; DROP2; }          break;  // SET REGISTER
        case 't': /*FREE*/                                         break;
        case 'u': /*FREE*/                                         break;
        case 'v': /*FREE*/                                         break;
        case 'w': loopExit('}');                                   break;
        case 'x': doExt();                                         break;
        case 'y': /*FREE*/                                         break;
        case 'z': /*FREE*/                                         break;
        case '{': { LOOP_ENTRY_T *x = lpush();                             // 123 WHILE
                x->start = pc; x->end = 0;
                if (!T) { skipTo('}'); }
            } break;
        case '|':  /*FREE*/                                        break;  // 124
        case '}': if (!T) { ldrop(); DROP1; }                              // 125
            else { lAt()->end = pc; pc = lAt()->start; }
            break;
        case '~': ir = (*pc++)-'0';                                        // 126 SET-LOCAL
            if (BetweenI(ir,0,9)) { locals[ir+locStart] = pop(); } break;
        }
    }
    return pc;
}
