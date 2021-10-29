// S4 - a stack VM, inspired by Sandor Schneider's STABLE - https://w3group.de/stable.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define CELL  long
#define UCELL unsigned long
#define addr  unsigned short
#define byte  unsigned char

#define STK_SZ            7
#define LSTACK_SZ         4
#define HERE          REG[7]
#define NUM_REGS          26
#define NUM_FUNCS         26
#define USER_SZ     (16*1024)

typedef struct {
    addr pc;
    CELL from;
    CELL to;
    addr end;
} LOOP_ENTRY_T;

struct {
    UCELL dsp, rsp, lsp;
    CELL  reg[NUM_REGS];
    byte  user[USER_SZ];
    addr  func[NUM_FUNCS];
    CELL  dstack[STK_SZ + 1];
    addr  rstack[STK_SZ + 1];
    LOOP_ENTRY_T lstack[LSTACK_SZ];
} sys;

byte isBye = 0, isError = 0;
char buf[100];
FILE* input_fp;

#define REG        sys.reg
#define USER       sys.user
#define FUNC       sys.func
#define T          sys.dstack[sys.dsp]
#define N          sys.dstack[sys.dsp-1]
#define R          sys.rstack[sys.rsp]
#define L          sys.lsp
#define DROP1      pop()
#define DROP2      pop(); pop()
#define BetweenI(n, x, y) ((x <= n) && (n <= y))

inline void push(CELL v) { if (sys.dsp < STK_SZ) { sys.dstack[++sys.dsp] = v; } }
inline CELL pop() { return (sys.dsp) ? sys.dstack[sys.dsp--] : 0; }

void rpush(addr v) { if (sys.rsp < STK_SZ) { sys.rstack[++sys.rsp] = v; } }
addr rpop() { return (sys.rsp) ? sys.rstack[sys.rsp--] : 0; }

