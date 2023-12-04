#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CODE_SZ       0x7FFF
#define VARS_SZ       0xFFFF
#define DICT_SZ         8192
#define LPRIM             31
#define STK_SZ            64
#define btwi(a,b,c)   ((b<=a) && (a<=c))
#define comma(x)      code[here++]=(x)
#define call(x)       comma(x|0x8000)
#define jmp(x)        comma(x&0x7FFF)
#define NCASE         goto next; case

#define TOS           stk[sp].i
#define NOS           stk[sp-1].i
#define PUSH(x)       stk[++sp].i=(x)
#define POP()         stk[sp--].i
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]


typedef unsigned short ushort;
typedef unsigned char byte;
typedef union { double f; long i; } SE_T;
typedef struct { ushort xt; byte fl; byte ln; char nm[28]; } DE_T;

SE_T stk[STK_SZ], rstk[STK_SZ];
DE_T dict[DICT_SZ];
static int lstk[60], lsp;
static char tib[128];
static ushort wc, code[CODE_SZ+1];
static int here, last, state, base, sp, rsp, pc;

DE_T *addWord(const char *wd);

enum {
    STOP, LIT1, LIT2, EXIT, DUP, SWAP, DROP, FOR, INDEX, NEXT,   //  0 ->  9
    EMIT, DOT, JMPZ, ADD, SUB, MUL, DIV, P17, P18, P19,          // 10 -> 19
    TMR, P21, P22, P23, P24, P25, P26, P27, COLON, SEMI,         // 20 -> 29
    IMM, BYE                                                     // 30 -> 31
};

long fetch2(ushort *a) { return (*(a+1))<<16 | *(a); }
void makeImm() { dict[last-1].fl=1; }

void Exec(int start) {
    pc = start;
    // printf("-ex:%d-",pc);
    next:
    wc=code[pc++];
    switch(wc) {
        case   0: { return; }
        NCASE  1: { PUSH(code[pc++]); }
        NCASE  2: { PUSH(fetch2(&code[pc])); pc += 2; }
        NCASE  3: { if (rsp) { pc=rstk[rsp--].i; } else { return; } }
        NCASE  4: { long t=TOS; PUSH(t); }
        NCASE  5: { long t=TOS, n=NOS; NOS=t; TOS=n;  }
        NCASE  6: { sp -= sp ? 1 : 0; }
        NCASE  7: { lsp += 3; L2=pc; L1=POP(); L0=0; }
        NCASE  8: { PUSH(L0); }
        NCASE  9: { if (++L0<L1) { pc=L2; } else { lsp-=3; } }
        NCASE 10: { printf("%c", (char)POP()); }
        NCASE 11: { printf("%ld", POP()); }
        NCASE 12: { if (POP()==0) { pc=code[pc]; } else { ++pc; } }
        NCASE 13: { long t=POP(); TOS+=t; }
        NCASE 14: { long t=POP(); TOS-=t; }
        NCASE 15: { long t=POP(); TOS*=t; }
        NCASE 16: { long t=POP(); TOS/=t; }
        NCASE 17: { }
        NCASE 18: { }
        NCASE 19: { }
        NCASE 20: { PUSH(clock()); }
        NCASE 21: { }
        NCASE 22: { }
        NCASE 23: { }
        NCASE 24: { }
        NCASE 25: { }
        NCASE 26: { }
        NCASE 27: { }
        NCASE 28: { addWord(0); state=1; }
        NCASE 29: { comma(EXIT); state=0; }
        NCASE 30: { makeImm(); }
        NCASE 31: { printf("\n"); exit(0); }
            goto next;
        default: {
            if (wc & 0x8000) { rstk[++rsp].i=pc; }
            pc = (wc & 0x7FFF );
            goto next;
        }
    }
}

char wd[32], *toIn;
int nextWord() {
    int l = 0;
    while (*toIn && (*toIn<33)) { ++toIn; }
    while (*toIn && (*toIn>32)) { wd[l++] = *(toIn++); }
    wd[l] = 0;
    return l;
}

