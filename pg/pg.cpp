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
typedef union { cell_t c; char b[sizeof(cell_t)]; } val_t;
union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;
typedef struct { char name[16]; cell_t p; } dict_t;

cell_t regs[REGS_SZ], lstk[LSTK_SZ+1];
cell_t stk[STK_SZ];
cell_t sp, lsp, last, here;
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

#define PGC(x) pgm[x].c
#define PGB(x,y) pgm[x].b[y]

#define LIT    1
#define FOR    2
#define NXT    3
#define MOV   10
#define ADD   20
#define SUB   21
#define MUL   22
#define DIV   23

void comp(char ir, char a1, char a2, char a3) {
    PGB(here,0) = ir;
    PGB(here,1) = a1;
    PGB(here,2) = a2;
    PGB(here++,3) = a3;
}
void litC(cell_t v, char reg) { comp(LIT,reg,0,0); pgm[here++].c = v; }
void forC(char f, char t) { comp(FOR,f,t,0);  }
void nxtC() { comp(NXT,0,0,0);  }
void movC(char d, char s) { comp(MOV,d,s,0);  }

cell_t numArg(char *w) {
    cell_t v;
    getword(w);
    if (isnum(w, &v)) {  }
    return v;
}

int parseword(char *w) {
    cell_t v;
    if (isnum(w, &v)) {
        litC(v, 1);
        return 1;
    }
    if (strcmp(w, "")==0) {
        return 1;
    }
    if (strcmp(w, "")==0) {
        return 1;
    }
    if (strcmp(w, "")==0) {
        return 1;
    }
    return 0;
}

void parse() {
    char w[32];
    while (getword(w)) {
        if (parseword(w) == 0) { return; }
    }
}

#define NEXT goto next
#define IRB(x) ir->b[x]

val_t *pc, *ir;


void Run(cell_t x) {
    sp = lsp = 0;
    pc = &pgm[x];

next:
    ir = (pc++);
    // printf("-pc:%ld,ir:%d-",pc-1,(int)ir.b[0]);
    switch (IRB(0)) {
    case 0: return;
    case LIT: regs[IRB(1)] = (pc++)->c; NEXT;
    case FOR: lsp+=3; L0=regs[IRB(1)]; L1=regs[IRB(2)]; L2=(cell_t)pc; NEXT;
    case NXT: if (++L0<L1) { pc=(val_t*)L2; } else { lsp-=3; } NEXT;
    case MOV: regs[IRB(1)] = regs[IRB(2)]; NEXT;
    case ADD: regs[IRB(1)] += regs[IRB(2)]; NEXT;
    case SUB: regs[IRB(1)] -= regs[IRB(2)]; NEXT;
    case MUL: regs[IRB(1)] *= regs[IRB(2)]; NEXT;
    case DIV: regs[IRB(1)] /= regs[IRB(2)]; NEXT;
    default: printf("-[%d]?-",(int)IRB(0));
    }
}

#ifdef isPC
void Loop() {
    // char *y = here;
    // int sz = &BYTES(MEM_SZ)-y-1;
    // if (input_fp) {
    //     if (fgets(y, sz, input_fp) != y) {
    //         fclose(input_fp);
    //         input_fp = NULL;
    //     }
    // }
    // if (!input_fp) {
    //     // putchar('\n'); putchar('q'); putchar('4'); putchar('>');
    //     //printf("\nq4:(%ld)> ", acc);
    //     if (fgets(y, sz, stdin) != y) { isBye=1; return; }
    // }
    // Run(y);
}
int main(int argc, char *argv[]) {
    // int r='A';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG(r++) = atoi(y); }
    init();
    //input_fp = fopen("src.q4", "rb");
    //while (isBye == 0) { Loop(); }
    litC(0,1);
    litC(500*1000*1000,2);
    forC(1,2);
    comp(MOV,24,12,0);
    comp(MUL,24,23,0);
    comp(ADD,24,1,0);
    nxtC();
    comp(0,0,0,0);
    cell_t s = clock();
    Run(1);
    cell_t e = clock();
    printf("time: %ld (%ld)\n",e-s,(e-s)/1000);
    return 0;
}
#endif
