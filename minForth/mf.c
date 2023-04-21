#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define CODESZ      100000
#define VARSSZ      1000
#define DICTSZ      1000

#define NCASE       goto next; case
#define NEXT        goto next
#define PS(x)       stk[++sp]=(x)
#define POP         stk[sp--]
#define S0          stk[sp]
#define S1          stk[sp-1]
#define S2          stk[sp-2]
#define RPS(x)      rstk[++rsp]=(long)(x)
#define RPOP        rstk[rsp--]
#define R0          rstk[rsp]
#define R1          rstk[rsp-1]
#define R2          rstk[rsp-2]
#define D1          sp--
#define D2          sp-=2
#define L0          lstk[lsp]
#define L1          lstk[lsp-1]
#define L2          lstk[lsp-2]
#define BTW(a,b,c)  ((b<=a)&&(a<=c))
#define HERE        vars.l[0]
#define LAST        vars.l[1]

#define WORDSZ      sizeof(long)
#define OPIR(x)     x.ir[3]
#define OPARG(x)    (x.wd&0x00ffffff)
#define u           OPIR(code[pc-1])

#define IMMED       0x01
#define INLINE      0x02
#define LIT         '1'

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef struct {
    ulong addr;
    byte flgs;
    byte len;
    char name[16];
} dict_t;

unsigned long ir;
long stk[32], rstk[32], lstk[30], sp, rsp, lsp, t, a;
union { byte b[VARSSZ*sizeof(long)]; long l[VARSSZ]; } vars;
char *toIN, wd[32];
char code[CODESZ];
dict_t dict[DICTSZ];

void run(long pc) {
    next:

    // printf("-pc/ir:%ld/%c/%d-",pc,code[pc],code[pc]);
    switch(code[pc++]) {
        case 0: return;
        case LIT: PS(*(long*)(&code[pc])); pc += sizeof(long);
        NCASE ' ':
        NCASE '!': vars.l[a] = POP;
        NCASE '@': PS(vars.l[a]);
        NCASE '#': t=S0; PS(t);
        NCASE '$': t=S0; S0=S1; S1=t;
        NCASE '%': t=S1; PS(t);
        NCASE '\'': D1;
        NCASE '*': S1*=S0; D1;
        NCASE '+': S1+=S0; D1;
        NCASE '-': S1-=S0; D1;
        NCASE '.': printf(" %ld",POP);
        NCASE '/': S1 /= ((S0)?S0:1); D1;
        NCASE ':': RPS(pc+sizeof(long)); pc = *(long*)&code[pc];
        NCASE ';': pc = (0<rsp) ? RPOP : 0;
        NCASE '<': S1=(S1<S0)?-1:0;  D1;
        NCASE '=': S1=(S1==S0)?-1:0; D1;
        NCASE '>': S1=(S1>S0)?-1:0;  D1;
        NCASE 'a': t=code[pc++];
            if (t=='a') { PS(a); }                          // a
            else if (t=='>') { a = POP; }                   // >a
            else if (t=='@') { PS(vars.b[a]); }             // a@
            else if (t=='!') { vars.b[a] = (byte)POP; }     // a!
            else if (t=='1') { PS(vars.b[a++]); }           // a@+
            else if (t=='2') { vars.b[a++] = (byte)POP; }   // a!+
        NCASE 'I': PS(L0);
        NCASE 'N': printf("\n");
        NCASE 'L': t=code[pc++];
            if (t=='@') { S0 = *(long*)S0; }                  // l@
            else if (t=='!') { *(long*)S1 = S0; } D2;         // l!
        NCASE 'l': t=code[pc++];
            if (t=='@') { S0 = vars.l[S0]; }                  // l@
            else if (t=='!') { vars.l[S0] = S1; } D2;         // l!
        NCASE 'm': t=code[pc++];
            if (t=='@') { S0 = *(byte*)S0; }                  // m@
            else if (t=='!') { *(byte*)S1 = (byte)S0; } D2;   // m!
        NCASE 'Q': exit(0);
        NCASE 'T': PS(clock());
        NCASE '[': lsp+=3; L0 = POP; L1 = POP; L2 = pc;
        NCASE ']': if (++L0 < L1) { pc = L2; } else { lsp -= 3; }
        NCASE '\\': if (0 < sp) sp--;
        NCASE 'd': --S0;
        NCASE 'e': printf("%c",(char)POP);
        NCASE 'i': ++S0;
        NCASE '{': lsp += 3; L2 = pc;
        NCASE '}': if (S0) { pc = L2; } else { lsp -= 3; }
        default: return;
    }
}

