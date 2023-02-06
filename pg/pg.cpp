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
typedef struct { char name[16]; char *xt; int f; } dict_t;

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

void CComma() {*here = (char)POP; here += 1; }
void Comma() {*(cell_t *)here = POP; here += sizeof(cell_t); }

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

// ( nm--n 1 )
// ( nm--0 )
void isNum() {
    char *w = (char*)TOS;
    TOS = 0;
    cell_t neg = (*w=='-') ? 1 : 0;
    w += (neg) ? 1 : 0;
    if (!BTW(*w, '0', '9')) { return; }
    TOS = *(w++) - '0';
    while (BTW(*w, '0', '9')) { TOS = (TOS * 10) + *(w++) - '0'; }
    if (*w) { TOS = 0; return; }
    if (neg) { TOS = -TOS; }
    PUSH(1);
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
    case 4: PUSH(*(cell_t*)pc); pc += sizeof(cell_t); NEXT;
    case 10: if (*pc!=';') { rstk[++rsp]=(pc+sizeof(char*)); }
    case 11: pc = *(char**)pc;         NEXT;
    case ';': if (rsp < 1) { rsp = 0; return; }
            pc = rstk[rsp--];          NEXT;
    case '#': PUSH(0); TOS=NOS;        NEXT;
    case '$': t1=TOS; TOS=NOS; NOS=t1; NEXT;
    case '+': NOS += TOS; DROP1;       NEXT;
    case '-': NOS -= TOS; DROP1;       NEXT;
    case '*': NOS *= TOS; DROP1;       NEXT;
    case '/': NOS /= TOS; DROP1;       NEXT;
    case '\\': DROP1;                  NEXT;
    case '.': printf("%d", POP);       NEXT;
    case 'T': PUSH(clock());           NEXT;
    case 'X': isBye=1;                 return;
    case 'd': --TOS;                   NEXT;
    case 'i': ++TOS;                   NEXT;
    case '{': lsp+=3; L0=(cell_t)pc;   NEXT;
    case '}': if (POP) { pc=(char*)L0; } else { lsp-=3; }; NEXT;
    case 127: isBye = 1;               return;
    default: printf("-[%d]?-",(int)*(pc-1));  NEXT;
    }
}

void doInline(char *x) {
    PUSH(*(x++)); CComma();
    while ((*x) && (*x != ';')) { PUSH(*(x++)); CComma(); }
}

void ParseWord() {
    char *w = (char*)TOS;
    isNum();
    if (POP) {
        // if (state) { PUSH(4); CComma(); Comma(); }
        PUSH(4); CComma(); Comma();
        RET(1);
    }
    PUSH(w);
    find();
    if (POP) {
        cell_t f = POP;
        char *xt = (char*)POP;
        //if (state == 0) { Run(xt); }
        //else if (f & 0x01) { Run(xt); }
        //else
        if (f & 0x02) { doInline(xt); }
        else { PUSH(xt); PUSH(10); CComma(); Comma(); }
        RET(1);
    }
    printf("[%s]??", w);
    if (state) {
        ++last;
        state = 0;
    }
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
    Run(chere);
    here = chere;
}

void loadLine(const char *x) {
    in = tib;
    while (*x) { *(in++) = *(x++); }
    *in = 0;
    ParseLine(tib);
}

void loadPrim(const char *name, int flags, const char *code) {
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
    loadPrim("bye", 0, "X");
    loadPrim("exit", 2, ";");
    loadPrim("timer", 2, "T");
    loadPrim("dup", 2, "#");
    loadPrim("drop", 2, "\\");
    loadPrim("over", 2, "%");
    loadPrim("swap", 2, "$");
    loadPrim("/", 2, "/");
    loadPrim("*", 2, "*");
    loadPrim("-", 2, "-");
    loadPrim("+", 2, "+");
    loadPrim(".", 2, ".");
    loadPrim("1-", 2, "d");
    loadPrim("1+", 2, "i");
    loadPrim("begin", 2, "{");
    loadPrim("while", 2, "}");
    loadLine("234 dup + .");
    // loadLine(": cell 4 ;");
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
