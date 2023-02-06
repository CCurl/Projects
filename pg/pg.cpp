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
#define REGS_SZ           128 // 'z'-'A'+1
#define FUNCS_SZ         1023

#define IF     5
#define ELSE   6
#define THEN   7
#define CALL  15
#define JMP   16
#define JMPZ  17

#define TOS (*sp)
#define NOS (*(sp-1))
#define PUSH(x) *(++sp)=(cell_t)(x)
#define POP *(sp--)
#define DROP1 sp--
#define DROP2 sp-=2
#define RET(x) PUSH(x); return;

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

void CCOMMA(cell_t x) { *(here++) = (char)x; }
void COMMA(cell_t x) { *(cell_t*)here = x; here += sizeof(cell_t); }
void CComma() { CCOMMA(POP); }
void Comma() { COMMA(POP); }

// ( d s--d )
void strCat() {
    char *s = (char*)POP;
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
    char *s = (char*)POP;
    char *d = (char*)TOS;
    while (*s && *d) {
        if (lower(*s) != lower(*d)) { break; }
        s++; d++;
    }
    TOS = (*s==*d) ? -1 : 0;
}

// ( d s--n )
void strCmp() {
    char *s = (char*)POP;
    char *d = (char*)TOS;
    while (*s && *d) { if (*s != *d) { break; } }
    TOS = (*s==*d) ? -1 : 0;
}

// ( nm-- )
void Create() {
    --last;
    PUSH(0); TOS=NOS;
    NOS = (cell_t)&last->name[0];
    strCpy();
    last->xt = here;
    last->f = 0;
}

// ( nm--xt flags 1 )
// ( nm--0 )
void find() {
    char *nm = (char*)POP;
    dict_t *x = last;
    dict_t *end = (dict_t*)&BYTES(MEM_SZ);
    while (x < end) {
        PUSH(nm); PUSH(&x->name[0]);
        strCmpI();
        if (POP) {
            PUSH(x->xt);
            PUSH(x->f);
            RET(1);
        }
        ++x;
    }
    PUSH(0);
}

// ( --n 1 )
// ( --0 )
void isDecimal(const char *wd) {
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { RET(0); }
    while (BTW(*wd, '0', '9')) { x = (x * 10) + (*(wd++) - '0'); }
    if (*wd) { RET(0); }
    PUSH(isNeg ? -x : x);
    RET(1);
    /*
    if (*wd && (*wd != '.')) { RET(0); }
    if (*wd == 0) { PUSH(isNeg ? -x : x); RET(1); }
    // Must be a '.', make it a float
    ++wd;
    float fx = (float)x, d = 10;
    while (BTW(*wd, '0', '9')) { fx += (*(wd++) - '0') / d; d *= 10; }
    if (*wd) { RET(0); }
    PUSH(0);
    // FTOS = isNeg ? -fx : fx;
    RET(1);
    */
}


// ( nm--n 1 )
// ( nm--0 )
void isNum() {
    char *wd = (char*)POP;
    if ((wd[0] == '\'') && (wd[2] == '\'') && (wd[3] == 0)) { PUSH(wd[1]); RET(1); }
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
    PUSH(x);
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
    while (*in && (*in < 32)) { ++in; }
    if (*in == 0) { RET(0); }
    PUSH(in);
    PUSH(0);
    while (32 < *in) { ++in; ++TOS; }
    *(in++) = 0;
}

#define NEXT goto next

