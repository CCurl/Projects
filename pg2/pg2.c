#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define NEXT   goto stkNext
#define NCASE  NEXT; case
#define NEXT2  goto regNext
#define RCASE NEXT2; case
#define BCASE  break; case
#define RETCASE  return; case
#define PS(x)  stk[++sp]=(cell_t)(x)
#define PP     stk[sp--]
#define PP32   (int32_t)stk[sp--]
#define S0     stk[sp]
#define S1     stk[sp-1]
#define S2     stk[sp-2]
#define RPS(x) rstk[++rsp]=(x)
#define RPP    rstk[rsp--]
#define R0     rstk[rsp]
#define R1     rstk[rsp-1]
#define R2     rstk[rsp-2]
#define D1     sp--
#define D2     sp-=2
#define L0     lstk[lsp]
#define L1     lstk[lsp-1]
#define L2     lstk[lsp-2]
#define BTW(a,b,c) ((b<=a)&&(a<=c))

#define CELL_SZ sizeof(cell_t)

typedef int64_t cell_t;
typedef union { char c[8]; short in[4]; int32_t i32[2]; int64_t i64; double d; } op_t;

op_t code[256], *op;
cell_t stk[32], rstk[32], lstk[30], t;
cell_t reg[26], loc[10];
int sp, rsp, lsp, locBase, here, u, pc;

#define OPC(x)   op->c[x]
#define OP32(x)  op->i32[x]
#define OP64     op->i64

void dumpOp(int a, op_t *op) {
    printf("\npc: %-3d - ", a);
    printf("%2x/%c", OPC(0), BTW(OPC(0),32,126) ? (char)OPC(0) : '.');
    for (int i=1; i<CELL_SZ; i++) { printf(",%2x", OPC(i)&0xff); }
    printf(" - i32: %8lx,%8lx", (long) OP32(0), (long)OP32(1));
    printf(" - i64: %jx / %jd", OP64, OP64);
}

void runReg(int start) {
    pc = start;
    regNext:
    op = &code[pc++];
    // dumpOp(pc, op);
    switch (OPC(0)) {
        case  0: return;
        case  '+': reg[OPC(3)] = reg[OPC(1)] + reg[OPC(2)];
        RCASE '*': reg[OPC(3)] = reg[OPC(1)] * reg[OPC(2)];
        RCASE '-': reg[OPC(3)] = reg[OPC(1)] - reg[OPC(2)];
        RCASE '/': reg[OPC(3)] = reg[OPC(1)] / reg[OPC(2)];
        RCASE '.': printf(" %jd", reg[OPC(1)]);
        RCASE ',': printf("%c",  (char)reg[OPC(1)]);
        RCASE '[': lsp+=3; L0=PP; L1=PP; L2=pc;
        RCASE ']': if (++L0<L1) { pc=(int)L2; } else { lsp-=3; }
        RCASE 't': t=clock(); if (BTW(OPC(1),0,25)) { reg[OPC(1)]=t; } else { PS(t); }
        RCASE 'l': PS(loc[locBase+OPC(1)]);
        RCASE 'L': loc[locBase+OPC(1)] = PP;
        RCASE 'm': reg[OPC(1)] = OP32(1);
        RCASE 'M': loc[locBase+OPC(1)] = OP32(1);
        RCASE 'n': PS(OP32(1));
        RCASE 'r': PS(reg[OPC(1)]);
        RCASE 's': reg[OPC(1)] = PP;
        RCASE 'v': reg[OPC(2)] = reg[OPC(1)];
        NEXT2; default: dumpOp(pc-1, op); NEXT2;
    }
}

void runStack(const char *x) {
    pc = 0;
    stkNext:
    u = x[pc++];

    switch(u) {
        case  0: return;
        case  ' ':
        NCASE '!': printf("%c",u);
        NCASE '"': printf("%c",u);
        NCASE '#': t=S0; PS(t);
        NCASE '$': t=S0; S0=S1; S1=t;
        NCASE '%': t=S1; PS(t);
        NCASE '&': printf("%c",u);
        NCASE '\'': D1;
        NCASE '(': if (PP==0) { while (x[pc++]!=')') {} }
        NCASE ')': printf("%c",u);
        NCASE '*': S1*=S0; D1;
        NCASE '+': S1+=S0; D1;
        NCASE ',': printf("%c",u);
        NCASE '-': S1-=S0; D1;
        NCASE '.': printf(" %jd",PP);
        NCASE '/': S1/=S0; D1;
        NCASE '0': case '1': case '2': case '3': case  '4': case '5': case '6': case '7':
        case  '8': case '9': PS(u-'0'); while (BTW(x[pc],'0','9')) { S0=(S0*10)+(x[pc++]-'0'); }
        NCASE '<': S1=(S1<S0)?-1:0;  D1;
        NCASE '=': S1=(S1==S0)?-1:0; D1;
        NCASE '>': S1=(S1>S0)?-1:0;  D1;
        NCASE 'I': PS(L0);
        NCASE 'N': printf("\n");
        NCASE 'Q': exit(0);
        NCASE 'T': PS(clock());
        NCASE '[': lsp+=3; L0=PP; L1=PP; L2=(cell_t)pc;
        NCASE '\\': if (0<sp) sp--;
        NCASE ']': if (++L0<L1) { pc=(int)L2; } else { lsp-=3; }
        NCASE 'd': --S0;
        NCASE 'i': ++S0;
        NCASE 'l': u=x[pc++]; PS(&loc[u-'0']);
        NCASE 'r': u=x[pc++]; PS(reg[u-'A']);
        NCASE 's': u=x[pc++]; reg[u-'A']=PP;
        NCASE '{': lsp+=3; L2=(cell_t)pc;
        NCASE '}': if (PP) { pc=(int)L2; } else { lsp -=3; }
    }
}

