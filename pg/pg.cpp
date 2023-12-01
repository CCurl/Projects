#include <stdio.h>
#include <stdlib.h>

#define CODE_SZ       32767
#define VARS_SZ       65536
#define NPRIMS           32
#define STK_SZ           64
#define btwi(a,b,c)   ((b<=a) && (a<=c))
#define comma(x)      code[here++]=(x)
#define call(x)       comma((x<<1)+1)
#define jmp(x)        comma((x<<1)+0)

typedef unsigned short ushort;
typedef unsigned char byte;
typedef union { double f; long i; } SE_T;
typedef struct { ushort xt; byte fl; byte ln; char nm[28]; } DE_T;

SE_T stk[STK_SZ], rstk[STK_SZ];
static char tib[128];
static unsigned short wc, code[CODE_SZ+1], u;
static int here, last, state, base, sp, rsp, pc;

void p00() { pc = -1; }
void p01() { }
void p02() { }
void p03() { if (rsp) { pc=rstk[rsp--].i; } else {pc=-1;} }
void p04() { }
void p05() { }
void p06() { }
void p07() { }
void p08() { }
void p09() { }
void p10() { }
void p11() { }
void p12() { }
void p13() { }
void p14() { }
void p15() { }
void p16() { }
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
void p28() { }
void p29() { }
void p30() { }
void p31() { exit(0); }

void (*q[127])() = {
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

int strLen(const char *s) { int l=0; while (s[l]) { l++; } return l; }
int strEq(const char *s, const char *d) {
    while (*s == *d) { if (*s==0) { return 1; } s++; d++; }
    return 0;
}


DE_T *addWord(const char *wd) {
    last -= sizeof(DE_T)/2;
    int l = strLen(wd);
    if (l>27) { l=27; }
    DE_T *de=(DE_T*)&code[last];
    de->xt = here;
    de->fl = 0;
    de->ln = l;
    for (int i = 0; i < l; i++) { de->nm[i] = wd[i]; }
    de->nm[l]=0;
    return de;
}

DE_T *findWord(const char *wd) {
    DE_T *e=(DE_T*)&code[CODE_SZ-1];
    DE_T *de = (DE_T*)&code[last];
    while (de < e) {
        if (strEq(de->nm, wd)) { return de; }
        else { de++; }
    } 

    return (DE_T*)0;
}

void wordCode(ushort wc) {
    int addr = (wc>>1);
    if (wc & 0x01) { printf("call-%d]",addr); rstk[++rsp].i=pc; pc=addr; }
    else { printf("jmp-%d]",addr); pc=addr; }
}

void Exec(int start) {
    printf("[Exec:%d]", start);
    pc = start;
    while (btwi(pc,0,CODE_SZ)) {
        wc = code[pc++];
        printf("[%d:wc-%d,", pc-1, wc);
        if (wc<32) { printf("p]"); q[wc](); }
        else { wordCode(wc); }
    }
}

int parseWord(char *w) {
    if (!w) { w = &wd[0]; }
    DE_T *de =  findWord(w);
    if (de) { call(de->xt); return 1; }
    return 0;
}

// REP - Read/Execute/Print (no Loop)
void REP() {
    int h=here, l=last;
    printf("\ntxt: ");
    fgets(tib, sizeof(tib), stdin);
    toIn = &tib[0];
    while (nextWord()) {
        if (!parseWord(wd)) {
            printf("-%s?-", wd);
            here=h;
            last=l;
            state=0;
            return;
        }
    }
    if ((l==last) && (h<here)) {
        comma(0);
        here=h;
        Exec(h);
    }
}

void Init(FILE *fp) {
    int t;
    sp = rsp = state = 0;
    base = 10;
    here = NPRIMS;
    last = CODE_SZ;
    for (t=0; t<CODE_SZ; t++) { code[t]=0; }
    if (fp) {
        while ((t=fgetc(fp))!=EOF) { if (31<t) { code[here++]=t-32; } }
        fclose(fp); Exec(0);
    }
    addWord("test"); comma(2); comma(3);
    addWord("test2"); call(32); jmp(32);
    addWord("bye"); comma(31);
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc>1) { fp=fopen(argv[1], "rb"); }
    Init(fp);
    while (1) { REP(); }; // REPL
    return 0;
}
