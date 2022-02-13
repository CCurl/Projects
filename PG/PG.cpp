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

#define STK_SZ       15
#define LSTACK_SZ     8
#define NUM_FUNCS    26
#define NUM_REGS     26
#define USER_SZ     (32*1024)
#define LPC          10    // Locals Per Call

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

#define OP_STACK   0
#define OP_DEST    1
#define OP_ADD     2
#define OP_SUB     3
#define OP_MUL     4
#define OP_DIV     5
#define OP_EQU     6
#define OP_LT      7
#define OP_GT      8

typedef struct {
    CELL *dest; // 0 => none, 1-26 (1:a, 26:z)
    byte reg;  // 0 => none, 1-26 (1:a, 26:z)
    byte op;  // see above
    byte loc; // locals start
} CTX_T;

byte ir, isBye = 0;
static char buf[96];
addr pc, HERE;
CELL n1, t1, seed = 0;
ushort dsp, rsp, lsp, locStart;
CELL   dstack[STK_SZ + 1];
addr   rstack[STK_SZ + 1];
LOOP_ENTRY_T lstack[LSTACK_SZ + 1];
addr   func[NUM_FUNCS];
CELL   reg[NUM_REGS+1];
byte   user[USER_SZ];
CELL locals[STK_SZ * LPC];
int isError, paramNum;

int ctxSP;
CTX_T ctx[32];
#define CTX ctx[ctxSP]
#define CLR_CTX { CTX.dest = NULL; CTX.reg = CTX.op = 0; }

void ctxPush() { ++ctxSP; CTX.dest = NULL; CTX.reg = CTX.op = 0; CTX.loc = locStart; }
void ctxPop() { if (ctxSP) --ctxSP; }

void push(CELL v) { if (dsp < STK_SZ) { dstack[++dsp] = v; } }
CELL pop() { return (dsp) ? dstack[dsp--] : 0; }

inline void rpush(addr v) { if (rsp < STK_SZ) { rstack[++rsp] = v; } }
inline addr rpop() { return (rsp) ? rstack[rsp--] : 0; }

int fncSp;
char *fncStk[8];
inline void fncPush(char *v) { if (fncSp < 7) { fncStk[++fncSp] = v; } }
inline char *fncPop() { return (fncSp) ? fncStk[fncSp--] : 0; }

int dstSp;
CELL *dstStk[8];
inline void dstPush(CELL *v) { if (dstSp < 7) { dstStk[++dstSp] = v; } }
inline CELL *dstPop() { return (dstSp) ? dstStk[dstSp--] : 0; }

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
    ctxSP = 0;
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
        if (ir == '?') { skipTo('!', isCreate); continue; }
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
    if (exp == 0) { isError = 1; printString(msg); return 0; }
    return 1;
}

void doOp() {
    switch (CTX.op) {
    case OP_STACK: break;
    case OP_ADD: if (CTX.dest) { *CTX.dest += TOS; }
               else { NOS += TOS; }
               DROP1;  break;
    case OP_SUB: if (CTX.dest) { *CTX.dest -= TOS; }
               else { NOS -= TOS; }
               DROP1;  break;
    case OP_MUL: if (CTX.dest) { *CTX.dest *= TOS; }
               else { NOS += TOS; }
               DROP1;  break;
    case OP_DIV: if (!isOk(TOS, "-div0-")) { break; }
               if (CTX.dest) { *CTX.dest /= TOS; }
               else { NOS += TOS; }
               DROP1; break;
    case OP_EQU: NOS = (NOS == TOS) ? 1 : 0;
        DROP1; break;
    case OP_LT: NOS = (NOS < TOS) ? 1 : 0;
        DROP1; break;
    case OP_GT: NOS = (NOS > TOS) ? 1 : 0;
        DROP1;  break;
    case OP_DEST: *CTX.dest = pop();
        break;
    }
    CTX.op = OP_STACK;
    CTX.reg = 0;
}