void Run(char *y) {
    cell_t t1;
    pc = y;

next:
    // printf("-pc:%ld,ir:%d-",pc-1,(int)ir.b[0]);
    switch (*(pc++)) {
    case 0: return;
    case 1: PUSH(*(pc++));                                      NEXT;
    case 4: PUSH(*(cell_t*)pc); pc += sizeof(cell_t);           NEXT;
    case IF: CCOMMA(JMPZ); PUSH(here); COMMA(0);                NEXT;
    case ELSE: printf("-else-");                                NEXT;
    case THEN: t1=POP; *(cell_t*)(t1) = (cell_t)here;           NEXT;
    case CALL: if (*pc != ';') { rstk[++rsp] = (pc+sizeof(cell_t)); }
    case JMP: pc = *(char**)pc;         NEXT;
    case JMPZ: if (POP==0) { pc = *(char**)pc; }
             else { pc += sizeof(cell_t); }
        NEXT;
    case '#': PUSH(0); TOS=NOS;        NEXT;
    case '$': t1=TOS; TOS=NOS; NOS=t1; NEXT;
    case '*': NOS *= TOS; DROP1;       NEXT;
    case '+': NOS += TOS; DROP1;       NEXT;
    case ',': printf("%c", (char)POP); NEXT;
    case '-': NOS -= TOS; DROP1;       NEXT;
    //case '/': NOS /= TOS; DROP1;       NEXT;
    case '.': printf("%d", POP);       NEXT;
    case ';': if (rsp<1) { rsp=0; return; } pc=rstk[rsp--]; NEXT;
    case 'T': PUSH(clock());           NEXT;
    case 'X': isBye=1;                 NEXT;
    case '\\': DROP1;                  NEXT;
    case 'd': --TOS;                   NEXT;
    case 'i': ++TOS;                   NEXT;
    case '[': lsp+=3; L2=(cell_t)pc; L0=POP; L1=POP;   NEXT;
    case ']': if (++L0<L1) { pc=(char*)L2; } else { lsp-=3; }; NEXT;
    case '{': lsp+=3; L0=(cell_t)pc;   NEXT;
    case '}': if (POP) { pc=(char*)L0; } else { lsp-=3; }; NEXT;
    default: printf("-[%d]?-",(int)*(pc-1));  break;
    }
}

void doInline(char *x) {
    CCOMMA(*(x++));
    while ((*x) && (*x != ';')) { CCOMMA(*(x++)); }
}

void ParseWord() {
    char *w = (char*)TOS;
    isNum();
    if (POP) {
        // if (state) { PUSH(4); CComma(); Comma(); }
        CCOMMA(4); Comma();
        RET(1);
    }
    PUSH(w);
    find();
    if (POP) {
        cell_t f = POP;
        char *xt = (char*)POP;
        //if (state == 0) { Run(xt); }
        if (f & 0x01) { Run(xt); }
        else if (f & 0x02) { doInline(xt); }
        else { CCOMMA(CALL); COMMA((cell_t)xt); }
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
    char *chere = here;
    while (isBye == 0) {
        getword();
        if (POP == 0) break;
        ParseWord();
        if (POP == 0) return;
    }
    PUSH(';'); CComma();
    Run(chere);
    here = chere;
}

void loadLine(const char *x) {
    in = tib;
    while (*x) { *(in++) = *(x++); }
    *in = 0;
    ParseLine(tib);
}

void loadPrim(const char *nm, int f, int n, char c1, char c2) {
    PUSH(nm); Create();
    last->f = f;
    *(here++) = c1;
    if (1 < n) { *(here++) = c2; }
    *(here++) = ';';
}

void loadPrimXX(const char *name, int flags, const char *code) {
    PUSH(name); Create();
    last->f = flags;
    while (*code) { *(here++) = *(code++); }
    *(here++) = ';';
}

void init() {
    here = &BYTES(0);
    last = (dict_t*)&BYTES(MEM_SZ);
    sp = stk;
    in = tib;
    base = 10;
    loadPrim("bye", 0, 1, 'X', 0);
    loadPrim("cell", 2, 2, 1, sizeof(cell_t));
    loadPrim("exit",  0, 1, ';', 0);
    loadPrim("timer", 2, 1, 'T', 0);
    loadPrim("if",    1, 1, IF, 0);
    loadPrim("else",  1, 1, ELSE, 0);
    loadPrim("then",  1, 1, THEN, 0);
    loadPrim("dup",   2, 1, '#', 0);
    loadPrimXX("drop", 2, "\\");
    loadPrimXX("over", 2, "%");
    loadPrimXX("swap", 2, "$");
    loadPrimXX("/", 2, "/");
    loadPrimXX("*", 2, "*");
    loadPrimXX("-", 2, "-");
    loadPrimXX("+", 2, "+");
    loadPrimXX(".", 2, ".");
    loadPrimXX("emit", 2, ",");
    loadPrimXX("1-", 2, "d");
    loadPrimXX("1+", 2, "i");
    loadPrimXX("begin", 2, "{");
    loadPrimXX("while", 2, "}");
    loadPrimXX("do", 2, "[");
    loadPrimXX("loop", 2, "]");
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
