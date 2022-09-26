// s3.cpp - inspired by STABLE from Sandor Schneider
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define btw(a,b,c) ((b<=a) && (a<=c))
#define TOS     st.i[s]
#define NOS     st.i[s-1]
#define R0      st.i[r]
#define R1      st.i[r+1]
#define R2      st.i[r+2]
#define BSZ    (64*1024)
#define ISZ      (BSZ/4)
#define FSZ        10
#define MAX_FN  0x3FF

union fib { float f[ISZ]; long i[ISZ]; }; static union fib st;
static char *y, stb[BSZ]; 
static long c, cb=1, h, sb=4, rb=64, rg=68, lb=125, r, s, t, u, fpSp;
static long fn, fa, funcs[MAX_FN+1], p;
static long sd, fp;
long fpStk[FSZ];

void init() {
    s=sb-1; h=cb; u=ISZ-500; fpSp=0;
    fp = (long)stdin;
    for (int i=0; i<ISZ; i++) { st.i[i]=0; }
    for (int i=0; i<BSZ; i++) { stb[i]=0; }
    for (int i=0; i<FSZ; i++) { fpStk[i]=0; }
    for (int i=0; i<MAX_FN; i++) { funcs[i]=0; }
    st.i[0] = h;
}
int funcN(int x) { 
    unsigned long hh = 5381;
    while (btw(stb[x],'A','Z') || btw(stb[x],'a','z')) { hh=((hh<<5)^hh)+(stb[x++]-'A'); }
    fn = (hh & MAX_FN); fa=funcs[fn]; // printf("\n(fn:%d,fa:%d)",fn, fa);
    return x;
}
void fSystem() { system((char*)st.i[s--]); }
void fOpen() { t=st.i[s--]; y=&stb[TOS]; TOS=(long)fopen(y, (t) ? "wb" : "rb"); }
void fClose() { if (TOS) { fclose((FILE*)TOS); } s--; }
void fLoad() { 
    st.i[++s]=0; fOpen(); t=st.i[s--];
    if (t) { if ((FILE*)fp != stdin) { fpStk[++fpSp] = fp; } fp = t; }
    else { printf("-loadFail-"); }
}
/* <33 */ void X() { if (u && (u != 10)) printf("-IR %d (%c)?", u, u); p = 0; } void N() {}
/*  !  */ void fStore() { st.i[TOS]=NOS; s-=2; }
/*  "  */ void fDotQ() { while (stb[p] != '"') { putc(stb[p++], stdout); } ++p; }
/*  #  */ void fDup() { t = TOS; st.i[++s] = t; }
/*  $  */ void fSwap() { t = TOS; TOS = NOS; NOS = t; }
/*  %  */ void fOver() { t = NOS; st.i[++s] = t; }
/*  &  */ void fSlMod() { u = NOS; t = TOS; NOS = u / t; TOS = u % t; }
/*  '  */ void fAscii() { st.i[++s] = stb[p++]; }
/*  (  */ void fIf() { if (st.i[s--] == 0) { while (stb[p++] != ')'); } }
/*  *  */ void fMult() { NOS *= TOS; s--; }
/*  +  */ void fAdd() { NOS += TOS; s--; }
/*  ,  */ void fEmit() { putc(st.i[s--], stdout); }
/*  -  */ void fSub() { NOS -= TOS; s--; }
/*  .  */ void fDot() { printf("%ld", st.i[s--]); }
/*  /  */ void fDiv() { NOS /= TOS; s--; }
/* 0-9 */ void n09() { st.i[++s] = (u - '0'); 
    while (btw(stb[p], '0', '9')) { TOS = (TOS * 10) + stb[p++] - '0'; }
    if (stb[p] == 'e') { ++p; st.f[s] = (float)TOS; } }
/*  :  */ void fCreate() { p=funcN(p); if (fa) { printf("-redef:%u at %d-", fn, fa); }
    while (stb[p]==' ') { ++p; } funcs[fn]=p;
    while (stb[p++]!=';') {}
    if (h<p) { h=p; } st.i[0]=h; }
