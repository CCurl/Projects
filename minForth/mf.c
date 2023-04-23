#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MEM_SZ      1000
#define VARSSZ      1000
#define DICTSZ      1000
#define STK_MASK    0x0f

#define NCASE       goto next; case
#define PUSH(x)     push((long)x)
#define POP         pop()
#define RPUSH(x)    rpush((long)x)
#define RPOP        rpop()
#define S0          stk[sp]
#define S1          stk[(sp-1)&STK_MASK]
#define D1          sp=((sp-1)&STK_MASK)
#define D2          sp=((sp-2)&STK_MASK)
#define R0          rstk[rsp]
#define L0          lstk[lsp]
#define L1          lstk[lsp-1]
#define L2          lstk[lsp-2]
#define BTW(a,b,c)  ((b<=a)&&(a<=c))
#define MEMB(x)     mem.b[(x)]
#define MEML(x)     mem.l[(x)]
#define HERE        MEML(0)
#define LAST        MEML(1)

#define WORDSZ      sizeof(long)

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

long stk[STK_MASK+1], rstk[STK_MASK+1], lstk[30], sp, rsp, lsp, t, a;
union { byte b[MEM_SZ*sizeof(long)]; long l[MEM_SZ]; } mem;
char *toIN, wd[32];
dict_t dict[DICTSZ];

void push(long x) { sp = ((sp+1) & STK_MASK); stk[sp] = x; }
long pop() { long x = stk[sp]; D1; return x; }

void rpush(long x) { rsp = ((rsp+1) & STK_MASK); rstk[rsp] = x; }
long rpop() { long x = rstk[rsp]; (rsp = (rsp-1) & STK_MASK); return x; }

void run(long start) {
    byte *pc = &MEMB(start);
    next:

    // printf("-pc/ir:%ld/%c/%d-",pc,*(pc),*(pc));
    switch(*(pc++)) {
        NCASE ' ': // NOP
        NCASE LIT: PUSH(*(long*)pc); pc += sizeof(long);
        NCASE '!': MEML(a) = POP;
        NCASE '@': PUSH(MEML(a));
        NCASE '#': t=S0; PUSH(t);
        NCASE '$': t=S0; S0=S1; S1=t;
        NCASE '%': t=S1; PUSH(t);
        NCASE '\'': D1;
        NCASE '*': S1*=S0; D1;
        NCASE '+': S1+=S0; D1;
        NCASE '-': S1-=S0; D1;
        NCASE '.': printf(" %ld",POP);
        NCASE '/': S1 /= ((S0)?S0:1); D1;
        NCASE ':': if (*(pc+sizeof(long)) != ';') { RPUSH(pc+sizeof(long)); }
        case  'J': pc = &MEMB(*(long*)pc); 
        NCASE 'Z': if (S0 == 0) { pc = &MEMB(*(long*)pc); } else { pc+=sizeof(long); }
        NCASE 'z': if (S0 != 0) { pc = &MEMB(*(long*)pc); } else { pc+=sizeof(long); }
        NCASE ';': if (0 < rsp) { pc = (byte*)RPOP; } else { return; }
        NCASE '<': S1=(S1<S0)?-1:0;  D1;
        NCASE '=': S1=(S1==S0)?-1:0; D1;
        NCASE '>': S1=(S1>S0)?-1:0;  D1;
        NCASE 'a': t=*(pc++);
            if (t=='a') { PUSH(a); }                        // a
        else if (t=='>') { a = POP; }                       // >a
            else if (t=='@') { PUSH(MEMB(a)); }             // a@
            else if (t=='!') { MEMB(a) = (byte)POP; }       // a!
            else if (t=='1') { PUSH(MEMB(a++)); }           // a@+
            else if (t=='2') { MEMB(a++) = (byte)POP; }     // a!+
        NCASE 'I': PUSH(L0);
        NCASE 'N': printf("\n");
        NCASE 'l': t=*(pc++);
            if (t=='@') { S0 = MEML(S0); }
            else if (t=='!') { MEML(S0) = S1; D2; }
        NCASE 'm': t=*(pc++);
            if (t=='@') { S0 = *(byte*)S0; }
            else if (t=='!') { *(byte*)S1 = (byte)S0; D2; }
        NCASE 'Q': exit(0);
        NCASE 'T': PUSH(clock());
        NCASE '[': lsp+=3; L0 = POP; L1 = POP; L2 = (long)pc;
        NCASE ']': if (++L0 < L1) { pc = (byte*)L2; } else { lsp -= 3; }
        NCASE '\\': if (0 < sp) sp--;
        NCASE 'd': --S0;
        NCASE 'e': printf("%c",(char)POP);
        NCASE 'i': ++S0;
        NCASE '{': lsp += 3; L2 = (long)pc;
        NCASE '}': if (S0) { pc = (byte*)L2; } else { lsp -= 3; }
        NCASE 0: return;
        default: printf("-ir:%d-",*(pc-1)); return;
    }
}