addr run(addr start) {
    pc = start;
    isError = rsp = lsp = locStart = fncSp = 0;
    fncStk[0] = 0;
    dstStk[0] = 0;
    ctxSP = 0;
    while (!isError && pc) {
        while (BetweenI(*pc, 1, 32)) { pc++; }
        ir = *(pc++);
        switch (ir) {
        case 0:                                                      return pc;
        case '!':           break;
        case '"': while (*(pc) != ir) { printChar(*(pc++)); }; ++pc; break;  // 34 PRINT
        case '#': ir = *(pc++) - '0';
            if (BetweenI(ir, 0, (LPC-1))) {
                int x = (LPC <= locStart) ? locStart - LPC : locStart;
                push(locals[x + ir]);
            }
            break;  // 35 local for (param)
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                   break;  // 36 SWAP
        case '%':  ir = *(pc++) - '0';
            if (BetweenI(ir, 0, (LPC-1))) {
                if (*pc == ':') { CTX.dest = &locals[locStart + ir]; ++pc;  }
                else { push(locals[locStart + ir]); }
            }                                                      break;  // 37 local 0-9
        case '&': /*FREE*/                                         break;  // 38
        case '\'': push(*(pc++));                                  break;  // 39 CHAR-LIT
        // case '(': if (!TOS) { skipTo(')', 0); } DROP1;             break;  // 40 IF
        case ')': doOp(); t1 = (CELL)fncPop();
            if (t1) {
                rpush(pc);
                // locStart += LPC;
                pc = (addr)t1;
            } break;  // 41 ENDIF
        case '*': CTX.op = OP_MUL;                           break;  // 43 ADD
        case '+': CTX.op = OP_ADD;                           break;  // 43 ADD
        case '-': CTX.op = OP_SUB;                           break;  // 43 ADD
        case '/': CTX.op = OP_DIV;                           break;  // 47 DIVIDE
        case ',': doOp(); CLR_CTX;                           break;  // 46 COMMA
        case '.': doOp(); CLR_CTX;                           break;  // 46 DOT
        case '0': case '1': case '2': case '3': case '4':                  // 48-57 NUMBER
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            while (BetweenI(*pc, '0', '9')) {
                TOS = (TOS * 10) + *(pc++) - '0';
            } 
            doOp(); break;
        case ':': ir = *(pc++);
            if (!BetweenI(ir, 'a', 'z')) { isError = 1; break; }     // 58 CREATE
            func[ir-'a'] = pc;
            skipTo(';', 0);
            HERE = (HERE < pc) ? pc : HERE;
            break;
        case ';': pc = rpop(); locStart -= (locStart) ? LPC : 0;   break;  // 59 RETURN
        case '<': CTX.op = OP_LT;             break;  // 60 LESS-THAN
        case '=': CTX.op = OP_EQU;            break;  // 60 LESS-THAN
        case '>': CTX.op = OP_GT;             break;  // 60 LESS-THAN
        case '?': if (pop() == 0) { skipTo('!', 0); }              break;  // 63
        case '@': return pc;                              break;  // 64 FETCH
        case 'A': case 'B': case 'C': case 'D': case 'E': 
        case 'F': case 'G': case 'H': case 'I': case 'J': 
        case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': 
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            printStringF("-func:%c", ir);
            break;
        case '[': doFor();                                         break;  // 91 FOR
        case '\\': DROP1;                                          break;  // 92 DROP
        case ']': doNext();                                        break;  // 93 NEXT
        case '^': if (*pc == 'd') { printStringF("%d", pop()); ++pc; }
                if (*pc == 'h') { printStringF("$%x", pop()); ++pc; }
                if (*pc == 'c') { printChar(pop()); ++pc; }
                if (*pc == 'b') { printChar(32); ++pc; }
                if (*pc == 'n') { printString("\r\n"); ++pc; }
                break;  // 94 Exit LOOP
        case '_': TOS = (TOS) ? 0 : 1;                             break;  // 95 NOT (LOGICAL)
        case '`':             break;
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            t1 = ir - 'a' + 1;
            if (*(pc) == ':') {
                CTX.dest = &reg[t1];
                CTX.op = OP_DEST;
                ++pc;
            } else {
                push(reg[t1]);
                doOp();
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
    vmInit();
    FILE *fp = fopen("code.txt","rb");
    if (fp) {
        fread(user, 1, USER_SZ, fp);
        fclose(fp);
    }
    run(user);
    return 0;
}