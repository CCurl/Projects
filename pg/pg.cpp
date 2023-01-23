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

#define BTW(a,b,c) ((b<=a) && (a<=c))

// #define ACC          regs[dstReg]
#define RG(x)        regs[(x)]
// #define RG(x)        regs[(x)-'A']

#define PC           *(pc)
#define IR           *(pc-1)
#define NR           *(pc++)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]

#define BYTES(x)      mem.b[x]
#define CELLS(x)      mem.c[x]

typedef long cell_t;
typedef union { cell_t c; unsigned char b[sizeof(cell_t)]; } val_t;
union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;
typedef struct { char name[16]; cell_t p; } dict_t;

cell_t stk[STK_SZ], sp;
cell_t st[64], s;
cell_t lstk[LSTK_SZ+1], lsp;
cell_t regs[REGS_SZ], last, here, chere;
val_t pgm[1024];
dict_t dict[1024];

#ifdef isPC
FILE *input_fp;
int isBye;
#endif

void init() {
    here = 0;
    last = 0;
    pgm[here++].b[0] = ';';
}

int create(const char *name, int addr) {
    strcpy(dict[last].name, name);
    dict[last].p = addr;
    return (last++);
}

int find(const char *name) {
    for (int i=0; i<last; i++) {
        if (strcmp(dict[i].name, name) == 0) { return dict[i].p; }
    }
    return 0;
}

int isnum(char *w, cell_t *val) {
    *val = 0;
    cell_t t1, neg = (*w=='-') ? 1 : 0;
    w += (neg) ? 1 : 0;
    if (!BTW(*w, '0', '9')) { return 0; }
    t1 = *(w++) - '0';
    while (BTW(*w, '0', '9')) { t1 = (t1 * 10) + *(w++) - '0'; }
    if (*w) { return 0; }
    *val = neg ? -t1 : t1;
    return 1;
}

int getword(char *w) {
    int l = 0;
    *w = 0;
    return l;
}

#define LIT1   1
#define LIT2   2
#define RLIT1  3
#define RLIT2  4
#define FOR    '['
#define NXT    ']'
#define NDX    'I'
#define MOV   10
#define ADD   '+'
#define SUB   '-'
#define MUL   '*'
#define DIV   '/'
#define MOD   '%'
#define SMOD  25
#define PUSH  30
#define PREG  31
#define SREG  32
#define LT    '<'
#define LTE   '<'+128
#define EQ    '='
#define GTE   '>'+128
#define GT    '>'
#define IF    '?'
#define JMP   'J'
#define LNOT  '~'
#define DOT   '.'
#define TIMER 'T'

#define PGC(x) pgm[x].c
#define PGB(x,y) pgm[x].b[y]

void comp(unsigned char ir, char a1, char a2, char a3) {
    PGB(here, 0) = ir;
    PGB(here, 1) = a1;
    PGB(here, 2) = a2;
    PGB(here++, 3) = a3;
}

void compLit(cell_t num, char reg, int isReg) {
    char lit1 = isReg ? RLIT1 : LIT1;
    char lit2 = isReg ? RLIT2 : LIT2;
    cell_t mx = isReg ? 0x0000FFFF : 0x00FFFFFF;
    if (num <= mx) {
        PGC(here) = isReg ? (num << 16) : (num << 8);
        if (isReg) { PGB(here, 1) = reg; }
        PGB(here++, 0) = lit1;
    } else {
        PGB(here, 0) = lit2;
        PGB(here++, 1) = reg;
        pgm[here++].c = num;
    }
}

typedef struct { char op, src, dst; cell_t num; } cc_t;

const char *getNum(const char *str) {
    st[++s]=0;
    while (*str==' ') { ++str; }
    if (BTW(*str,'0','9')) {
        st[s] = *(str++) - '0';
        while (BTW(*str, '0', '9')) { st[s] = (st[s] * 10) + *(str++) - '0'; }
        st[++s]=1;
        while (*str == ' ') { ++str; }
    }
    return str;
}

#define RA(x) ((x)-'A')
#define NS *(src++)
#define S0 *(src)
#define S1 *(src+1)
#define S2 *(src+2)
#define S3 *(src+3)

void parse(const char *src) {
    cell_t t1, t2, b1;
    //char r1;
    //cc_t cc;

    while (*src) {
        switch(*(src++)) {
            case ' ': break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                src = getNum(src - 1); --s;
                if (S0 == 's') { ++src; compLit(st[s--],RA(NS),1); }
                else { compLit(st[s--], 0, 0); }
                break;
            case 'm': if (S0=='v') { comp(MOV,S1,S2,0); src += 3; }
                    break;
            case '+': comp(ADD, RA(S0), RA(S1), RA(S2)); src+=3; break;
            case '-': comp(SUB, RA(S0), RA(S1), RA(S2)); src+=3; break;
            case '*': comp(MUL, RA(S0), RA(S1), RA(S2)); src += 3; break;
            case '%': comp(MOD, 0, 0, 0); break;
            case '<': if (S0 == '=') { comp(LTE, 0, 0, 0); ++src; }
                    else { comp(LT, 0, 0, 0); }
                    break;
            case '=': comp(EQ, 0, 0, 0); break;
            case '>': if (S0 == '=') { comp(GTE, 0, 0, 0); ++src; }
                    else { comp(GT, 0, 0, 0); }
                    break;
            case '.': comp(DOT, NS, 0, 0); break;
            case '/': if (S0 == '%') { comp(SMOD, S1, S2, 0); src += 3; }
                    else { comp(DIV, RA(S0), RA(S1), RA(S2)); src += 3; }break;
            case '(': st[++s]=here; comp(IF, 0, 0, 0); break;
            case ')': t1=st[s--]; pgm[t1].c=(here<<8); PGB(t1,0)=IF; break;
            case '[': comp(FOR,0,0,0); break;
            case 'I': comp(NDX,0,0,0); break;
            case ']': comp(NXT,0,0,0); break;
            case 'r': comp(PREG,RA(NS),0,0); break;
            case 's': comp(SREG,RA(NS),0,0); break;
            case 'x': t1 = NS; if (t1=='T') { comp(TIMER,0,0,0); }
                break;
            case '~': comp(LNOT,0,0,0); break;
            //case '!': pgm[here].b[0] = cc.op;
            //    pgm[here].b[1] = cc.dst;
            //    pgm[here++].b[2] = cc.src;
            //    if (cc.op == LIT) { pgm[here++].c = cc.num; }
            //    break;
        }
    }
    pgm[here].c = 0;
}

