#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define SNEXT    goto stkNext
#define SCASE    SNEXT; case
#define RNEXT    goto regNext
#define RCASE    RNEXT; case
#define BCASE    break; case
#define RETCASE  return; case
#define PS(x)    stk[++sp]=(cell_t)(x)
#define PP       stk[sp--]
#define PP32     (int32_t)stk[sp--]
#define S0       stk[sp]
#define S1       stk[sp-1]
#define S2       stk[sp-2]
#define RPS(x)   rstk[++rsp]=(x)
#define RPP      rstk[rsp--]
#define R0       rstk[rsp]
#define R1       rstk[rsp-1]
#define R2       rstk[rsp-2]
#define D1       sp--
#define D2       sp-=2
#define L0       lstk[lsp]
#define L1       lstk[lsp-1]
#define L2       lstk[lsp-2]
#define BTW(a,b,c) ((b<=a)&&(a<=c))

#define CELL_SZ sizeof(cell_t)

typedef int64_t cell_t;
typedef union { char c[8]; short in[4]; int32_t i32[2]; int64_t i64; double d; } op_t;
typedef struct { char name[32]; int val; } lbl_t;

op_t code[256], *op;
cell_t stk[32], rstk[32], lstk[30], t;
cell_t reg[26], loc[10];
lbl_t lbl[512];
int sp, rsp, lsp, locBase, here, vhere, u, pc, nlbls, err;

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

enum {	ADD='+', SUB='-', MULT='*', DIV='/',
	EMIT=',', DOT='.', FOR='[', NEXT=']',
	TIME='t', RLOC='l', SLOC='L', NLOC='M',
	RREG='r', SREG='s', NREG='n', CREG='C', XREG='x',
	PNUM='P', LBL=':', VAR='v', EOL=0
};

void runReg(int start) {
    pc = start;
    regNext:
    op = &code[pc++];
    // dumpOp(pc, op);
    switch (OPC(0)) {
        case  0: return;
        case  ADD:  reg[OPC(3)] = reg[OPC(1)] + reg[OPC(2)];
        RCASE MULT: reg[OPC(3)] = reg[OPC(1)] * reg[OPC(2)];
        RCASE SUB:  reg[OPC(3)] = reg[OPC(1)] - reg[OPC(2)];
        RCASE DIV:  reg[OPC(3)] = reg[OPC(1)] / reg[OPC(2)];
        RCASE DOT:  printf(" %jd", reg[OPC(1)]);
        RCASE EMIT: printf("%c",  (char)reg[OPC(1)]);
        RCASE FOR:  lsp+=3; L0=PP; L1=PP; L2=pc;
        RCASE NEXT: if (++L0<L1) { pc=(int)L2; } else { lsp-=3; }
        RCASE TIME: t=clock(); if (BTW(OPC(1),0,25)) { reg[OPC(1)]=t; } else { PS(t); }
        RCASE RLOC: PS(loc[locBase+OPC(1)]);
        RCASE SLOC: loc[locBase+OPC(1)] = PP;
        RCASE NLOC: loc[locBase+OPC(1)] = OP32(1);
        RCASE PNUM: PS(OP32(1));

        RCASE NREG: reg[OPC(1)] = OP32(1);
        RCASE RREG: PS(reg[OPC(1)]);
        RCASE SREG: reg[OPC(1)] = PP;
        RCASE CREG: reg[OPC(2)] = reg[OPC(1)];
        
        RNEXT; default: dumpOp(pc-1, op); RNEXT;
    }
}