void c0(char c) {
    code[here].c[0] = c;
    code[++here].c[0] = 0;
}

void c01(char c0, char c1) {
    code[here].c[0] = c0;
    code[here].c[1] = c1-'A';
    code[++here].c[0] = 0;
}

void c0_l1(char c, int32_t l) {
    code[here].c[0] = c;
    code[here].i32[1] = l;
    code[++here].c[0] = 0;
}

void c01_l1(char c0, char c1, int32_t l) {
    code[here].c[0] = c0;
    code[here].c[1] = c1-'A';
    code[here].i32[1] = l;
    // printf("\nc0:%c,c1:%c,l:%d", code[here].c[0], code[here].c[1]+'A', code[here].i32[1]);
    code[++here].c[0] = 0;
}

void c0123(char c0, char c1, char c2, char c3) {
    code[here].c[0] = c0;
    code[here].c[1] = c1-'A';
    code[here].c[2] = c2-'A';
    code[here].c[3] = c3-'A';
    code[++here].c[0] = 0;
}

int strEq(const char *s, const char *d) {
    while (*s || *d) if (*(s++) != *(d++)) return 0;
    return 1;
}

char w[132], *in;

int nextWord() {
    // printf("-in:%s-",in);
    int l=0;
    while (*in && (*in < 33)) in++;
    while (*in && (*in > 32)) w[l++]=*(in++);
    w[l]=0;
    return l;
}

int doId() {
    printf("-id?:%s-", w);
    return 0;
}

int doRegR() {
    if (BTW(w[1],'A','Z')) { c01('r', w[1]); return 1; }
    if (BTW(w[1],'0','9')) { c01('l', w[1]); return 1; }
    return 0;
}

int doRegS() {
    if (BTW(w[1],'A','Z')) { c01('s', w[1]); return 1; }
    if (BTW(w[1],'0','9')) { c01('L', w[1]); return 1; }
    return 0;
}

void pw() {
    // printf("\n-pw:%s-",w);
    if (BTW(w[0],'0','9')) {       // 1234 or 1234sX
        int l=0;
        PS(w[l++]-'0');
        while (BTW(w[l],'0','9')) { S0=(S0*10)+(w[l++]-'0'); }
        if ((w[l]=='s') && BTW(w[l+1],'A','Z')) { c01_l1('m', w[l+1], PP32); }
        else if ((w[l]=='s') && BTW(w[l+1],'0','9')) { c01_l1('M', w[l+1], PP32); }
        else { c0_l1('n', PP32); }
        return;
    }

    switch (w[0]) {
        case  '+': c0123( w[0], w[1], w[2], w[3]);
        RETCASE '*': c0123( w[0], w[1], w[2], w[3]);
        RETCASE '-': c0123( w[0], w[1], w[2], w[3]);
        RETCASE '/': c0123( w[0], w[1], w[2], w[3]);
        RETCASE 'r': if (!doRegR()) { doId(); }        // RegRead
        RETCASE 's': if (!doRegS()) { doId(); }        // RegSet
        RETCASE 't': c01( w[0], w[1]);    // Timer
        RETCASE '.': c01( w[0], w[1] );
        RETCASE ',': c01( w[0], w[1] );
        RETCASE ']': c0(  w[0] );
        RETCASE '[': c0(  w[0] );
        return; default: printf("unknown [%d]", w[0]);
    }
}

void cc(char *x) {
    here = (here<1) ? 1 : here;
    in = x;
    while (nextWord()) {
        if (strEq(w,"//")) return;
        pw();
        code[0].i64 = here;
    }
}

int loop(FILE *fp) {
    int lh = here;
    char x[128], ok = 0;
    if (fp) {
        if (fgets(x, 128, fp) != x) { return 0; }
    } else { fgets(x, 128, stdin); }
    cc(x);
    return 1;
}

int main() {
    here = sp = rsp = lsp = 0;
    FILE *fp = fopen("code.txt", "rt");
    while (fp) {
        if (!loop(fp)) { 
            fclose(fp);
            fp=NULL;
            for (int i=0; i<here; i++) { dumpOp(i, &code[i]); }
            runReg(1);
        }
    }

    runStack("T 200000000 0[rArB+sC] T $-.N");
    // run("T 250 1000#**{d#}\\ T $-.N");
}
