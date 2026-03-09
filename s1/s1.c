// s1.c - inspired by Sandor Schneider's STABLE (https://w3group.de/stable.html)
// Memory: [ SYS |STK  |FUNCS |VARS   |RSTK    |LSTK    |CODE    ]
//         [ 0-3 |4-64 |65-90 |97-122 |128-199 |200-255 |256-END ]
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SZ 65536
#define NFUNC 26
#define btw(a,b,c) ((b<=a) && (a<=c))
#define TOS st.i[s]
#define NOS st.i[s-1]
#define BP  st.b[p]
#define BPP st.b[p++]
#define RCASE return; case
static union { float f[SZ/4]; long i[SZ/4]; char b[SZ]; } st; static char ex[80], *y;
static int sb=4, s, rb='z', r, fb=128, lb=200, cb=(256*4), l=200, c, h, p, t, u;
/* <33 */ void N() {} void X() { if (u && (u!=10)) printf("-IR %d (%c)?", u, u); p=0; }
/*  !  */ void f33() { st.i[TOS]=NOS; s-=2; }
/*  "  */ void f34() { while (BP!='"') { putchar(BPP); } ++p; }
/*  #  */ void f35() { ++s; TOS=NOS; }
/*  $  */ void f36() { t=TOS; TOS=NOS; NOS=t; }
/*  %  */ void f37() { t=NOS; st.i[++s]=t; }
/*  &  */ void f38() { u=NOS; t=TOS; TOS=u/t; NOS=u%t; }
/*  '  */ void f39() { st.i[++s]=BPP; }
/*  (  */ void f40() { if (st.i[s--]==0) { while (BPP!=')'); } }
/*  *  */ void f42() { NOS *= TOS; s--; }
/*  +  */ void f43() { if (BP=='+') { ++TOS; p++; } else { NOS += TOS; s--; } }
/*  ,  */ void f44() { putchar(st.i[s--]); }
/*  -  */ void f45() { if (BP=='-') { --TOS; p++; } else { NOS -= TOS; s--; } }
/*  .  */ void f46() { printf("%ld", st.i[s--]); }
/*  /  */ void f47() { NOS /= TOS; s--; }
          void nS() { u=1; t=0; while (btw(BP,'0','9')) { t=(t*10)+BPP-'0'; u*=10; } }
/* 0-9 */ void n09() { --p; nS(); st.i[++s]=t; if (BP!='.') {return; }
            ++p; nS(); st.f[s]=st.i[s]; st.f[s]+=((float)t/(float)u); }
/*  :  */ void f58() { u=BPP; if (!btw(u,'A','Z')) { return; } st.i[u]=p;
            while (BP && (BP!=':')) { ++p; } BPP=';'; st.i[0]=h=p; }
/*  ;  */ void f59() { if (rb<r) { p=st.i[r--]; } else { r=rb; p=0; } }
/*  <  */ void f60() { t=TOS; u=NOS; s--; TOS=(u<t)?-1:0; if (BP=='=') { ++p; TOS=(u<=t)?-1:0; } }
/*  =  */ void f61() { NOS=(NOS==TOS)?-1:0; s--; }
/*  >  */ void f62() { t=TOS; u=NOS; s--; TOS=(u>t)?-1:0; if (BP=='=') { ++p; TOS=(u>=t)?-1:0; } }
/*  ?  */ void f63() { c=fgetc(stdin); st.i[++s]=(c!=EOF)?c:0; }
/*  @  */ void f64() { TOS=st.i[TOS]; }
/* A-Z */ void AZ()  { t=st.i[u]; if (t) { if (BP!=';') { st.i[++r]=p; } p=t; } }
/*  [  */ void f91() { l+=3; st.i[l-2]=p; st.i[l-1]=st.i[s--]; st.i[l]=0; }
/*  \  */ void f92() { s=(s<sb)?sb-1:s-1; }
/*  ]  */ void f93() { ++st.i[l]; if (st.i[l] < st.i[l-1]) { p=st.i[l-2]; } else { l=(lb<l)?lb:l-3; } }
/*  ^  */ void f94() { switch (BPP) {              case 'T': st.i[++s]=clock();
            RCASE '+': st.f[s-1]+=st.f[s]; s--;   RCASE '-': st.f[s-1]-=st.f[s]; s--;
            RCASE 'F': if (lb<l) { l-=3; }        RCASE 'W': if (lb<l) { lb--; }
            RCASE 'Y': system(&st.b[TOS]); s--;   RCASE 'I': st.i[++s]=st.i[l];
            RCASE 'X': exit(0); } }
