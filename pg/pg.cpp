#include <stdio.h>
#include <stdlib.h>

#define btw(a,b,c) ((b<a) && (a<c))
#define btwi(a,b,c) ((b<=a) && (a<=c))

#define CODE_SZ   32767
#define VARS_SZ   65536
#define WC_SZ        99
#define STK_SZ       64

typedef unsigned short ushort;
typedef unsigned char byte;
typedef union { double f; long i; } SE_T;
typedef struct { ushort xt; byte fl; byte ln; char nm[28]; } DE_T;

SE_T stk[STK_SZ], rstk[STK_SZ];
static char tib[128];
static unsigned short wc, code[CODE_SZ+1], u;
static int here, last, state, base, sp, rsp, pc;

void p00() { printf("-p]"); pc = -1; }
void p01() { printf("-p]"); exit(0); }
void p02() { printf("-p]"); }
void p03() { printf("-p]"); }
void p04() { printf("-p]"); }
void p05() { printf("-p]"); }
void p06() { printf("-p]"); }
void p07() { printf("-p]"); }
void p08() { printf("-p]"); }
void p09() { printf("-p]"); }
void p10() { printf("-p]"); }
void p11() { printf("-p]"); }
void p12() { printf("-p]"); }
void p13() { printf("-p]"); }
void p14() { printf("-p]"); }
void p15() { printf("-p]"); }
void p16() { printf("-p]"); }
void p17() { printf("-p]"); }
void p18() { printf("-p]"); }
void p19() { printf("-p]"); }
void p20() { printf("-p]"); }
void p21() { printf("-p]"); }
void p22() { printf("-p]"); }
void p23() { printf("-p]"); }
void p24() { printf("-p]"); }
void p25() { printf("-p]"); }
void p26() { printf("-p]"); }
void p27() { printf("-p]"); }
void p28() { printf("-p]"); }
void p29() { printf("-p]"); }
void p30() { printf("-p]"); }
void p31() { printf("-p]"); }

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
    int addr = (wc>>2);
    if (wc & 0x01) { printf("-call-%d]",addr); }
    else { printf("-jmp-%d]",addr); }
}
void Exec(int start) {
    pc = start;
    while (btwi(pc,0,WC_SZ)) {
        wc = code[pc++];
        printf("[wc-%d", wc);
        if (wc<32) { q[wc](); }
        else { wordCode(wc); }
    }
}

int parseWord(char *w) {
    if (!w) { w = &wd[0]; }
    DE_T *de =  findWord(w);
    if (de) {
        printf("-%s:%d-", de->nm, de->xt);
        return 1;
    }
    if (strEq(w,"bye")) { exit(0); }
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
    if (l==last) {
        here=h;
        Exec(h);
    }
}
void Init(FILE *fp) {
    int t;
    sp = rsp = here = state = 0;
    base = 10;
    last = CODE_SZ;
    for (t=0; t<WC_SZ; t++) { code[t]=0; }
    if (fp) {
        while ((t=fgetc(fp))!=EOF) { if (31<t) { code[here++]=t-32; } }
        fclose(fp); Exec(0);
    }
    addWord("test");
    addWord("test2");
    findWord("test");
    findWord("no");
}
int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc>1) { fp=fopen(argv[1], "rb"); }
    Init(fp);
    while (1) { REP(); }; // REPL
    return 0;
}
