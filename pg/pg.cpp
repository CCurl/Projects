// q4.cpp - a register-based interpreter

// Windows PC (Visual Studio)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define isPC
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
//#include <math.h>

#define STK_SZ             32
#define LSTK_SZ            11
#define MEM_SZ          10000
#define CODE_SZ         20480
#define REGS_SZ           128   // 'z'-'A'+1

enum { 
    STOP = 0,
    LIT1, LIT4, IF, ELSE, THEN, 
    CALL, EXIT, JMP, JMPZ,
    DUP, SWAP, OVER, DROP,
    ADD, SUB, MULT, DIV, INC, DEC,
    DO, INDEX, LOOP, BEGIN, WHILE,
    EMIT, DOT, TIMER,
    STORE, FETCH, 
    CREATE, GETWORD, ISNUM, 
    DEFINE, ENDWORD, IMMED, INLINE,
    NOOP, BYE
};

#define TOS (*sp)
#define NOS (*(sp-1))
#define DROP1 sp--
#define DROP2 sp-=2
#define RET(x) push(x); return;
#define NEXT goto next

#define BTW(a,b,c) ((b<=a) && (a<=c))

// #define ACC          regs[dstReg]
#define RG(x)        regs[(x)]
#define RGA(x)        regs[(x)-'A']

#define PC           *(pc)
#define IR           *(pc-1)
#define NR           *(pc++)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]

#define BYTES(x)      mem.b[x]
#define CELLS(x)      mem.c[x]

typedef long cell_t;
typedef uint8_t byte;
union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;
typedef struct { char *xt; char f; char name[11]; } dict_t;

cell_t stk[STK_SZ], *sp, rsp;
char *rstk[STK_SZ];
cell_t state, base;
cell_t lstk[LSTK_SZ+1], lsp;
cell_t regs[REGS_SZ];
dict_t dict[1024];
byte bytes[2048];
char *here, *pc, tib[128], *in;
dict_t *last;

#ifdef isPC
FILE *input_fp;
int isBye;
#endif

void push(cell_t x) { *(++sp) = (cell_t)(x); }
cell_t pop() { return *(sp--); }
void CComma(cell_t x) { *(here++) = (char)x; }
void Comma(cell_t x) { *(cell_t*)here = x; here += sizeof(cell_t); }
void Store(char *loc, cell_t x) { *(cell_t*)loc = x; }
cell_t Fetch(char *loc) { return *(cell_t*)loc; }

// ( d s--d )
void strCat() {
    char *s = (char*)pop();
    char *d = (char*)TOS;
    while (*d) ++d;
    while (*s) { *(d++)=*(s++); }
    *d = 0;
    DROP1;
}

// ( d s--d )
void strCpy() {
    char *d = (char*)NOS;
    *d = 0;
    strCat();
}

int lower(int x) { return BTW(x,'A','Z') ? x+32: x; }

// ( d s--n )
void strCmpI() {
    char *s = (char*)pop();
    char *d = (char*)TOS;
    while (*s && *d) {
        if (lower(*s) != lower(*d)) { break; }
        s++; d++;
    }
    TOS = (*s==*d) ? -1 : 0;
}

// ( d s--n )
void strCmp() {
    char *s = (char*)pop();
    char *d = (char*)TOS;
    while (*s && *d) { if (*s != *d) { break; } }
    TOS = (*s==*d) ? -1 : 0;
}

// ( nm-- )
void Create() {
    --last;
    push(0); TOS=NOS;
    NOS = (cell_t)&last->name[0];
    strCpy();
    last->xt = here;
    last->f = 0;
}

// ( nm--xt flags 1 )
// ( nm--0 )
void find() {
    char *nm = (char*)pop();
    dict_t *x = last;
    dict_t *end = (dict_t*)&BYTES(MEM_SZ);
    while (x < end) {
        push((cell_t)nm); push((cell_t)&x->name[0]);
        strCmpI();
        if (pop()) {
            push((cell_t)x->xt);
            push(x->f);
            RET(1);
        }
        ++x;
    }
    push(0);
}

