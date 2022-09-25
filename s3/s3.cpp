// S2.c - inspired by, and based on, STABLE from Sandor Schneider
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define btw(a,b,c) ((b<=a) && (a<=c))
#define TOS st.i[s]
#define NOS st.i[s-1]
#define SZ 10000
union fib { float f[SZ / 4]; long i[SZ / 4]; char b[SZ]; }; static union fib st;
static char ex[80], * y; static int c, cb = 800, h, p, lb = 125, rg = 68, rb = 64, r, sb = 4, s, t, u;
/* <33 */ void X() { if (u && (u != 10)) printf("-IR %d (%c)?", u, u); p = 0; } void N() {}
/*  !  */ void fStore() { st.i[TOS] = NOS; s -= 2; }
/*  "  */ void fDotQ() { while (st.b[p] != '"') { putc(st.b[p++], stdout); } ++p; }
/*  #  */ void fDup() { t = TOS; st.i[++s] = t; }
/*  $  */ void fSwap() { t = TOS; TOS = NOS; NOS = t; }
/*  %  */ void fOver() { t = NOS; st.i[++s] = t; }
/*  &  */ void fSlMod() { u = NOS; t = TOS; NOS = u / t; TOS = u % t; }
/*  '  */ void fAscii() { st.i[++s] = st.b[p++]; }
/*  (  */ void fIf() { if (st.i[s--] == 0) { while (st.b[p++] != ')'); } }
/*  *  */ void fMult() { NOS *= TOS; s--; }
/*  +  */ void fAdd() { NOS += TOS; s--; }
/*  ,  */ void fEmit() { putc(st.i[s--], stdout); }
/*  -  */ void fSub() { NOS -= TOS; s--; }
/*  .  */ void fDot() { printf("%ld", st.i[s--]); }
/*  /  */ void fDiv() { NOS /= TOS; s--; }
/* 0-9 */ void n09() {
    st.i[++s] = (u - '0'); while (btw(st.b[p], '0', '9')) { TOS = (TOS * 10) + st.b[p++] - '0'; }
    if (st.b[p] == 'e') { ++p; st.f[s] = (float)TOS; } }
/*  :  */ void fFunc() {
    u = st.b[p++]; if (!btw(u, 'A', 'Z')) { return; } if (st.b[p] == ' ') { ++p; }
    st.i[u] = p; while (st.b[p++] != ';') {} if (h < p) { h = p; } st.i[0] = h; }
/*  ;  */ void fRet() { p = st.i[r++]; if (rb < r) { r = rb; p = 0; } }
/*  <  */ void fLT() { t = TOS; u = NOS; s--; TOS = (u < t) ? -1 : 0; if (st.b[p] == '=') { ++p; TOS = (u <= t) ? -1 : 0; } }
/*  =  */ void fEq() { NOS = (NOS == TOS) ? -1 : 0; s--; }
/*  >  */ void fGT() { t = TOS; u = NOS; s--; TOS = (u > t) ? -1 : 0; if (st.b[p] == '=') { ++p; TOS = (u >= t) ? -1 : 0; } }
/*  ?  */ void fKey() { c = fgetc(stdin); st.i[++s] = (c != EOF) ? c : 0; }
/*  @  */ void fFetch() { TOS = st.i[TOS]; }
/* A-Z */ void AZ() { if (st.i[u]) { if (st.b[p] != ';') { st.i[--r] = p; } p = st.i[u]; } }
/*  \  */ void fDrop() { --s; }
/*  [  */ void fDo() { r -= 3; st.i[r+2]=p; st.i[r]=st.i[s--]; st.i[r+1]=st.i[s--]; }
/* n/a */ void fLoopS(int x) { if ((x==1) && (st.i[r]<st.i[r+1])) { p=st.i[r+2]; return; }
            if ((x==0) && (st.i[r]>st.i[r+1])) { p=st.i[r+2]; return; }
            r+=3; }
/*  ]  */ void fLoop() { ++st.i[r]; fLoopS(1); }
/*  ^  */ void f94() { p=st.i[r++]; }
/*  _  */ void fNeg() { TOS = -TOS; }
/*  `  */ void fSys() { y = ex; while ((31 < st.b[p]) && (st.b[p] != '`')) { *(y++) = st.b[p++]; } *y = 0; ++p; system(ex); }
/*  b  */ void fBit() {
    u = st.b[p++]; if (u == '~') { TOS = ~TOS; }
    else if (u == '&') { NOS &= TOS; s--; }
    else if (u == '|') { NOS |= TOS; s--; }
    else if (u == '^') { NOS ^= TOS; s--; }
    else { putc(32, stdout); --p; } }
