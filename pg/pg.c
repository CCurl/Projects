#include <stdio.h>
#include <stdlib.h>

#define CODE_SZ       32767
#define VARS_SZ       65536
#define DICT_SZ        8192
#define NPRIMS           32
#define STK_SZ           64
#define btwi(a,b,c)   ((b<=a) && (a<=c))
#define comma(x)      code[here++]=(x)
#define call(x)       comma((x<<1)+1)
#define jmp(x)        comma((x<<1)+0)

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
static unsigned short wc, code[CODE_SZ+1], u;
static int here, last, state, base, sp, rsp, pc;

DE_T *addWord(const char *wd);

enum {
    STOP, LIT1, LIT2, EXIT, DUP, SWAP, DROP, FOR, INDEX, NEXT,   //  0 ->  9
    EMIT, DOT, JMPZ, ADD, SUB, MUL, DIV, P17, P18, P19,          // 10 -> 19
    P20, P21, P22, P23, P24, P25, P26, P27, COLON, SEMI,         // 20 -> 29
    IMM, BYE                                                     // 30 -> 31
};

void p00() { pc = -1; }
void p01() { PUSH(code[pc++]); }
void p02() { long x=code[pc+1]; x=(x<<16)+code[pc]; PUSH(x); pc += 2; }
void p03() { if (rsp) { pc=rstk[rsp--].i; } else { pc=-1; } }
void p04() { long t=TOS; PUSH(t); }
void p05() { long t=TOS, n=NOS; NOS=t; TOS=n;  }
void p06() { sp -= sp ? 1 : 0; }
void p07() { lsp += 3; L2=pc; L1=POP(); L0=0; }
void p08() { PUSH(L0); }
void p09() { if (++L0<L1) { pc=L2; } else { lsp-=3; } }
void p10() { printf("%c", (char)POP()); }
void p11() { printf("%ld", POP()); }
void p12() { if (POP()==0) { pc=code[pc]; } else { ++pc; } }
void p13() { long t=POP(); TOS+=t; }
void p14() { long t=POP(); TOS-=t; }
void p15() { long t=POP(); TOS*=t; }
void p16() { long t=POP(); TOS/=t; }
void p17() { }
void p18() { }
void p19() { }
void p20() { }
void p21() { }
void p22() { }
void p23() { }
void p24() { }
void p25() { }
void p26() { }
void p27() { }
void p28() { addWord(0); state=1; }
void p29() { comma(EXIT); state=0; }
void p30() { dict[last-1].fl=1; }
void p31() { printf("\n"); exit(0); }

void (*q[NPRIMS])() = {
    p00,p01,p02,p03,p04,p05,p06,p07,p08,p09,p10,p11,p12,p13,p14,p15,
    p16,p17,p18,p19,p20,p21,p22,p23,p24,p25,p26,p27,p28,p29,p30,p31
};

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
    de->xt = here;
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

void Exec(int start) {
    pc = start;
    while (btwi(pc,0,CODE_SZ)) {
        wc = code[pc++];
        if (wc<NPRIMS) { q[wc](); }
        else {
            int addr = (wc>>1);
            if (wc & 0x01) { rstk[++rsp].i=pc; }
            pc=addr;
        }
    }
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

    if (isNum(wd, base)) {
        long n = POP();
        if (btwi(n,0,0x7fff)) {
            comma(1); comma(n);
        } else {
            comma(2); comma(n & 0xffff); comma(n >> 16);
        }
        return 1;
    }

    DE_T *de = findWord(w);
    if (de) {
        if (de->fl == 1) {   // IMMEDIATE
            if (de->xt < NPRIMS) { q[de->xt](); }
            else {
                code[here+10]=de->xt & 0x01;
                code[here+11]=EXIT;
                Exec(here+10);
            }
        } else {
            if (de->xt < NPRIMS) { comma(de->xt); }
            else { call(de->xt); }
        }
        return 1;
     }

    return 0;
}

int parseLine(const char *ln) {
    int h=here, l=last;
    toIn = (char *)ln;
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
    here = NPRIMS;
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
    addPrim(":",    COLON); p30();
    addPrim(";",    SEMI);  p30();
    addPrim("IMMEDIATE",  IMM);
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc>1) { fp=fopen(argv[1], "rt"); }
    Init();
    while (1) { fp = REP(fp); }; // REPL
    return 0;
}