int lower(char c) { return btwi(c,'A','Z') ? c+32 : c; }
int strLen(const char *s) { int l=0; while (s[l]) { l++; } return l; }
int strEqI(const char *s, const char *d) {
    while (lower(*s) == lower(*d)) { if (*s==0) { return 1; } s++; d++; }
    return 0;
}


DE_T *addWord(const char *w) {
    if (!w) { nextWord(); w=wd; }
    int l = strLen(w);
    if (l==0) return (DE_T*)0;
    if (l>27) { l=27; }
    DE_T *de=(DE_T*)&dict[last++];
    de->xt = here | 0x8000;
    de->fl = 0;
    de->ln = l;
    for (int i = 0; i < l; i++) { de->nm[i] = w[i]; }
    de->nm[l]=0;
    return de;
}

DE_T *findWord(const char *w) {
    if (!w) { nextWord(); w=wd; }
    for (int e=last-1; 0<=e; e-- ) {
        DE_T *de = (DE_T*)&dict[e];
        if (strEqI(de->nm, w)) { return de; }
    } 

    return (DE_T*)0;
}

long isNum(const char *w, int b) {
    long n = 0;
    b = b ? b : 10;
    while (btwi(*w,'0','9')) {
        n = (n*b) + *(w++)-'0';
    }
    if (*w) { return 0; }
    PUSH(n);
    return 1;
}

int parseWord(char *w) {
    if (!w) { w = &wd[0]; }
    // printf("-pw:%s-",w);

    if (isNum(wd, base)) {
        long n = POP();
        if (btwi(n,0,0x7fff)) {
            comma(1); comma((short)n);
        } else {
            comma(2); comma(n & 0xffff); comma(n >> 16);
        }
        return 1;
    }

    DE_T *de = findWord(w);
    if (de) {
        if (de->fl == 1) {   // IMMEDIATE
            int h = here+10;
            code[h]=de->xt;
            if (de->xt > LPRIM) { code[h] |= 0x8000; }
            code[h+1]=EXIT;
            Exec(h);
        } else {
            if (de->xt <= LPRIM) { comma(de->xt); }
            else { call(de->xt); }
        }
        return 1;
     }

    return 0;
}

int parseLine(const char *ln) {
    int h=here, l=last;
    toIn = (char *)ln;
    // printf("-pl:%s-",ln);
    while (nextWord()) {
        if (!parseWord(wd)) {
            printf("-%s?-", wd);
            here=h;
            last=l;
            state=0;
            return 0;
        }
    }
    if ((l==last) && (h<here) && (state==0)) {
        comma(0);
        here=h;
        Exec(h);
    }
    return 1;
}

// REP - Read/Execute/Print (no Loop)
FILE *REP(FILE *fp) {
    if (!fp) { fp = stdin; }
    if (fp == stdin) { printf(" ok\n"); }
    if (fgets(tib, sizeof(tib), fp) == tib) {
        parseLine(tib);
        return fp;
    }
    if (fp == stdin) { exit(0); }
    fclose(fp);
    return NULL;
}

void addPrim(const char *wd, ushort prim) {
    addWord(wd)->xt = prim;
}

void Init() {
    int t;
    sp = rsp = lsp = state = last = 0;
    base = 10;
    here = 0;
    for (t=0; t<CODE_SZ; t++) { code[t]=0; }
    addPrim("BYE",  BYE);
    addPrim("EXIT", EXIT);
    addPrim("DUP",  DUP);
    addPrim("SWAP", SWAP);
    addPrim("DROP", DROP);
    addPrim("FOR",  FOR);
    addPrim("I",    INDEX);
    addPrim("NEXT", NEXT);
    addPrim("EMIT", EMIT);
    addPrim(".",    DOT);
    addPrim("+",    ADD);
    addPrim("-",    SUB);
    addPrim("*",    MUL);
    addPrim("/",    DIV);
    addPrim("TIMER",  TMR);
    addPrim(":",    COLON); makeImm();
    addPrim(";",    SEMI);  makeImm();
    addPrim("IMMEDIATE",  IMM);
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc>1) { fp=fopen(argv[1], "rt"); }
    Init();
    while (1) { fp = REP(fp); }; // REPL
    return 0;
}