/*  c  */ void fCOp() { u = st.b[p++]; if (u == '@') { TOS = st.b[TOS]; } else if (u == '!') { st.b[TOS] = NOS; s -= 2; } }
/*  d  */ void fRDec() { u = st.b[p++]; if (btw(u, 'A', 'Z')) { st.i[u + 32]--; } else { --p; --TOS; } }
/*  e  */ void fExec() { st.i[--r] = p; p = st.i[s--]; }
/*  f  */ void fFloat() {
    u = st.b[p++];                                if (u == '.') { printf("%g", st.f[s--]); }
    else if (u == '@') { st.f[s] = st.f[TOS]; }
    else if (u == '!') { st.f[TOS] = st.f[s - 1]; s -= 2; }
    else if (u == '+') { st.f[s - 1] += st.f[s]; s--; }
    else if (u == '<') { TOS = (st.f[s - 1] < st.f[s]) ? -1 : 0; }
    else if (u == '-') { st.f[s - 1] -= st.f[s]; s--; }
    else if (u == '>') { TOS = (st.f[s - 1] > st.f[s]) ? -1 : 0; }
    else if (u == '*') { st.f[s - 1] *= st.f[s]; s--; }
    else if (u == 'i') { TOS = (int)st.f[s]; }
    else if (u == '/') { st.f[s-1] /= st.f[s]; s--; }
    else if (u == 'f') { st.f[s] = (float)TOS; }
    // else if (u == 's') { st.f[s] = (float)sqrt(st.f[s]); }
    // else if (u == 't') { st.f[s] = (float)tanh(st.f[s]); }
    else if (u == 'O') { y = &st.b[NOS]; t = TOS; NOS = (long)fopen(y, (t) ? "wb" : "rb"); s--; }
    else if (u == 'C') { if (TOS) { fclose((FILE*)TOS); } s--; }
    else if (u == 'R') { s++; TOS = 0; if (NOS) fread((void*)&TOS, 1, 1, (FILE*)NOS); }
    else if (u == 'W') { if (TOS) { fwrite((void*)&NOS, 1, 1, (FILE*)TOS); } s -= 2; } }
/*  i  */ void fRInc() { u = st.b[p++]; if (btw(u, 'A', 'Z')) { st.i[u + 32]++; } else { --p; ++TOS; } }
/*  l  */ void fLoc() {
            u = st.b[p++]; if (btw(u, '0', '9')) { st.i[++s] = lb + u - '0'; }
            else if (u == '+') { lb += (lb < 185) ? 10 : 0; }
            else if (u == '-') { lb -= (125 < lb) ? 10 : 0; }}
/*  n  */ void fIndex() { st.i[++s] = st.i[r]; }
/*  q  */ void fDotS() { int i; for (i = sb; i <= s; i++) { printf("%c%ld", (i == sb) ? 0 : 32, st.i[i]); } }
/*  r  */ void fReg() { u = st.b[p++]; if (btw(u, 'A', 'Z')) { st.i[++s] = st.i[u + 32]; } }
/*  s  */ void fRSet() { u = st.b[p++]; if (btw(u, 'A', 'Z')) { st.i[u + 32] = st.i[s--]; } }
/*  t  */ void fClk() { st.i[++s] = clock(); }
/*  x  */ void fExt() { u = st.b[p++]; if (u == '%') { NOS %= TOS; s--; }
        else if (u == 'L') { r+=3; }
        else if (u == ']') { u=(st.i[r]<st.i[r+1])?1:0; st.i[r]+=st.i[s--]; fLoopS(u); }
        else if (u == 'Q') { exit(0); }}
/*  {  */ void fBegin() { r-=3; st.i[r]=p; }
/*  |  */ void fQt() { while (st.b[p] != '|') { st.b[TOS++] = st.b[p++]; } st.b[TOS++] = 0; ++p; }
/*  }  */ void fWhile() { if (st.i[s--]) { p=st.i[r]; } else { r+=3; } }
/*  ~  */ void fLNot() { TOS=(TOS)?0:-1; }
void (*q[127])() = { X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,N,fStore,fDotQ,fDup,fSwap,fOver,fSlMod,
    fAscii,fIf,N,fMult,fAdd,fEmit,fSub,fDot,fDiv,n09,n09,n09,n09,n09,n09,n09,n09,n09,n09,fFunc,fRet,fLT,fEq,fGT,fKey,fFetch,
    AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,fDo,fDrop,fLoop,f94,fNeg,fSys,X,
    fBit,fCOp,fRDec,fExec,fFloat,X,X,fRInc,X,X,fLoc,X,fIndex,X,X,fDotS,fReg,fRSet,fClk,X,X,X,fExt,X,N,fBegin,fQt,fWhile,fLNot };
void R(int x) { s = (s < sb) ? (sb - 1) : s; r = rb; p = x; while (p) { u = st.b[p++]; q[u](); } }
void H(char* s) { FILE* fp = fopen("h.txt", "at"); if (fp) { fprintf(fp, "%s", s); fclose(fp); } }
void L() { y = &st.b[h]; printf("\ns3:("); fDotS(); printf(")>"); fgets(y, 128, stdin); H(y); R(h); }
int main(int argc, char* argv[]) {
    int i, j; s=sb-1; h=cb; u=SZ-500; for (i=0; i<(SZ/4); i++) { st.i[i]=0; }
    st.i[lb] = argc; for (i=1; i<argc; ++i) {
        y=argv[i]; t=atoi(y);
        if ((t) || (y[0] == '0' && y[1]==0)) { st.i[lb+i]=t; }
        else { st.i[lb+i]=u; for (j=0; y[j]; j++) { st.b[u++]=y[j]; } st.b[u++]=0; } }
    if ((argc>1) && (argv[1][0]!='-')) {
        FILE* fp = fopen(argv[1], "rb");
        if (fp) { while ((c=fgetc(fp))!=EOF) { if (btw(c,32,126)) st.b[h++]=c; } fclose(fp); st.i[0]=h; R(cb); }
    } while (1) { L(); }; return 0;
}