#define NEXT goto next
#define IRB(x) ir->b[x]

val_t *pc, *ir;

void Run(cell_t x) {
    sp = lsp = 0;
    pc = &pgm[x];
    cell_t t1;

next:
    ir = (pc++);
    // printf("-pc:%ld,ir:%d-",pc-1,(int)ir.b[0]);
    switch (IRB(0)) {
    case 0: return;
    case LIT1: st[++s] = ir->c >> 8; NEXT;
    case LIT2: st[++s] = (pc++)->c; NEXT;
    case RLIT1: regs[IRB(1)] = ir->c >> 16; NEXT;
    case RLIT2: regs[IRB(1)] = (pc++)->c; NEXT;
    case FOR: lsp+=3; L0=st[s--]; L1=st[s--]; L2=(cell_t)pc; NEXT;
    case NDX: st[++s] = L0; NEXT;
    case NXT: if (++L0<L1) { pc=(val_t*)L2; } else { lsp-=3; } NEXT;
    case DOT: t1 = IRB(1); if (t1 == '.') { printf("%ld", st[s--]); }
            else if (t1 == 'b') { printf(" "); }
            else if (t1 == 'n') { printf("\n"); }
            else if (BTW(t1, 'A', 'Z')) { printf("%ld", regs[RA(IRB(1))]); }
            NEXT;
    case MOV: regs[IRB(1)] = regs[IRB(2)]; NEXT;
    case ADD: regs[IRB(1)] = regs[IRB(2)] + regs[IRB(3)]; NEXT;
    case SUB: regs[IRB(1)] = regs[IRB(2)] - regs[IRB(3)]; NEXT;
    case MUL: regs[IRB(1)] = regs[IRB(2)] * regs[IRB(3)]; NEXT;
    case DIV: regs[IRB(1)] = regs[IRB(2)] / regs[IRB(3)]; NEXT;
    case MOD: --s; st[s] = st[s] % st[s+1]; NEXT;
    case SMOD: --s; regs['Q'] = st[s] % st[s+1];
        regs['R'] = st[s] % st[s+1]; --s; NEXT;
    case LT:  st[s-1] = (st[s-1] <  st[s]) ? -1 : 0; --s; NEXT;
    case LTE: st[s-1] = (st[s-1] <= st[s]) ? -1 : 0; --s; NEXT;
    case EQ:  st[s-1] = (st[s-1] == st[s]) ? -1 : 0; --s; NEXT;
    case GTE: st[s-1] = (st[s-1] >= st[s]) ? -1 : 0; --s; NEXT;
    case GT:  st[s-1] = (st[s-1] >  st[s]) ? -1 : 0; --s; NEXT;
    case IF: if (st[s--] == 0) { pc = (val_t*)(&pgm[(ir->c) >> 8]); } NEXT;
    case JMP: pc = (val_t*)((ir->c) >> 8); NEXT;
    case PUSH: st[++s] = (pc++)->c; NEXT;
    case PREG: st[++s]= regs[IRB(1)]; NEXT;
    case SREG: regs[IRB(1)]=st[s--]; NEXT;
    case LNOT: st[s] = st[s] ? 0 : -1; NEXT;
    case TIMER: st[++s] = clock(); NEXT;
    default: printf("-[%d]?-",(int)IRB(0));
    }
}

#ifdef isPC
void Loop() {
    char y[256];
    y[0] = 0;
    int sz = sizeof(y);
    chere = here;
    if (input_fp) {
        if (fgets(y, sz, input_fp) != y) {
            fclose(input_fp);
            input_fp = NULL;
        }
    }
    if (!input_fp) {
        // putchar('\n'); putchar('q'); putchar('4'); putchar('>');
        printf("\nq5:(%ld)> ", s);
        if (fgets(y, sz, stdin) != y) { isBye=1; return; }
    }
    if (y[0]) {
        parse(y);
        Run(chere);
        here = chere;
    }
}

void test(const char *src) {
    init();
    parse(src);
    cell_t s = clock();
    Run(1);
    cell_t e = clock();
    printf("time: %ld (%ld)\n", e - s, (e - s) / 1000);
}

int main(int argc, char *argv[]) {
    // int r='A';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG(r++) = atoi(y); }
    //test("1 2 < (12345sD)");
    // test("500000000 1[*YMX+YYB]");
    // test("500000000 1[I 3 % ~ (123 sF)]");
    init();
    input_fp = fopen("src.q5", "rb");
    while (isBye == 0) { Loop(); }
    return 0;
}
#endif