/*  ;  */ void fRet() { p = st.i[r++]; if (rb < r) { r = rb; p = 0; } }
/*  <  */ void fLT() { t = TOS; u = NOS; s--; TOS = (u < t) ? -1 : 0; if (stb[p] == '=') { ++p; TOS = (u <= t) ? -1 : 0; } }
/*  =  */ void fEq() { NOS = (NOS == TOS) ? -1 : 0; s--; }
/*  >  */ void fGT() { t = TOS; u = NOS; s--; TOS = (u > t) ? -1 : 0; if (stb[p] == '=') { ++p; TOS = (u >= t) ? -1 : 0; } }
/*  ?  */ void fKey() { c = fgetc(stdin); st.i[++s] = (c != EOF) ? c : 0; }
/*  @  */ void fFetch() { TOS = st.i[TOS]; }
/* A-Z */ void AZ() { p=funcN(p-1); if (fn) { st.i[--r]=p; p=fa; } }
/*  \  */ void fDrop() { --s; }
/*  [  */ void fDo() { r -= 3; st.i[r+2]=p; st.i[r]=st.i[s--]; st.i[r+1]=st.i[s--]; }
/* n/a */ void fLoopS(int x) { if ((x==1) && (R0<R1)) { p=st.i[r+2]; return; }
            if ((x==0) && (R0>R1)) { p=R2; return; }
            r+=3; }
/*  ]  */ void fLoop() { ++st.i[r]; fLoopS(1); }
/*  ^  */ void fLeave() { p=st.i[r++]; }
/*  _  */ void fNeg() { TOS = -TOS; }
/*  `  */ void fSys() { c = BSZ-100; t=c;
        while ((31 < stb[p]) && (stb[p] != '`')) { stb[t++] = stb[p++]; }
        stb[t]=0; ++p; st.i[++s]=(long)&stb[c]; fSystem(); }
/*  b  */ void fBit() {
    u = stb[p++]; if (u == '~') { TOS = ~TOS; }
    else if (u == '&') { NOS &= TOS; s--; }
    else if (u == '|') { NOS |= TOS; s--; }
    else if (u == '^') { NOS ^= TOS; s--; }
    else { putc(32, stdout); --p; } }
/*  c  */ void fCOp() { u=stb[p++]; if (u=='@') { TOS=stb[TOS]; } else if (u=='!') { stb[TOS]=(char)NOS; s-=2; } }
/*  d  */ void fRegDec() { u = stb[p++]; if (btw(u, 'A', 'Z')) { st.i[u + 32]--; } else { --p; --TOS; } }
/*  e  */ void fExec() { st.i[--r] = p; p = st.i[s--]; }
/*  f  */ void fFloat() { u = stb[p++]; // printf("-flt:%c-",u);
    if (u == '.') { printf("%g", st.f[s--]); }
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
    else if (u == 'O') { fOpen(); }
    else if (u == 'C') { fClose(); }
    else if (u == 'L') { fLoad(); }
    else if (u == 'R') { st.i[++s]=0; if (NOS) fread((void*)&TOS, 1, 1, (FILE*)NOS); }
    else if (u == 'W') { if (TOS) { fwrite((void*)&NOS, 1, 1, (FILE*)TOS); } s -= 2; } }
/*  i  */ void fRegInc() { u = stb[p++]; if (btw(u, 'A', 'Z')) { st.i[u + 32]++; } else { --p; ++TOS; } }
/*  l  */ void fLoc() {
            u = stb[p++]; if (btw(u, '0', '9')) { st.i[++s] = lb + u - '0'; }
            else if (u == '+') { lb += (lb < 185) ? 10 : 0; }
            else if (u == '-') { lb -= (125 < lb) ? 10 : 0; }}