void runStack(const char *x) {
    pc = 0;
    stkNext:
    u = x[pc++];

    switch(u) {
        case  0: return;
        case  ' ':
        SCASE '!': printf("%c",u);
        SCASE '"': printf("%c",u);
        SCASE '#': t=S0; PS(t);
        SCASE '$': t=S0; S0=S1; S1=t;
        SCASE '%': t=S1; PS(t);
        SCASE '&': printf("%c",u);
        SCASE '\'': D1;
        SCASE '(': if (PP==0) { while (x[pc++]!=')') {} }
        SCASE ')': printf("%c",u);
        SCASE '*': S1*=S0; D1;
        SCASE '+': S1+=S0; D1;
        SCASE ',': printf("%c",u);
        SCASE '-': S1-=S0; D1;
        SCASE '.': printf(" %jd",PP);
        SCASE '/': S1/=S0; D1;
        SCASE '0': case '1': case '2': case '3': case  '4': case '5': case '6': case '7':
        case  '8': case '9': PS(u-'0'); while (BTW(x[pc],'0','9')) { S0=(S0*10)+(x[pc++]-'0'); }
        SCASE '<': S1=(S1<S0)?-1:0;  D1;
        SCASE '=': S1=(S1==S0)?-1:0; D1;
        SCASE '>': S1=(S1>S0)?-1:0;  D1;
        SCASE 'I': PS(L0);
        SCASE 'N': printf("\n");
        SCASE 'Q': exit(0);
        SCASE 'T': PS(clock());
        SCASE '[': lsp+=3; L0=PP; L1=PP; L2=(cell_t)pc;
        SCASE '\\': if (0<sp) sp--;
        SCASE ']': if (++L0<L1) { pc=(int)L2; } else { lsp-=3; }
        SCASE 'd': --S0;
        SCASE 'i': ++S0;
        SCASE 'l': u=x[pc++]; PS(&loc[u-'0']);
        SCASE 'r': u=x[pc++]; PS(reg[u-'A']);
        SCASE 's': u=x[pc++]; reg[u-'A']=PP;
        SCASE '{': lsp+=3; L2=(cell_t)pc;
        SCASE '}': if (PP) { pc=(int)L2; } else { lsp -=3; }
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

int strCpy(char *d, const char *s) {
    int l = 0;
    while (*s) { *(d++) = *(s++); ++l; }
    *d = 0;
    return l;
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

int nextToken() {
    if (!nextWord()) return EOL;
    if (strEq(w, "//")) return EOL;
    if (w[0] == '+') return ADD;
    if (w[0] == '*') return MULT;
    if (w[0] == '-') return SUB;
    if (w[0] == '/') return DIV;
    if (w[0] == ':') return LBL;
    if (strEq(w, "[")) return FOR;
    if (strEq(w, "]")) return NEXT;
    if (strEq(w, "var")) return VAR;
    if (w[0] == 'r') return RREG;
    if (w[0] == 's') return SREG;
    if (w[0] == 't') return TIME;
    if (w[0] == ',') return EMIT;
    if (w[0] == '.') return DOT;
    if (BTW(w[0], '0', '9')) return PNUM;
    printf("-token:[%s]?-", w);
    err = 1;
    return EOL;
}

int doLabel(int isVar) {
    if (strCpy(lbl[nlbls].name, &w[1])) {
        lbl[nlbls].val = (isVar==0) ? here : vhere;
        if (isVar) { vhere += sizeof(int64_t); }
        printf("-lbl:%s:%d-", w+1, lbl[nlbls].val);
        ++nlbls;
    }

    return 0;
}

int doId() {
    for (int i=0; i<nlbls; i++) {
        if (strEq(w,lbl[i].name)) {
            printf("-id:%s:%d-", w, lbl[i].val);
            return 0;
        }
    }
    printf("-id:%s?-", w);
    return 0;
}

int doRegR() {
    if (BTW(w[1],'A','Z')) { c01(RREG, w[1]); return 1; }
    if (BTW(w[1],'0','9')) { c01(RLOC, w[1]); return 1; }
    return 0;
}

int doRegS() {
    if (BTW(w[1],'A','Z')) { c01(SREG, w[1]); return 1; }
    if (BTW(w[1],'0','9')) { c01(SLOC, w[1]); return 1; }
    return 0;
}

void doNum() {
    int l=0;
    PS(w[l++]-'0');
    while (BTW(w[l],'0','9')) { S0=(S0*10)+(w[l++]-'0'); }
    if ((w[l]=='s') && BTW(w[l+1],'A','Z')) { c01_l1(NREG, w[l+1], PP32); }
    else if ((w[l]=='s') && BTW(w[l+1],'0','9')) { c01_l1(NLOC, w[l+1], PP32); }
    else { c0_l1(PNUM, PP32); }
    return;
 }

void pw(int tok) {
    // printf("\n-pw:%s-",w);

    if (BTW(w[0],'A','Z')) {
        doId();
        return;
    }

    switch (tok) {
        case  PNUM: doNum();
        RETCASE ADD:  c0123( ADD, w[1], w[2], w[3]);
        RETCASE MULT: c0123( MULT, w[1], w[2], w[3]);
        RETCASE SUB:  c0123( SUB, w[1], w[2], w[3]);
        RETCASE DIV:  c0123( DIV, w[1], w[2], w[3]);
        RETCASE RREG: if (!doRegR()) { doId(); }        // RegRead
        RETCASE SREG: if (!doRegS()) { doId(); }        // RegSet
        RETCASE TIME: c01( TIME, w[1]);    // Timer
        RETCASE DOT:  c01( DOT, w[1] );
        RETCASE EMIT: c01( EMIT, w[1] );
        RETCASE FOR:  c0(  FOR );
        RETCASE NEXT: c0(  NEXT );
        RETCASE LBL:  doLabel(0);
        RETCASE VAR:  doLabel(1);
        return; default: printf("unknown [%d]", w[0]);
    }
}

void cc(char *x) {
    here = (here<1) ? 1 : here;
    in = x;
    while (1) {
        int tok = nextToken();
        if ((err) || (tok == EOL)) return;
        pw(tok);
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
    return (err) ? 0 : 1;
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

    runStack("NT 150000000 0[rArB+sC] T $-.N");
    // run("T 250 1000#**{d#}\\ T $-.N");
}
