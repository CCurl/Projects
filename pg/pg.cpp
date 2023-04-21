#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define CODESZ      1000
#define DICTSZ      1000

#define NCASE       goto next; case
#define NEXT        goto next
#define PS(x)       stk[++sp]=(x)
#define POP         stk[sp--]
#define S0          stk[sp]
#define S1          stk[sp-1]
#define S2          stk[sp-2]
#define RPS(x)      rstk[++rsp]=(x)
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
#define LIT          1

typedef unsigned char byte;
typedef unsigned long ulong;

typedef union {
    ulong wd;
    byte ir[WORDSZ];
} op_t;

typedef struct {
    ulong addr;
    byte attr[2];
    byte flgs;
    byte len;
    char name[16];
} dict_t;

unsigned long ir;
long stk[32], rstk[32], lstk[30], sp, rsp, lsp, t;
union { byte b[10000*sizeof(long)]; long l[10000]; } vars;
char *toIN, wd[32];
op_t code[CODESZ];
dict_t dict[DICTSZ];

void run(long pc) {
    next:

    switch(OPIR(code[pc++])) {
        NCASE LIT: PS(OPARG(code[pc-1]));
        NCASE ' ':
        NCASE '!': vars.l[S0] = S1; D2;
        NCASE '"': while (0) {}
        NCASE '#': t=S0; PS(t);
        NCASE '$': t=S0; S0=S1; S1=t;
        NCASE '%': t=S1; PS(t);
        NCASE '&': printf("%c",u);
        NCASE '\'': D1;
        NCASE '(': printf("%c",u);
        NCASE ')': printf("%c",u);
        NCASE '*': S1*=S0; D1;
        NCASE '+': S1+=S0; D1;
        NCASE ',': printf("%c",u);
        NCASE '-': S1-=S0; D1;
        NCASE '.': printf(" %ld",POP);
        NCASE '/': S1 /= ((S0)?S0:1); D1;
        NCASE '0': case '1': case '2': case '3': 
        NCASE '4': case '5': case '6': case '7': 
        NCASE '8': case '9': printf("%c",u);
        NCASE ':': RPS(pc); pc = OPARG(code[pc-1]);
        NCASE ';': pc = (0<rsp) ? RPOP : 0;
        NCASE '<': S1=(S1<S0)?-1:0;  D1;
        NCASE '=': S1=(S1==S0)?-1:0; D1;
        NCASE '>': S1=(S1>S0)?-1:0;  D1;
        NCASE '?': printf("%c",u);
        NCASE '@': S0 = vars.l[S0];
        NCASE 'a': PS(a);
        NCASE 'A': printf("%c",u);
        NCASE 'B': printf("%c",u);
        NCASE 'C': S0 = vars.b[S0];
        NCASE 'D': printf("%c",u);
        NCASE 'E': printf("%c",u);
        NCASE 'F': printf("%c",u);
        NCASE 'G': printf("%c",u);
        NCASE 'H': printf("%c",u);
        NCASE 'I': PS(L0);
        NCASE 'J': printf("%c",u);
        NCASE 'K': printf("%c",u);
        NCASE 'L': printf("%c",u);
        NCASE 'M': printf("%c",u);
        NCASE 'N': printf("\n");
        NCASE 'O': printf("%c",u);
        NCASE 'P': printf("%c",u);
        NCASE 'Q': exit(0);
        NCASE 'R': printf("%c",u);
        NCASE 'S': printf("%c",u);
        NCASE 'T': PS(clock());
        NCASE 'U': printf("%c",u);
        NCASE 'V': printf("%c",u);
        NCASE 'W': printf("%c",u);
        NCASE 'X': printf("%c",u);
        NCASE 'Y': printf("%c",u);
        NCASE 'Z': printf("%c",u);
        NCASE '[': lsp+=3; L0=POP; L1=POP; L2=pc;
        NCASE '\\': if (0<sp) sp--;
        NCASE ']': if (++L0<L1) { pc=L2; } else { lsp-=3; }
        NCASE '^': printf("%c",u);
        NCASE '_': printf("%c",u);
        NCASE '`': printf("%c",u);
        NCASE 'a': printf("%c",u);
        NCASE 'b': printf("%c",u);
        NCASE 'c': S0 = vars.b[S0];
        NCASE 'd': --S0;
        NCASE 'e': printf("%c",(char)POP);
        NCASE 'f': printf("%c",u);
        NCASE 'g': printf("%c",u);
        NCASE 'h': printf("%c",u);
        NCASE 'i': ++S0;
        NCASE 'j': printf("%c",u); 
        NCASE 'k': printf("%c",u);
        NCASE 'l': printf("%c",u);
        NCASE 'm': printf("%c",u);
        NCASE 'n': printf("%c",u);
        NCASE 'o': printf("%c",u);
        NCASE 'p': printf("%c",u);
        NCASE 'q': printf("%c",u);
        NCASE 'r': printf("%c",u);
        NCASE 's': printf("%c",u);
        NCASE 't': printf("%c",u);
        NCASE 'u': printf("%c",u);
        NCASE 'v': printf("%c",u);
        NCASE 'w': printf("%c",u);
        NCASE 'x': printf("%c",u);
        NCASE 'y': printf("%c",u);
        NCASE 'z': printf("%c",u);
        NCASE '{': lsp+=3; L2=(long)pc;
        NCASE '|': printf("%c",u);
        NCASE '}': if (POP) { pc=L2; } else { lsp -=3; }
        NCASE '~': printf("%c",u); NEXT;
        NCASE 0:
        default: return;
    }
}

long compile(byte op, ulong arg) {
    code[HERE].wd = arg;
    OPIR(code[HERE++]) = op;
    return HERE-1;
}

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
        compile(LIT, num);
        return 1;
    }
    return 0;
}

int doCreate(byte flgs) {
    int l = doWord();
    if (l==0) { return 0; }
    dict_t *dp = &dict[LAST++];
    dp->addr = HERE;
    dp->flgs = flgs;
    dp->attr[0] = dp->attr[1] = 0;
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
        for (int i = 0; i < l; i++) { compile(wd[i], 0); }
        return 1;
    }
    if (dict[f].flgs & IMMED) {
        run(dict[f].addr);
    } else if (dict[f].flgs & INLINE) {;
        long x = dict[f].addr;
        compile(OPIR(code[x]), OPARG(code[x]));
        while (OPIR(code[++x]) != ';') { compile(OPIR(code[x]), OPARG(code[x])); }
    } else {
        compile(':', dict[f].addr);
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
    compile(0,0);
    return st;
}

void defNum(const char *nm, long val) {
    toIN = (char*) nm;
    doCreate(INLINE);
    compile(LIT, val);
    compile(';', 0);
    printf("-%s: %08lx-", nm, val);
}

int main() {
    sp = rsp = lsp = 0;
    HERE = 0;
    LAST = 0;
    doParse(": here 0 @ ; : last 1 @ ;");
    doParse(":I if here ; :I then last $! ;");
    doParse(":i begin { ; :i while } ;");
    doParse(":i do    [ ; :i loop  ] ;");
    doParse(" : timer T ;  : elapsed timer $ - . ;");
    run(doParse("timer 500 1000 # * * # . 0 do loop elapsed N"));
    run(doParse("T 200 1000 #**#.{ d # }\\T$-.N"));
    run(doParse("here . last . N"));
}