void cComma(char v) { code[HERE++] = v; }
void comma(long v) { *(long*)&code[HERE] = v; HERE += sizeof(long); }

int doWord() {
    int l=0;
    while (BTW(*toIN,1,32)) { ++toIN; }
    while (32 < *toIN) { wd[l++] = *(toIN++); }
    wd[l] = 0;
    return l;
}

int doNum() {
    if (BTW(*toIN,'0','9')) {
        long num = 0;
        while BTW(*toIN,'0','9') { num = (num*10) + *(toIN++) - '0'; }
        cComma(LIT); comma(num);
        return 1;
    }
    return 0;
}

int doCreate(byte flgs) {
    int l = doWord();
    // printf("-cr:%s-",wd);
    if (l==0) { return 0; }
    dict_t *dp = &dict[LAST++];
    dp->addr = HERE;
    dp->flgs = flgs;
    if (15<l) { l = 15; }
    dp->len = l;
    for (int i=0; i<=l; i++) { dp->name[i] = wd[i]; }
    return 1;
}

int strEq(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 == *s2) { ++s1; ++s2; }
        else { return 0; }
    }
    return (*s1 == *s2) ? 1 : 0;
}

int doFind(char *nm) {
    int l = 0;
    while (nm[l]) { ++l; }
    for (int i = LAST-1; 0 <= i; i--) {
        dict_t *dp = &dict[i];
        if ((dp->len == l) && strEq(nm, dp->name)) { return i; }
    }
    return -1;
}

int doDict() {
    int l = doWord();
    if (l==0) { return 0; }
    if (strEq(wd, ":")) { return doCreate(0); }
    if (strEq(wd,":i")) { return doCreate(INLINE); }
    if (strEq(wd,":I")) { return doCreate(IMMED); }
    int f = doFind(wd);
    if (f < 0) {
        for (int i = 0; i < l; i++) { cComma(wd[i]); }
        return 1;
    }
    dict_t *dp = &dict[f];
    if (dp->flgs & IMMED) {
        run(dp->addr);
    } else if (dict[f].flgs & INLINE) {;
        char *x = &code[dp->addr];
        cComma(*(x++));
        while (*x != ';') { cComma(*(x++)); }
    } else {
        cComma(':'); comma((long)dp->addr);
    }
    return 1;
}

long doParse(const char *src) {
    toIN = (char*)src;
    long st = HERE;
    while (*toIN) {
        while (BTW(*toIN,1,32)) { ++toIN; continue; }
        if (doNum()) { continue; }
        if (doDict()) { continue; }
    }
    cComma(0);
    return st;
}

int main() {
    sp = rsp = lsp = 0;
    HERE = 0;
    LAST = 0;
    doParse(":i H 0 ;     :i L 1 ;  :i here H l@; :i last L l@ ;");
    doParse(":i dup # ;   :i swap $ ; :i drop \\ ;");
    doParse(":i a aa ;    :i >a a> ;  :i a@+ a1 ; :i a!+ a2 ;");
    doParse(":i begin { ; :i while } ;");
    doParse(":i 1+  i ;   :i 1- d ;");
    doParse(":i do    [ ; :i loop  ] ;");
    doParse(":I if here ; :I then last swap l! ;");
    doParse(":  timer T ; : elapsed timer swap - . N ;");
    doParse(":  mil 1000 dup * * ; : #. dup . ;");
    run(doParse("timer 500 mil #. 0 do loop elapsed"));
    run(doParse("timer 200 mil #. begin 1- while drop elapsed"));
    run(doParse("here . last . N"));
}
