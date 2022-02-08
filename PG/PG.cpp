// R4 - A Minimal Interpreter

#define  _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <varargs.h>
#include <stdio.h>
#include <conio.h>

typedef unsigned char byte;
typedef unsigned char *addr;
typedef long CELL;
typedef unsigned long UCELL;
typedef unsigned short ushort;

#define STK_SZ 15
#define LSTACK_SZ 8
#define NUM_FUNCS 26
#define NUM_REGS 26
#define USER_SZ  (32*1024)

#define CELL_SZ    sizeof(CELL)
#define INDEX      reg[8]
#define TOS        dstack[dsp]
#define A          (addr)TOS
#define NOS        dstack[dsp-1]
#define R          rstack[dsp]
#define DROP1      pop()
#define DROP2      pop(); pop()
#define BetweenI(n, x, y) (((x) <= (n)) && ((n) <= (y)))
#define isLocal(x) (('0' <= (x)) && ((x) <= '9'))
#define isRegChar(x) (('A' <= (x)) && ((x) <= 'Z'))
#define isReg(n) ((0 <= (n)) && ((n) < NUM_REGS))
#define isFunc(n) ((0 <= (n)) && ((n) < NUM_FUNCS))
#define ABS(x) ((x < 0) ? -x : x)


typedef struct {
    addr start;
    CELL from;
    CELL to;
    addr end;
} LOOP_ENTRY_T;


byte ir, isBye = 0, isError = 0;
static char buf[96];
addr pc, HERE;
CELL n1, t1, seed = 0;
ushort dsp, rsp, lsp, locStart;
CELL   dstack[STK_SZ + 1];
addr   rstack[STK_SZ + 1];
LOOP_ENTRY_T lstack[LSTACK_SZ + 1];
addr   func[NUM_FUNCS];
CELL   reg[NUM_REGS];
byte   user[USER_SZ];
CELL locals[STK_SZ * 10];
int destReg = -1;

void push(CELL v) { if (dsp < STK_SZ) { dstack[++dsp] = v; } }
CELL pop() { return (dsp) ? dstack[dsp--] : 0; }

inline void rpush(addr v) { if (rsp < STK_SZ) { rstack[++rsp] = v; } }
inline addr rpop() { return (rsp) ? rstack[rsp--] : 0; }

static float flstack[STK_SZ + 1];
static int flsp = 0;

#define fT flstack[flsp]
#define fN flstack[flsp-1]
#define fDROP1 flpop()
#define fDROP2 flpop(); flpop();
static void flpush(float v) { if (flsp < STK_SZ) { flstack[++flsp] = v; } }
static float flpop() { return (flsp) ? flstack[flsp--] : 0; }

#define lAt() (&lstack[lsp])
inline LOOP_ENTRY_T* lpush() { if (lsp < STK_SZ) { ++lsp; } return lAt(); }
inline LOOP_ENTRY_T* ldrop() { if (0 < lsp) { --lsp; } return lAt(); }

