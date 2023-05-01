#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MEM_SZ      1024*1024
#define STK_MASK    0x1f

#define BCASE       break; case
#define NCASE       goto next; case
#define RPUSH(x)    rpush((long)x)
#define RPOP        rpop()
#define S0          stk[sp]
#define S1          stk[sp-1]
#define MEMB(x)     mem.b[(x)]
#define MEML(x)     mem.l[(x)]
#define CELL_SZ     sizeof(long)

enum {
    JUMP=0, RET, JMPT0, JMPC0, CALL, ACSTOREINC, ACATINC, SYS,
    U8, AATINC, LIT, AAT, LIT1, ASTOREINC, UE, ASTORE,
    COM, TIMES2, DIV2, ADDMULT, XOR, AND, U16, ADD,
    POPR, AVALUE, DUP, OVER, PUSHR, TOA, NOP, DROP,
    EMIT=101, FOPEN, FCLOSE, CCOMMA, COMMA, CREATE, FIND,
    DOT, HA, LA, STA, CSZ
};

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef struct { long next, xt; byte f, l; char name[32]; } de_t;

long stk[STK_MASK+1], rstk[STK_MASK+1];
long sp, rsp, t, a, cf, st, input_fp;
char tib[128], wd[32], *in = &tib[0];
union { byte b[MEM_SZ*CELL_SZ]; long l[MEM_SZ]; } mem;
byte *H, *L;

void push(long x) { stk[++sp] = x; }
long pop() { return stk[sp--]; }

void rpush(long x) { rsp = ((rsp+1) & STK_MASK); rstk[rsp] = x; }
long rpop() { long x = rstk[rsp]; (rsp = (rsp-1) & STK_MASK); return x; }

#ifndef NEEDS_ALIGN

long GetNumAt(byte *a) { return *(long*)a; }
void SetNumAt(byte *a, long val) { *(long*)a = val; } 

#else
long GetNumAt(byte *a) {
    return *a | *(a+1)<<8 | *(a+2)<<16 | *(a+2)<<24;
}

void SetNumAt(byte *a, long val) {
    *a = (val & 0xFF);
    *(a+1) = (val>>8 & 0xFF);
    *(a+2) = (val>>16 & 0xFF);
    *(a+2) = (val>>24 & 0xFF);
}
#endif

void CComma(long x) { *(H++) = (byte)x; }
void Comma(long x) { SetNumAt(H, x); H += CELL_SZ; }

int nextWord() {
    int l = 0;
    while (*in && (*in<33)) { ++in; }
    while (*in && (*in>32)) { wd[l++] = *(in++); }
    wd[l] = 0; return l;
}

int strLen(char *s1) {
    int l = 0;
    while (*s1) { ++l; ++s1; }
    return l;
}

int strEq(char *s1, char *s2) {
    while (*s1 || *s2) { if (*(s1++) != *(s2++)) { return 0; } }
    return (*s1 == *s2) ? 1 : 0;
}

void strCpy(char *s1, char *s2) {
    while (*s2) { *(s1++) = *(s2++); }
    (*s1) = 0;
}

void create(char *name) {
    if (name == 0) { name = wd; nextWord(); }
    // printf("-cr:%s-", name);
    int l = strLen(name);
    de_t *dp = (de_t*)L-(CELL_SZ*2+3+l);
    dp->next = (long)L;
    dp->xt = (long)H;
    dp->f = 0;
    dp->l = l;
    strCpy(dp->name, name);
    // printf("-nm:%s,%p-", dp->name, dp);
    L = (byte*)dp;
}

de_t *find(char *name) {
    if (name == 0) { name = wd; nextWord(); }
    de_t *dp = (de_t*)L;
    // printf("-find:%s(%p)-\n", name, dp);
    while (dp < (de_t*)&MEMB(MEM_SZ*CELL_SZ)) {
        // printf("-f?:%s-\n", dp->name);
        if (strEq(dp->name, name)) { return dp; }
        dp = (de_t*)dp->next;
    }
    return 0;
}

void sysOP(long op) {
    long x1, x2;
    switch (op)
    {
        case  EMIT:   putchar((int)pop());
        BCASE FOPEN:  x2=pop(); x1=pop(); push((long)fopen((char*)x1, (char*)x2));
        BCASE FCLOSE: x1=pop(); fclose((FILE*)x1);
        BCASE CCOMMA: x1=pop(); CComma(x1);
        BCASE COMMA:  x1=pop(); Comma(x1);
        BCASE CREATE: create(0);
        BCASE FIND:   find(0);
        BCASE DOT:    printf("%ld ", pop());
        BCASE HA:     push((long)&H);
        BCASE LA:     push((long)&L);
        BCASE STA:    push((long)&st);
        BCASE CSZ:    push(CELL_SZ);
        default: printf("-sysOP:%ld?-", op);
    }
}

