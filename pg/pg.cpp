// S2.c - inspired by Sandor Schneider's STABLE (https://w3group.de/stable.html)

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define BTW(a,b,c) ((b<=a) && (a<=c))

#define R0 rtos
#define R1 rstk[rsp]

#define STK_SZ         64
#define DATA_SZ      1000
#define CODE_SZ     10000

#define RG(x)        regs[x-'A']
#define DST          RG('A')
#define IR           *(pc-1)
#define NR           *(pc++)

char code[CODE_SZ];
long data[DATA_SZ];
long regs[26], stk[STK_SZ];

static char ex[256], *pc;
static const char *y;
static int rsp, t1, t2;

float toFlt(int x) { return *(float*)&x; }
int toInt(float x) { return *(int*)&x; }
char *toNum(char *cp) { t1=*(cp++)-'0'; while (BTW(*cp,'0','9')) { t1=(t1*10)+*(cp++)-'0'; } return cp; }

void XXX() { if (IR && (IR!=10)) printf("-IR %d (%c)?", IR, IR); pc=0; }
/*<33*/ void NOP() { }
/* ! */ void f33() { if (BTW(*pc, '0', '9')) { pc = toNum(pc); } else { t1 = RG(NR); } data[t1] = DST; }
/* " */ void f34() { }
/* # */ void f35() { }
/* $ */ void f36() { }
/* % */ void f37() { }
/* & */ void f38() { }
/* ' */ void f39() { }
/* ( */ void f40() { }
/* ) */ void f41() { }
/* * */ void f42() { if (BTW(*pc,'0','9')) { pc=toNum(pc); DST*=t1; } else { DST*=RG(NR); } }
/* + */ void f43() { if (BTW(*pc,'0','9')) { pc=toNum(pc); DST+=t1; } else { DST+=RG(NR); } }
/* , */ void f44() { printf("%c", RG(NR)); }
/* - */ void f45() { if (BTW(*pc,'0','9')) { pc=toNum(pc); DST-=t1; } else { DST-=RG(NR); } }
/* . */ void f46() { printf("%d", RG(NR)); }
/* / */ void f47() { if (BTW(*pc,'0','9')) { pc=toNum(pc); DST/=t1; } else { DST/=RG(NR); } }
/*0-9*/ void n09() { pc = toNum(pc-1); DST = t1; }
/* 0 */ void f48() { }
/* 1 */ void f49() { }
/* 2 */ void f50() { }
/* 3 */ void f51() { }
/* 4 */ void f52() { }
/* 5 */ void f53() { }
/* 6 */ void f54() { }
/* 7 */ void f55() { }
/* 8 */ void f56() { }
/* 9 */ void f57() { }
/* : */ void f58() { RG(NR) = DST; }
/* ; */ void f59() { t1 = NR; RG(NR) = RG(t1); }
/* < */ void f60() { }
/* = */ void f61() { }
/* > */ void f62() { }
/* ? */ void f63() { }
/* @ */ void f64() { if (BTW(*pc, '0', '9')) { pc = toNum(pc); } else { t1 = RG(NR); } DST = data[t1]; }
/*A2Z*/ void A2Z() { DST = RG(IR); }
/* [ */ void f91() { }
/* \ */ void f92() { }
/* ] */ void f93() { }
/* ^ */ void f94() { }
/* _ */ void f95() { }
/* ` */ void f96() { }
/*a-z*/ void a2z() { t1 = IR; printf("-call (%c)-", t1); }
/* { */ void f123() { }
/* | */ void f124() { }
/* } */ void f125() { }
/* ~ */ void f126() { printf("bye."); exit(0); }

void (*jt[128])()={
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //   0 ..  15
    XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,  XXX,   //  16 ..  31
    NOP,  f33,  f34,  f35,  f36,  f37,  f38,  f39,  f40,  NOP,  f42,  f43,  f44,  f45,  f46,  f47,   //  32 ..  47
    n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  n09,  f58,  f59,  f60,  f61,  f62,  f63,   //  48 ..  63
    f64,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,   //  64 ..  79
    A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  A2Z,  f91,  f92,  f93,  f94,  f95,   //  80 ..  95
    f96,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,   //  96 .. 111
    a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  a2z,  f123, f124, f125, f126, XXX    // 112 .. 127
};

void R(const char *x) {
    rsp = 0;
    pc = (char *)x;
    while (pc) { jt[*(pc++)](); }
}
void H(char *s) { /* FILE* fp = fopen("h.txt", "at"); if (fp) { fprintf(fp, "%s", s); fclose(fp); } */ }
void L() {
    printf(" (ok)\npg: "); // f37(); printf(")>"); 
    fgets(ex, 128, stdin); H(ex); R(ex);
}
int main(int argc, char *argv[]) {
    //int i,j; s=sb-1; h=cb; ir=SZ-500; for (i=0; i<(SZ/4); i++) { st.i[i]=0; }
    //st.i[0]=h; st.i[lb]=argc; for (i=1; i < argc; ++i) { y=argv[i]; t=atoi(y);
    //    if ((t) || (y[0]=='0' && y[1]==0)) { st.i[lb+i]=t; }
    //    else { st.i[lb+i]=ir; for (j=0; y[j]; j++) { st.b[ir++]=y[j]; } st.b[ir++]=0; } }
    //if ((argc>1) && (argv[1][0]!='-')) { FILE *fp=fopen(argv[1], "rb"); 
    //    if (fp) {while ((c=fgetc(fp))!=EOF) { st.b[h++]=(31<c)?c:32; } fclose(fp); st.i[0]=h; R(cb); }
    //} 
    R("32:L 10:N");
    R("123:B 321+B :C .A ,L .C ,N");
    R("33:B ;BC .C ,N");
    R("200:B 111!B @200 .B ,L .A ,N");
    while (1) { L(); }
    return 0;
}