void vmInit() {
    dsp = rsp = lsp = flsp = locStart = 0;
    for (int i = 0; i < NUM_REGS; i++) { reg[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { user[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { func[i] = 0; }
    HERE = &user[0];
}

void printChar(const char c) { printf("%c", c); }
void printString(const char* str) { printf("%s", str); }
CELL getSeed() { return GetTickCount(); }
CELL doMillis() { return (CELL)GetTickCount(); }
CELL doMicros() { return (CELL)doMillis() * 1000; }
void doDelay(CELL ms) { Sleep(ms); }
int getChar() { return _getch(); }
int charAvailable() { return _kbhit(); }


void setCell(byte* to, CELL val) {
#ifdef _NEEDS_ALIGN_
    * (to++) = (byte)val;
    for (int i = 1; i < CELL_SZ; i++) {
        val = (val >> 8);
        *(to++) = (byte)val;
    }
#else
    * ((CELL*)to) = val;
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

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

void skipTo(byte to, int isCreate) {
    while (*pc) {
        ir = *(pc++);
        if (isCreate) { *(HERE++) = ir; }
        if ((to == '"') && (ir != to)) { continue; }
        if (ir == to) { return; }
        if (ir == '\'') { ++pc; continue; }
        if (ir == '(') { skipTo(')', isCreate); continue; }
        if (ir == '[') { skipTo(']', isCreate); continue; }
        if (ir == '"') { skipTo('"', isCreate); continue; }
    }
    isError = 1;
}

void doFor() {
    CELL t = (NOS < TOS) ? TOS : NOS;
    CELL f = (NOS < TOS) ? NOS : TOS;
    DROP2;
    LOOP_ENTRY_T* x = lpush();
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
    }
    else {
        INDEX = ldrop()->from;
    }
}

void loopExit() {
    if (!lsp) { isError = 1; return; }
    LOOP_ENTRY_T* x = lAt();
    ldrop();
    char c = ((x->from) || (x->to)) ? ']' : '}';
    if (c == '}') { DROP1; }
    if (x->end) { pc = x->end; }
    else {
        skipTo(c, 0);
    }
}

int isOk(int exp, const char* msg) {
    isError = (exp == 0); if (isError) { printString(msg); }
    return (isError == 0);
}

void doFloat() {
    switch (*(pc++)) {
    case '<': flpush((float)pop());                             return;
    case '>': push((CELL)flpop());                              return;
    case '+': fN += fT; fDROP1;                                 return;
    case '-': fN -= fT; fDROP1;                                 return;
    case '*': fN *= fT; fDROP1;                                 return;
    case '/': if (isOk(fT != 0, "-0div-")) { fN /= fT; fDROP1; }  return;
    case '.': printStringF("%g", flpop());                      return;
    default:
        isError = 1;
        printString("-flt?-");
    }
}

void doExt() {
    ir = *(pc++);
    switch (ir) {
    case 'I': ir = *(pc++);
        if (ir == 'A') {
            ir = *(pc++);
            if (ir == 'F') { push((CELL)&func[0]); }
            if (ir == 'H') { push((CELL)&HERE); }
            if (ir == 'R') { push((CELL)&reg[0]); }
            if (ir == 'U') { push((CELL)&user[0]); }
            return;
        };
        if (ir == 'C') { push(CELL_SZ); }
        if (ir == 'F') { push(NUM_FUNCS); }
        if (ir == 'H') { push((CELL)HERE); }
        if (ir == 'R') { push(NUM_REGS); }
        if (ir == 'U') { push(USER_SZ); }
        return;
    // case 'E': doEditor();                            return;
    case 'S': if (*pc == 'R') { ++pc; vmInit(); }    return;
    case 'N': push(doMicros());                      return;
    case 'T': push(doMillis());                      return;
    case 'W': if (0 < TOS) { doDelay(TOS); } pop();  return;
    case 'R': if (!seed) { seed = getSeed(); }
            seed ^= (seed << 13);
            seed ^= (seed >> 17);
            seed ^= (seed << 5);
            TOS = (TOS) ? ABS(seed) % TOS : seed;
            return;
    default:
        break;
        // pc = doCustom(ir, pc);
    }
}

addr run(addr start) {
    pc = start;
    isError = 0;
    rsp = lsp = locStart = 0;
    while (!isError && pc) {
        ir = *(pc++);
        switch (ir) {
        case 0:                                                    return pc;
        case ' ': while (*(pc) == ' ') { pc++; }                   break;  // 32
        case '!': setCell((byte*)TOS, NOS); DROP2;                 break;  // 33 STORE
        case '"': while (*(pc) != ir) { printChar(*(pc++)); }; ++pc; break;  // 34 PRINT
        case '#': push(TOS);                                       break;  // 35 DUP
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                   break;  // 36 SWAP
        case '%': push(NOS);                                       break;  // 37 OVER
        case '&': /*FREE*/                                         break;  // 38
        case '\'': push(*(pc++));                                  break;  // 39 CHAR-LIT
        case '(': if (!TOS) { skipTo(')', 0); } DROP1;             break;  // 40 IF
        case ')': /* endIf() */                                    break;  // 41 ENDIF
        case '*': t1 = pop(); TOS *= t1;                           break;  // 42 MULTIPLY
        case '+': t1 = pop(); TOS += t1;                           break;  // 43 ADD
        case '-': t1 = pop(); TOS -= t1;                           break;  // 45 SUBTRACT
        case ',': 
        case '.': if (0 < destReg) { reg[destReg] = pop(); }       break;  // 46 DOT
        case '/': if (isOk(TOS, "-0div-")) { NOS /= TOS; DROP1; }  break;  // 47 DIVIDE
        case '0': case '1': case '2': case '3': case '4':                  // 48-57 NUMBER
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            while (BetweenI(*pc, '0', '9')) {
                TOS = (TOS * 10) + *(pc++) - '0';
            } break;
        case ':': ir = *(pc++) - 'A';
            if (!BetweenI(ir, 0, 25)) { isError = 1; break; }  // 58 CREATE
            func[ir] = pc;
            skipTo(';', 0);
            HERE = (HERE < pc) ? pc : HERE;
            break;
        case ';': pc = rpop(); locStart -= (locStart) ? 10 : 0;    break;  // 59 RETURN
        case '<': t1 = pop(); TOS = (TOS < t1) ? 1 : 0;            break;  // 60 LESS-THAN
        case '=': t1 = pop(); TOS = (TOS == t1) ? 1 : 0;           break;  // 61 EQUALS
        case '>': t1 = pop(); TOS = (TOS > t1) ? 1 : 0;            break;  // 62 GREATER-THAN
        case '?': if (pop() == 0) { skipTo('.', 0); }              break;  // 63
        case '@': TOS = getCell((byte*)TOS);                       break;  // 64 FETCH
        case 'A': case 'B': case 'C': case 'D': case 'E': 
        case 'F': case 'G': case 'H': case 'I': case 'J': 
        case 'K': case 'L': case 'M': case 'N': case 'O': 
        case 'P': case 'Q': case 'R': case 'S': case 'T': 
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            t1 = ir - 'A';
            if (*(pc) == '(') {
                printString("-()-");
                skipTo(')', 0);
            }
            if (func[t1]) { rpush(pc); pc = func[t1]; }
            break;
        case '[': doFor();                                         break;  // 91 FOR
        case '\\': DROP1;                                          break;  // 92 DROP
        case ']': doNext();                                        break;  // 93 NEXT
        case '^': if (*pc == 'd') { printStringF("%d", pop()); ++pc; }
                if (*pc == 'h') { printStringF("$%x", pop()); ++pc; }
                break;  // 94 Exit LOOP
        case '_': TOS = (TOS) ? 0 : 1;                             break;  // 95 NOT (LOGICAL)
        case '`': push(TOS);                                                 // 96 ZCOPY
            while ((*pc) && (*pc != ir)) { *(A++) = *(pc++); }
            *(A++) = 0; pc++;
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            t1 = ir - 'a';
            if (*(pc) == ':') {
                destReg = t1;
                ++pc;
            } else {
                push(reg[t1]);
            }
            break;
        case '{': { LOOP_ENTRY_T* x = lpush();                             // 123 BEGIN
            x->start = pc; x->end = 0; x->from = x->to = 0;
            // if (!TOS) { skipTo('}', 0); }
        } break;
        case '|':  /*FREE*/                                        break;  // 124
        case '}': // if (!TOS) { ldrop(); DROP1; }                              // 125 WHILE
                lAt()->end = pc; 
                pc = lAt()->start;
                break;
        case '~': TOS = -TOS;                                      break;  // 126 NEGATE
        }
    }
    return pc;
}


int main() {
    FILE* fp = fopen("code.txt","rt");
    if (fp) {
        fread(user, 1, USER_SZ, fp);
        fclose(fp);
    }
    run(user);
    return 0;
}