/*  _  */ void f95() { TOS=-TOS; }
/*  `  */ void f96() { y=ex; while ((31<BP) && (BP!='`')) { *(y++)=BPP; } *y=0; ++p; system(ex); }
/* a-z */ void az()  { if (BP=='@') { st.i[++s]=st.i[u]; ++p; }
            else if (BP=='!') { st.i[u]=st.i[s--]; ++p; }
            if (BP=='+') { ++st.i[u]; ++p; }
            else if (BP=='-') { --st.i[u]; ++p; } }
/*  c  */ void f99() { switch (BPP) { case '@': TOS=st.b[TOS]; RCASE '!': st.b[TOS]=(NOS&0xFF); s-=2; } }
/*  f  */ void f102() { switch (BPP) {             case '.':  printf("%g", st.f[s--]);
            RCASE '+': st.f[s-1]+=st.f[s]; s--;   RCASE '-': st.f[s-1]-=st.f[s]; s--;
            RCASE '*': st.f[s-1]*=st.f[s]; s--;   RCASE '/': st.f[s-1]/=st.f[s]; s--;
            RCASE 'O': t=st.i[s--]; TOS=(long)fopen(&st.b[TOS], (t)?"wb":"rb");
            RCASE 'C': if (TOS) { fclose((FILE*)TOS); } s--;
            RCASE 'R': s++; TOS=0; if (NOS) fread((void*)&TOS, 1, 1, (FILE*)NOS);
            RCASE 'W': t=st.i[s--]; if (TOS) { fwrite((void*)&t, 1, 1, (FILE*)TOS); } } }
/*  {  */ void f123() { st.i[++l]=p; if (TOS==0) { while (BP!='}') { ++p; } } }
/*  |  */ void f124() { while (BP!='|') { st.b[TOS++]=BPP; } st.b[TOS++]=0; ++p; }
/*  }  */ void f125() { if (TOS) { p=st.i[l]; } else { --l; --s; } }
/*  ~  */ void f126() { TOS=TOS?0:-1; }
void fSTK() { for (int i=sb; i<=s; i++) { printf("%c%ld", (i==sb)?0:32, st.i[i]); } }
void (*q[127])()={ X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,N,f33,f34,f35,f36,f37,f38,
    f39,f40,N,f42,f43,f44,f45,f46,f47,n09,n09,n09,n09,n09,n09,n09,n09,n09,n09,f58,f59,f60,f61,f62,f63,f64,
    AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,AZ,f91,f92,f93,f94,f95,f96,
    az,az,f99,az,az,f102,az,az,az,az,az,az,az,az,az,az,az,az,az,az,az,az,az,az,az,az,f123,f124,f125,f126 };
void R(int x) { s=(s<sb)?(sb-1):s; r=rb; l=lb; p=x; while (p) { u=BPP; q[u](); } }
void L() { y=&st.b[h]; printf("\ns1:("); fSTK(); printf(")>"); fgets(y, 128, stdin); R(h); }
int main(int argc, char *argv[]) {
    s=sb-1; h=cb; u=SZ-1024; for (int i=0; i<(SZ/4); i++) { st.i[i]=0; }
    st.i[0]=h; st.i['a']=argc; for (int i=1; i < argc; ++i) {
        y=argv[i]; t=atoi(y);
        if ((t) || (y[0]=='0' && y[1]==0)) { st.i['a'+i]=t; }
        else { st.i['a'+i]=u; for (int j=0; y[j]; j++) { st.b[u++]=y[j]; } st.b[u++]=0; }
    }
    if ((argc>1) && (argv[1][0]!='-')) { FILE *fp=fopen(argv[1], "rb"); 
        if (fp) {while ((c=fgetc(fp))!=EOF) { st.b[h++]=(31<c)?c:32; } fclose(fp); st.i[0]=h; R(cb); }
    } while (1) { L(); } return 0;
}
