#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

#define NEXT   goto next
#define NCASE  NEXT; case
#define NEXT2  goto next2
#define NCASE2 NEXT2; case
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
typedef union { char c[8]; short s[4]; int32_t i32[2]; int64_t i64; double d; } op_t;

op_t code[256], *op;
cell_t stk[32], rstk[32], lstk[30], t;
cell_t reg[26], loc[10];
int sp, rsp, lsp, here, u, pc;


#define OPC(x)   op->c[x]
#define OP32(x)  op->i32[x]

void dumpOp(op_t *op) {
    printf("\npc: %d: ", pc-1);
    for (int i=0; i<CELL_SZ; i++) { printf("[%d,%02d]", i, OPC(i)); }
}

void run2(int start) {
    pc = start;
    next2:
    op = &code[pc++];
    switch (OPC(0)) {
        case  0: return;
        case   '+': reg[OPC(3)] = reg[OPC(1)] + reg[OPC(2)];
        NCASE2 '*': reg[OPC(3)] = reg[OPC(1)] * reg[OPC(2)];
        NCASE2 '-': reg[OPC(3)] = reg[OPC(1)] - reg[OPC(2)];
        NCASE2 '/': reg[OPC(3)] = reg[OPC(1)] / reg[OPC(2)];
        NCASE2 '.': printf(" %jd", reg[OPC(1)]);
        NCASE2 ',': printf("%c",  (char)reg[OPC(1)]);
        NCASE2 '[': lsp+=3; L0=PP; L1=PP; L2=pc;
            // printf("-[%ld:%ld]-", L0, L1);
        NCASE2 ']': if (++L0<L1) { pc=(int)L2; } else { lsp-=3; }
        NCASE2 't': t=clock(); if (BTW(OPC(1),0,25)) { reg[OPC(1)]=t; } else { PS(t); }
        NCASE2 'n': PS(OP32(1));
            // printf("-n[%d]-", OP32(1));
        NCASE2 'r': PS(reg[OPC(1)]);
        NCASE2 's': reg[OPC(1)] = OP32(1);
            // printf("-s%c:%ld-", OPC(1)+'A', reg[OPC(1)]);
            NEXT2;
        default: dumpOp(op); NEXT2;
    }
}

void run(const char *x) {
    pc = 0;
    next:
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
    NCASE '0': case '1': case '2': case '3':
    case  '4': case '5': case '6': case '7':
    case  '8': case '9': PS(u-'0');
        while (BTW(x[pc],'0','9')) { S0=(S0*10)+(x[pc++]-'0'); }
    NCASE ':': printf("%c",u);
    NCASE ';': printf("%c",u);
    NCASE '<': S1=(S1<S0)?-1:0;  D1;
    NCASE '=': S1=(S1==S0)?-1:0; D1;
    NCASE '>': S1=(S1>S0)?-1:0;  D1;
    NCASE '?': printf("%c",u);
    NCASE '@': printf("%c",u);
    NCASE 'A': printf("%c",u);
    NCASE 'B': printf("%c",u);
    NCASE 'C': printf("%c",u);
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
    NCASE '[': lsp+=3; L0=PP; L1=PP; L2=(cell_t)pc;
    NCASE '\\': if (0<sp) sp--;
    NCASE ']': if (++L0<L1) { pc=(int)L2; } else { lsp-=3; }
    NCASE '^': printf("%c",u);
    NCASE '_': printf("%c",u);
    NCASE '`': printf("%c",u);
    NCASE 'a': printf("%c",u);
    NCASE 'b': printf("%c",u);
    NCASE 'c': printf("%c",u);
    NCASE 'd': --S0;
    NCASE 'e': printf("%c",u);
    NCASE 'f': printf("%c",u);
    NCASE 'g': printf("%c",u);
    NCASE 'h': printf("%c",u);
    NCASE 'i': ++S0;
    NCASE 'j': printf("%c",u);
    NCASE 'k': printf("%c",u);
    NCASE 'l': u=x[pc++]; PS(&loc[u-'0']);
    NCASE 'm': printf("%c",u);
    NCASE 'n': printf("%c",u);
    NCASE 'o': printf("%c",u);
    NCASE 'p': printf("%c",u);
    NCASE 'q': printf("%c",u);
    NCASE 'r': u=x[pc++]; PS(reg[u-'A']);
    NCASE 's': u=x[pc++]; reg[u-'A']=PP;
    NCASE 't': printf("%c",u);
    NCASE 'u': printf("%c",u);
    NCASE 'v': printf("%c",u);
    NCASE 'w': printf("%c",u);
    NCASE 'x': printf("%c",u);
    NCASE 'y': printf("%c",u);
    NCASE 'z': printf("%c",u);
    NCASE '{': lsp+=3; L2=(cell_t)pc;
    NCASE '|': printf("%c",u);
    NCASE '}': if (PP) { pc=(int)L2; } else { lsp -=3; }
    NCASE '~': printf("%c",u);
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

void cc(const char *s) {
    int h = here;
    while (*s) {
        if (*s==' ') { ++s; }
        else if (*s=='+') { c0123( s[0], s[1], s[2], s[3]); s += 4; }
        else if (*s=='*') { c0123( s[0], s[1], s[2], s[3]); s += 4; }
        else if (*s=='-') { c0123( s[0], s[1], s[2], s[3]); s += 4; }
        else if (*s=='/') { c0123( s[0], s[1], s[2], s[3]); s += 4; }
        else if (*s=='t') { c01(   s[0], s[1]            ); s += 2; }
        else if (*s=='r') { c01(   s[0], s[1]            ); s += 2; }
        else if (*s=='.') { c01(   s[0], s[1]            ); s += 2; }
        else if (*s==',') { c01(   s[0], s[1]            ); s += 2; }
        else if (*s=='[') { c0(    s[0]                  ); s += 1; }
        else if (*s==']') { c0(    s[0]                  ); s += 1; }
        else if (BTW(*s,'0','9')) { // 1234 or 1234sX
            PS(*(s++)-'0');
            while (BTW(*s,'0','9')) { S0=(S0*10)+(*(s++)-'0'); }
            if ((s[0]=='s') && BTW(s[1],'A','Z')) { c01_l1('s' ,s[1], PP32); s += 2; }
            else { c0_l1('n', PP32); }
        }
        else if (*s<32) {return; }
        else { printf("unknown [%d]", *(s++)); }
    }
    // run2(h);
}

int loop(FILE *fp) {
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
            run2(0);
        }
    }

    run("T 300000000 0[rArB+sC] T $-.N");
    // run("T 250 1000#**{d#}\\ T $-.N");
}