void run(byte *pc) {
    next:
    // printf("-pc/ir:%p/%d-\n",pc,*(pc));
    switch(*(pc++)) {
        // 00-07: Flow control
        case  JUMP: pc = (byte*)GetNumAt(pc); 
        NCASE RET: if (0 < rsp) { pc = (byte*)RPOP; } else { return; }
        NCASE JMPT0: if (S0 == 0) { pc = (byte*)GetNumAt(pc); } else { pc+=CELL_SZ; }
        NCASE JMPC0: if (cf != 0) { pc = (byte*)GetNumAt(pc); } else { pc+=CELL_SZ; }
        NCASE CALL: RPUSH(pc+CELL_SZ); pc = (byte*)GetNumAt(pc);
        NCASE ACSTOREINC: MEMB(a++) = (byte)pop();  // NON-standard 
        NCASE ACATINC: push(MEMB(a++));             // NON-standard 
        NCASE SYS: sysOP(pop());                    // NON-standard 
        // 08-15: Memory access
        NCASE U8: // Unused
        NCASE AATINC: push(MEML(a++));
        NCASE LIT: push(GetNumAt(pc)); pc += CELL_SZ;
        NCASE AAT: push(MEML(a));
        NCASE LIT1: push(*(pc++));                  // NON-standard 
        NCASE ASTOREINC: MEML(a++) = pop();
        NCASE UE: // Unused
        NCASE ASTORE: MEML(a) = pop();
        // 16-23: ALU instructions
        NCASE COM: S0 = ~S0;
        NCASE TIMES2: S0 *= 2;
        NCASE DIV2:   S0 /= 2;
        NCASE ADDMULT: if (S0 & 0x01) { S0 += S1; }
        NCASE XOR: t=pop(); S0 ^= t;
        NCASE AND: t=pop(); S0 &= t;
        NCASE U16: // Unused
        NCASE ADD: t=pop(); S0 += t;
        // 24-31: Register instructions
        NCASE POPR: push(rpop());
        NCASE AVALUE: push(a);
        NCASE DUP: t=S0; push(t);
        NCASE OVER: t=S1; push(t);
        NCASE PUSHR: rpush(pop());
        NCASE TOA: a = pop();
        NCASE NOP: // NOP
        NCASE DROP: sp = (0<sp) ? sp-1: 0;
        default: printf("-ir:%u?-",*(pc-1)); return;
    }
}

#define BTW(n,l,h) ((l<=n)&(n<=h))
int parseNum(char *cp) {
    long x = 0;
    while (BTW(*cp,'0','9')) {
        x = (x*10) + (*(cp++)-'0');
    }
    if (*cp) return 0;
    push(x);
    return 1;
}

int isNum(char *cp) {
    // printf("-num:%s?-\n",cp);
    if (parseNum(cp)) {
        CComma(LIT); Comma(pop());
        return 1;
    }
    return 0;
}

int isDW(char *cp) {
    if (!strEq(cp, "-d-")) { return 0; }
    create(0);
    while (nextWord()) {
        if (strEq(wd,"-e-")) { break; }
        if (parseNum(wd)) { CComma(pop()); }
        else { printf("-dw:%s?-", wd); }
    }
    // printf("-end-");
    return 1;
}

int isWord(char *cp) {
    //printf("-word:%s?-\n",cp);
    de_t *dp = find(cp);
    if (!dp) { return 0; }
    CComma(CALL); Comma(dp->xt);
    return 1;
}

int parse(const char *cp) {
    in = (char*)cp;
    while (nextWord()) {
        //printf("-re:%s-",wd);
        if (isDW(wd)) { continue; }
        if (isNum(wd)) { continue; }
        if (isWord(wd)) { continue; }
        printf("-%s?-",wd);
        return 0;
    }
    return 1;
}

void repl(char *cp) {
    byte *cH = H, *cL = L;
    in = cp;
    if (in == 0) {
        printf(" ok\n");
        in = fgets(tib, sizeof(tib), stdin);
        if (!in) { st = 999; return; }
    }
    if (parse(in) == 0) { H = cH; return; }
    if (cL == L) { CComma(RET); H=cH; run(H); }
}

int main(int argc, char **argv) {
    sp = rsp = input_fp = 0;
    H = &MEMB(0);
    L = &MEMB(MEM_SZ*CELL_SZ);
    if (2 <= argc) {
        input_fp = (long)fopen(argv[1],"rb");
        if (input_fp) { printf("Cannot open: %s\n", argv[1]); }
    }
    repl("-d- immediate 12 2 12 110 7 12 2 12 112 7 23 29 1 -e-");
    repl("-d- : 12 106 7 1 -e- immediate");
    repl("-d- ; 12 1 12 104 7 1 -e-");
    repl("-d- . 12 108 7 1 -e-");
    repl("-d- emit 12 101 7 1 -e-");
    repl("-d- sys 7 1 -e-");
    repl("123 . 65 emit");
    while (st != 999) { repl(0); }
    return 0;
}