void cComma(char v) { MEMB(HERE++) = v; }
void comma(long v) { *(long*)&MEMB(HERE) = v; HERE += sizeof(long); }

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
    // printf("-cr:%s-\n",wd);
    if (l==0) { return 0; }
    dict_t *dp = &dict[LAST++];
    dp->addr = HERE;
    dp->flgs = flgs;
    if (15<l) { l = 15; }
    dp->len = l;
    for (int i=0; i<=l; i++) { dp->name[i] = wd[i]; }
    return 1;
}

int strLen(const char *s1) {
    int l = 0;
    while (*(s1++)) { ++l; }
    return l;
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
    // printf("-dd:%s-\n",wd);
    if (l==0) { return 0; }
    if (strEq(wd, ":")) { return doCreate(0); }
    if (strEq(wd,":i")) { return doCreate(INLINE); }
    if (strEq(wd,":M")) { return doCreate(IMMED); }
    int f = doFind(wd);
    if (f < 0) {
        for (int i = 0; i < l; i++) { cComma(wd[i]); }
        return 1;
    }
    dict_t *dp = &dict[f];
    if (dp->flgs & IMMED) {
        run(dp->addr);
    } else if (dict[f].flgs & INLINE) {;
        long x = dp->addr;
        cComma(MEMB(x++));
        while (MEMB(x) != ';') { cComma(MEMB(x++)); }
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
    MEMB(HERE) = 0;
    return st;
}

int repl() {
    char tib[128];
    printf(" ok\n");
    if (tib != fgets(tib, sizeof(tib), stdin)) { return 0; }
    int l = strLen(tib);
    while ((0 < l) && (tib[l-1] < 33)) { tib[--l] = 0; }
    long h = doParse(tib);
    if ((tib[0] == ':') && (tib[l-1] == ';')) { return 1; }
    run(h); HERE = h;
    return 1;
}

int main() {
    sp = rsp = lsp = 0;
    HERE = 32;
    LAST = 0;
    doParse(":i H 0     ;   :i L 1 ;  :i here H l@; :i last L l@ ;");
    doParse(":i dup #   ;   :i swap $ ; :i drop \\ ;");
    doParse(":i a aa    ;   :i >a a> ;  :i a@+ a1 ; :i a!+ a2 ;");
    doParse(":i begin { ;   :i while } ; : bye Q ;");
    doParse(":i 1+  i   ;   :i 1- d ;");
    doParse(":i do  [   ;   :i loop  ] ;");
    doParse(":M if here ;   :M then last swap l! ;");
    doParse(": timer T  ;   : elapsed timer swap - . N ;");
    doParse(": mil 1000 dup * * ; : #. dup . ;");
    doParse(": bm1 mil #. timer swap 0 do loop elapsed ;");
    doParse(": bm2 mil #. timer swap begin 1- while drop elapsed ;");
    run(doParse("here . last . N"));
    while (repl()) { }
    printf(" bye\n");
}