void vmInit() {
    sys.dsp = sys.rsp = sys.lsp = 0;
    for (int i = 0; i < NUM_REGS; i++) { REG[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { USER[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { FUNC[i] = 0; }
    REG['g' - 'a'] = (CELL)&sys.reg[0];
    REG['k' - 'a'] = 1000;
    REG['s' - 'a'] = (CELL)&sys;
    REG['u' - 'a'] = (CELL)&sys.user[0];
    REG['z' - 'a'] = USER_SZ;
}

void setCell(byte* to, CELL val) {
    *(to++) = (val) & 0xff;
    *(to++) = (val >> 8) & 0xff;
    *(to++) = (val >> 16) & 0xff;
    *(to) = (val >> 24) & 0xff;
}

CELL getCell(byte* from) {
    CELL val = *(from++);
    val |= (*from++) << 8;
    val |= (*from++) << 16;
    val |= (*from) << 24;
    return val;
}

void printChar(const char c) { printf("%c", c); }
void printString(const char* str) { printf("%s", str); }

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

addr doDefineFunction(addr pc, char nm) {
    CELL fn = nm - 'A';
    USER[HERE++] = ':';
    USER[HERE++] = nm;
    FUNC[fn] = (addr)HERE;
    while (USER[pc]) { 
        USER[HERE++] = USER[pc++]; 
        if (USER[pc-1] == ';') { return pc; }
    }
    isError = 1;
    printString("-dfErr-");
    return 0;
}

addr doFor(addr pc) {
    CELL n = pop();
    if (n == 0) {
        while (USER[pc++] != ')') {}
        return pc;
    }
    if (L < LSTACK_SZ) {
        LOOP_ENTRY_T* x = &sys.lstack[L++];
        x->pc = pc;
        x->to = n;
        x->from = 1;
        x->end = 0;
        if (x->to < x->from) {
            push(x->to);
            x->to = x->from;
            x->from = pop();
        }
    }
    return pc;
}

addr doNext(addr pc) {
    if (L < 1) { L = 0; return pc; }
    LOOP_ENTRY_T* x = &sys.lstack[L - 1];
    ++x->from;
    if (x->from <= x->to) { x->end = pc; pc = x->pc; }
    else { L--; }
    return pc;
}

void dumpStack(int hdr) {
    if (hdr) { printStringF("\r\nSTACK: size: %d ", STK_SZ); }
    printString("(");
    for (UCELL i = 1; i <= sys.dsp; i++) { printStringF("%s%ld", (i > 1 ? " " : ""), sys.dstack[i]); }
    printString(")");
}

#ifdef __DEV_BOARD__
addr doPin(addr pc) {
    int ir = USER[pc++];
    CELL pin = pop(), val = 0;
    switch (ir) {
    case 'I': pinMode(pin, INPUT);         break;
    case 'U': pinMode(pin, INPUT_PULLUP);  break;
    case 'O': pinMode(pin, OUTPUT);        break;
    case 'R': ir = USER[pc++];
        if (ir == 'D') { push(digitalRead(pin)); }
        if (ir == 'A') { push(analogRead(pin)); }
        break;
    case 'W': ir = USER[pc++]; val = pop();
        if (ir == 'D') { digitalWrite(pin, val); }
        if (ir == 'A') { analogWrite(pin, val); }
        break;
    }
    return pc;
}
#endif

addr run(addr pc) {
    CELL t1, t2;
    isError = 0;
    while (!isError && (0 < pc)) {
        byte ir = USER[pc++];
        switch (ir) {
        case 0: return -1;
        case ' ': while (USER[pc] == ' ') { pc++; }      break;  // 32
        case '!': t2 = pop(); setCell((byte *)t2, pop()); break;  // 33
        case '_': while ((pc < USER_SZ) && (USER[pc] != '_')) {
            printChar(USER[pc++]);
        } ++pc;
            break;
        case '"': push(T);                      break;  // 35 (DUP)
        case '$': t1 = N; N = T; T = t1;        break;  // 36 (SWAP)
        case '#': push(N);                      break;  // 37 (OVER)
        case '&': t1 = pop(); T &= t1;          break;  // 38
        case '\'': push(USER[pc++]);            break;  // 39
        case '(': pc = doFor(pc);               break;  // 40
        case ')': pc = doNext(pc);              break;  // 41
        case '*': t1 = pop(); T *= t1;          break;  // 42
        case '+': t1 = pop(); T += t1;          break;  // 43
        case ',': printChar((char)pop());       break;  // 44
        case '-': t1 = pop(); T -= t1;          break;  // 45
        case '.': printStringF("%ld", pop());   break;  // 46
        case '/': t1 = pop();                           // 47
            if (t1) { T /= t1; }
            else { printString("-zeroDiv-"); isError = 1; }
            break;
        case '0': case '1': case '2': case '3': case '4':   // 48-57
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0');
            t1 = USER[pc] - '0';
            while (BetweenI(t1, 0, 9)) {
                T = (T * 10) + t1;
                t1 = USER[++pc] - '0';
            } break;
        case ':': ir = USER[pc++];
            if (BetweenI(ir, 'A', 'Z')) {
                pc = doDefineFunction(pc, ir);
            } break;
        case ';': if (sys.rsp < 1) { sys.rsp = 0;  return pc; }
                pc = rpop();                           break;  // 59
        case '<': t1 = pop(); T = T < t1 ? 1 : 0;     break;  // 60
        case '=': t1 = pop(); T = T == t1 ? 1 : 0;     break;  // 61
        case '>': t1 = pop(); T = T > t1 ? 1 : 0;     break;  // 62
        case '?': /* FREE */                           break;  // 63
        case '@': T = getCell((byte *)T);               break;  // 64
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': t1 = ir - 'A';
            if (FUNC[t1]) {
                rpush(pc);
                pc = FUNC[t1];
            } break;
        case '[':                       break;  //  91
        case '\\': isBye = 1;                          break;  //  92
        case ']':                  break;  //  93
        case '^': t1 = pop(); T ^= t1;                 break;  //  94
        case '`': break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
        case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
        case 's': case 't': case 'u': case 'v': case 'w': case 'x':
        case 'y': case 'z': 
            push((CELL)&REG[ir-'a']);  
            break;
        case '{':                   break;  // 123
        case '|': t1 = pop(); T |= t1;                 break;  // 124
        case '}':                 break;  // 125
        case '~': T = (T) ? 0 : 1;                     break;  // 126
        }
    }
    if (isError && ((CELL)pc < HERE)) { REG[4] = pc; }
    return pc;
}

void setCodeByte(addr loc, char ch) {
    if ((0 <= loc) && (loc < USER_SZ)) { USER[loc] = ch; }
}

CELL regVal(int regNum) { return REG[regNum]; }

void ok() {
    printString("\r\nmint:");
    dumpStack(0);
    printString(">");
}

void executeString(addr loc, const char* txt) {
    addr x = loc;
    while (*txt) { setCodeByte(x++, *(txt++)); }
    setCodeByte(x, 0);
    run(loc);
}

void loop() {
    char tib[100];
    addr nTib = (addr)USER_SZ - 100;
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { ok(); }
    if (fgets(tib, 100, fp) == tib) {
        // if (fp == stdin) { doHistory(tib); }
        executeString(nTib, tib);
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = NULL; // input_pop();
    }
}

int main(int argc, char** argv) {
    while (!isBye) { loop(); }
    return 0;
}