// ( --n 1 )
// ( --0 )
void isDecimal(const char *wd) {
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { RET(0); }
    while (BTW(*wd, '0', '9')) { x = (x * 10) + (*(wd++) - '0'); }
    if (*wd) { RET(0); }
    push(isNeg ? -x : x);
    RET(1);
    /*
    if (*wd && (*wd != '.')) { RET(0); }
    if (*wd == 0) { push(isNeg ? -x : x); RET(1); }
    // Must be a '.', make it a float
    ++wd;
    float fx = (float)x, d = 10;
    while (BTW(*wd, '0', '9')) { fx += (*(wd++) - '0') / d; d *= 10; }
    if (*wd) { RET(0); }
    push(0);
    // FTOS = isNeg ? -fx : fx;
    RET(1);
    */
}

// ( nm--n 1 )
// ( nm--0 )
void isNum() {
    char *wd = (char*)pop();
    if ((wd[0] == '\'') && (wd[2] == '\'') && (wd[3] == 0)) { push(wd[1]); RET(1); }
    int b = base, lastCh = '9';
    if (*wd == '#') { b = 10;  ++wd; }
    if (*wd == '$') { b = 16;  ++wd; }
    if (*wd == '%') { b = 2;  ++wd; lastCh = '1'; }
    if (b == 10) { isDecimal(wd); return; }
    if (b < 10) { lastCh = '0' + b - 1; }
    if (*wd == 0) { RET(0); }
    cell_t x = 0;
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((b == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((b == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { RET(0); }
        x = (x * b) + t;
    }
    push(x);
    RET(1);
}

void getInput() {
    in = tib;
    printf("\nok:()> ");
    fgets(tib, sizeof(tib), stdin);
}

// ( --addr len )
// ( --0 )
void getword() {
    while (*in && (*in < 33)) { ++in; }
    if (*in == 0) { RET(0); }
    push((cell_t)in);
    push(0);
    while (32 < *in) { ++in; ++TOS; }
    *(in++) = 0;
}

char *doExt(char *pc) {
    cell_t t = *(pc++);
    if (t=='C') {  }
    return pc;
}

void Run(char *y) {
    cell_t t1;
    pc = y;
    if (sp < stk) { sp = stk; printf("-su-"); }
    if (&stk[STK_SZ] < sp) { sp = stk; printf("-so-"); }

next:
    // printf("-pc:%p,ir:%d-",pc,(int)*pc);
    switch (*(pc++)) {
    case STOP:                                                 return;
    case LIT1: push(*(pc++));                                   NEXT;
    case LIT4: push(*(cell_t*)pc); pc += sizeof(cell_t);        NEXT;
    case IF: CComma(JMPZ); push((cell_t)here); Comma(0);        NEXT;
    case ELSE: printf("-else-");                                NEXT;
    case THEN: t1=pop(); Store((char *)t1, (cell_t)here);       NEXT;
    case CALL: if (*pc != EXIT) { rstk[++rsp] = (pc+sizeof(cell_t)); }
            pc = *(char**)pc;                                   NEXT;
    case EXIT: if (rsp<1) { rsp=0; return; } pc=rstk[rsp--];    NEXT;
    case JMP: pc = *(char**)pc;                                 NEXT;
    case JMPZ: if (pop()==0) { pc = *(char**)pc; }
            else { pc += sizeof(cell_t); }                      NEXT;
    case STORE: Store((char*)TOS, NOS); DROP2;                  NEXT;
    case DUP: push(TOS);                                        NEXT;
    case SWAP: t1=TOS; TOS=NOS; NOS=t1;                         NEXT;
    case OVER: push(NOS);                                       NEXT;
    case DROP: DROP1;                                           NEXT;
    case ADD: NOS += TOS; DROP1;                                NEXT;
    case SUB: NOS -= TOS; DROP1;                                NEXT;
    case MULT: NOS *= TOS; DROP1;                               NEXT;
    case DIV: NOS /= TOS; DROP1;                                NEXT;
    case EMIT: printf("%c", (char)pop());                       NEXT;
    case DOT: printf("%d ", pop());                             NEXT;
    case TIMER: push(clock());                                  NEXT;
    case DEC: --TOS;                                            NEXT;
    case INC: ++TOS;                                            NEXT;
    case CREATE: getword(); if (pop()) { Create(); }            NEXT;
    case DO: lsp+=3; L2=(cell_t)pc; L0=pop(); L1=pop();         NEXT;
    case LOOP: if (++L0<L1) { pc=(char*)L2; } else { lsp-=3; }; NEXT;
    case BEGIN: lsp+=3; L0=(cell_t)pc;                          NEXT;
    case WHILE: if (pop()) { pc=(char*)L0; } else { lsp-=3; };  NEXT;
    case GETWORD: getword();                                    NEXT;
    case DEFINE: getword(); if (pop()) { Create(); state=1; }   NEXT;
    case ENDWORD: state=0; CComma(EXIT);                        NEXT;
    case ISNUM: isNum();                                        NEXT;
    case IMMED: last->f |= 1;                                   NEXT;
    case INLINE: last->f |= 2;                                  NEXT;
    case BYE: isBye=1;                                         return;
    default: printf("-[%d]?-",(int)*(pc-1));  break;
    }
}

void doInline(char *x) {
    CComma(*(x++));
    while ((*x) && (*x != EXIT)) { CComma(*(x++)); }
}

void ParseWord() {
    char *w = (char*)TOS;
    isNum();
    if (pop()) {
        if (state) {
            if (BTW(TOS,0,255)) { CComma(LIT1); CComma(pop()); }
            else { CComma(LIT4); Comma(pop()); }
        }
        RET(1);
    }
    push((cell_t)w);
    find();
    if (pop()) {
        cell_t f = pop();
        char *xt = (char*)pop();
        if ((state == 0) || (f & 0x01)) { Run(xt); RET(1); }
        if (f & 0x02) { doInline(xt); }
        else { CComma(CALL); Comma((cell_t)xt); }
        RET(1);
    }
    printf("[%s]??", w);
    if (state) {
        ++last;
        state = 0;
    }
    base = 10;
    RET(0);
}

void ParseLine(char *x) {
    in = x;
    //char *chere = here;
    while (isBye == 0) {
        getword();
        if (pop() == 0) break;
        ParseWord();
        if (pop() == 0) return;
    }
    //CComma(EXIT);
    //Run(chere);
    //here = chere;
}

void loadLine(const char *x) {
    in = tib;
    while (*x) { *(in++) = *(x++); }
    *in = 0;
    ParseLine(tib);
}

void loadPrim(const char *nm, int f, int n, char c1, char c2) {
    push((cell_t)nm); Create();
    last->f = f;
    *(here++) = c1;
    if (1 < n) { *(here++) = c2; }
    *(here++) = EXIT;
}

void init() {
    here = &BYTES(0);
    last = (dict_t*)&BYTES(MEM_SZ);
    sp = stk;
    in = tib;
    base = 10;
    rsp = 0;
    loadPrim("bye",       0, 1, BYE,      0);
    loadPrim("cell",      2, 2, LIT1, sizeof(cell_t));
    loadPrim("exit",      2, 1, EXIT,     0);
    loadPrim("create",    2, 1, CREATE,   0);
    loadPrim(":",         0, 1, DEFINE,   0);
    loadPrim(";",         1, 1, ENDWORD,  0);
    loadPrim("getword",   2, 2, GETWORD,  0);
    loadPrim("immediate", 2, 1, IMMED,    0);
    loadPrim("inline",    2, 1, INLINE,   0);
    loadPrim("isnum" ,    2, 1, ISNUM,    0);
    loadPrim("timer",     2, 1, TIMER,    0);
    loadPrim("if",        1, 1, IF,       0);
    loadPrim("else",      1, 1, ELSE,     0);
    loadPrim("then",      1, 1, THEN,     0);
    loadPrim("dup",       2, 1, DUP,      0);
    loadPrim("drop",      2, 1, DROP,     0);
    loadPrim("over",      2, 1, OVER,     0);
    loadPrim("swap",      2, 1, SWAP,     0);
    loadPrim("+",         2, 1, ADD,      0);
    loadPrim("-",         2, 1, SUB,      0);
    loadPrim("*",         2, 1, MULT,     0);
    loadPrim("/",         2, 1, DIV,      0);
    loadPrim(".",         2, 1, DOT,      0);
    loadPrim("emit",      2, 1, EMIT,     0);
    loadPrim("1-",        2, 1, DEC,      0);
    loadPrim("1+",        2, 1, INC,      0);
    loadPrim("begin",     2, 1, BEGIN,    0);
    loadPrim("while",     2, 1, WHILE,    0);
    loadPrim("do",        2, 1, DO,       0);
    loadPrim("I",         2, 1, INDEX,    0);
    loadPrim("loop",      2, 1, LOOP,     0);
    // loadLine("timer 500000000 begin 1- dup while drop timer swap - .");
}

#ifdef isPC
int main(int argc, char *argv[]) {
    // int r='A';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG(r++) = atoi(y); }
    init();
    while (isBye == 0) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}
#endif