/*  n  */ void fIndex() { st.i[++s] = R0; }
/*  q  */ void fDotS() { int i; for (i = sb; i <= s; i++) { printf("%c%ld", (i == sb) ? 0 : 32, st.i[i]); } }
/*  r  */ void fRegGet() { u = stb[p++]; if (btw(u, 'A', 'Z')) { st.i[++s] = st.i[u + 32]; } }
/*  s  */ void fRegSet() { u = stb[p++]; if (btw(u, 'A', 'Z')) { st.i[u + 32] = st.i[s--]; } }
/*  t  */ void fClk() { st.i[++s] = clock(); }
/*  u  */ void fUser() { p=funcN(p); if (fa) { st.i[--r]=p; p=fa; } }
/*  x  */ void fExt() { u = stb[p++];
        if (u == '%') { NOS %= TOS; s--; } // MOD
        else if (u == 'U') { r+=3; } // UNLOOP
        else if (u == ']') { u=(R0<R1)?1:0; R0+=st.i[s--]; fLoopS(u); } // +LOOP
        else if (u == 'R') { sd = sd ? sd : (long)&cb; // RAND
            sd = (sd<<13)^sd; sd = (sd>>17)^sd; sd = (sd<<5)^sd;
            st.i[++s]=sd; } 
        else if (u == 'X') { init(); p=0; } // Reset
        else if (u == 'Y') { TOS=(long)&stb[TOS]; fSystem(); } // system
        else if (u == 'Z') { /* TOTO: zType */ } // ZTYPE
        else if (u == 'Q') { exit(0); }} // Exit
/*  {  */ void fBegin() { r-=3; R0=p; }
/*  |  */ void fQt() { while (stb[p] != '|') { stb[TOS++] = stb[p++]; } stb[TOS++] = 0; ++p; }
/*  }  */ void fWhile() { if (st.i[s--]) { p=R0; } else { r+=3; } }
/*  ~  */ void fLNot() { TOS=(TOS)?0:-1; }
void (*q[128])() = { 
    X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X, // 0:31
    N,fStore,fDotQ,fDup,fSwap,fOver,fSlMod,fAscii,fIf,N,fMult,fAdd,fEmit,fSub,fDot,fDiv, // 32:47
    n09,n09,n09,n09,n09,n09,n09,n09,n09,n09,fCreate,fRet,fLT,fEq,fGT,fKey, // 48:63
    fFetch,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ, // 64:79
    AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,fDo,fDrop,fLoop,fLeave,fNeg, // 80:95
    fSys,X,fBit,fCOp,fRegDec,fExec,fFloat,X,X,fRegInc,X,X,fLoc,X,fIndex,X, // 96:111
    X,fDotS,fRegGet,fRegSet,fClk,fUser,X,X,fExt,X,N,fBegin,fQt,fWhile,fLNot,X }; // 112:127
void R(int x) { s=(s<sb)?(sb-1):s; r=rb; p=x; while (p) { u=stb[p++]; q[u](); } }
void H(char* s) { FILE* fp = fopen("h.txt", "at"); if (fp) { fprintf(fp, "%s", s); fclose(fp); } }
void L() {
    if (feof((FILE*)fp)) {
        if (fp==(long)stdin) { exit(0); }
        st.i[++s]=fp; fClose();
        fp = (0<fpSp) ? fpStk[--fpSp] : (long)stdin;
    }
    if (fp == (long)stdin) { printf("\ns3:("); fDotS(); printf(")>"); }
    fgets(&stb[h], 128, (FILE*)fp);
    if (fp == (long)stdin) { H(&stb[h]); }
    R(h);
}
int main(int argc, char* argv[]) {
    init();
    st.i[lb] = argc;
    u = 1000;
    for (int i=1; i<argc; ++i) {
        y=argv[i]; t=atoi(y);
        if ((t) || (y[0] == '0' && y[1]==0)) { st.i[lb+i]=t; }
        else { st.i[lb+i]=u; for (int j=0; y[j]; j++) { stb[u++]=y[j]; } stb[u++]=0; }
    }
    if ((argc>1) && (argv[1][0]!='-')) {
        fp = (long)fopen(argv[1], "rb");
        if (!fp) { fp=(long)stdin; }
    }
    while (1) { L(); }
    return 0;
}
