// S2.c - based on Sandor Schneider's STABLE
#define _CRT_SECURE_NO_WARNINGS    // For Visual Studio
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define btw(a,b,c) ((b<=a) && (a<=c))
#define TOS st.i[s]
#define NOS st.i[s-1]
union flin { float f[2500]; int i[2500]; char b[10000]; }; static union flin st;
static char ex[80], u, a, k = 0;
static int c, m, r, cb = 7000, p, s=1, ro=64, rb=35, sb=1, t;
inline void push(int x) { st.i[++s] = x; } 
inline int pop() { return st.i[s--]; }
/* <33 */ void N() { p=(u==' ')?p:0; }
/*  !  */ void f33() { st.i[TOS] = NOS; s -= 2; }
/*  "  */ void f34() { while (st.b[p] != '"') { putc(st.b[p++], stdout); } ++p; }
/*  #  */ void f35() { st.i[s + 1] = TOS; ++s; }
/*  $  */ void f36() { t = TOS; TOS = NOS; NOS = t; }
/*  %  */ void f37() { push(NOS); }
/*  &  */ void f38() { t = TOS; TOS = NOS % t; NOS /= t; }
/*  '  */ void f39() { push(st.b[p++]); }
/*  (  */ void f40() { if (pop() == 0) { while (st.b[p++] != ')'); } }
/*  )  */ void f41() {}
/*  *  */ void f42() { NOS *= TOS; s--; }
/*  +  */ void f43() { NOS += TOS; s--; }
/*  ,  */ void f44() { putc(pop(), stdout); }
/*  -  */ void f45() { NOS -= TOS; s--; }
/*  .  */ void f46() { printf("%d", pop()); }
/*  /  */ void f47() { NOS /= TOS; s--; }
/* 0-9 */ void fN()  { push(u-'0'); while (btw(st.b[p],'0','9')) { TOS=(TOS*10)+(st.b[p++]-'0'); } }
/*  :  */ void f58() { u = st.b[p++]; if (btw(u, 'A', 'Z')) { st.i[u] = p; while (st.b[p++] != ';') {} m = p; } }
/*  ;  */ void f59() { p = st.i[r--]; if (r < rb) { r = rb; p = 0; } }
/*  <  */ void f60() { NOS = (NOS < TOS) ? 1 : 0; s--; }
/*  =  */ void f61() { NOS = (NOS == TOS) ? 1 : 0; s--; }
/*  >  */ void f62() { NOS = (NOS > TOS) ? 1 : 0; s--; }
/*  ?  */ void f63() {}
/*  @  */ void f64() { TOS = st.i[TOS]; }
/* A-Z */ void fF()  { st.i[++r] = p; p = st.i[u]; }
/*  [  */ void f91() { st.i[++r] = p; st.i[++r] = pop(); st.i[++r] = pop(); }
/*  \  */ void f92() { --s; }
/*  ]  */ void f93() { ++st.i[r]; if (st.i[r] <= st.i[r - 1]) { p = st.i[r - 2]; } else { r -= 3; } }
/*  ^  */ void f94() { p = st.i[r--]; }
/*  _  */ void f95() { TOS = -TOS; }
/*  `  */ void f96() { char *y=ex; while (st.b[p]!='`') { *(y++) = st.b[p++]; } *y=0; system(ex); ++p; }
/*  a  */ void f97() {}
/*  b  */ void f98() { putc(' ', stdout); }
/*  c  */ void f99() { u = st.b[p++];
              if (u == '@') { TOS=st.b[TOS]; }
              if (u == '!') { st.b[TOS]=NOS; s-=2; } }
/* d   */ void f100() { u = st.b[p++]; st.i[u+ro]--; }
/* e   */ void f101() {} 
/* f   */ void f102() { u = st.b[p++];
             if (u=='I') { st.f[s] = (float)st.i[s]; }
             else if (u=='O') { st.i[s] = (int)st.f[s]; }
             else if (u == '.') { printf("%f", st.f[s--]); }
             else if (u == '+') { st.f[s-1] += st.f[s]; s--; }
             else if (u == '-') { st.f[s-1] -= st.f[s]; s--; }
             else if (u == '*') { st.f[s-1] *= st.f[s]; s--; }
             else if (u == '/') { st.f[s-1] /= st.f[s]; s--; } }
/* g,h */ void f103() {} void f104() {}
/* i   */ void f105() { u=st.b[p++]; st.i[u+ro]++; }
/* j-m */ void f106() {} void f107() {} void f108() {} void f109() {} /* j-m */
/* n   */ void f110() { putc('\n',stdout); }
/* o-q */ void f111() {} void f112() {} void f113() {}
/* r   */ void f114() { u = st.b[p++]; push(st.i[u+ro]); }
/* s   */ void f115() { u = st.b[p++]; st.i[u+ro] = pop(); }
/* t   */ void f116() { push(GetTickCount()); }
/* u-w */ void f117() {} void f118() {} void f119() {}
/* x   */ void f120() { u = st.b[p++];
             if (u == 'U') { --r; }
             if (u == 'L') { --r; while (st.b[p++] != '}'); }
             if (u == 'I') { push(st.i[r]); }
             if (u == 'Q') { exit(0); } }
/* y,z */ void f121() {} void f122() {} /* y-z */
/*  {  */ void f123() { st.i[++r] = p; if (TOS == 0) { while (st.b[p] != '}') { ++p; } } }
/*  |  */ void f124() {}
/*  }  */ void f125() { if (TOS) { p = st.i[r]; } else { --r; --s; } }
/*  ~  */ void f126() {}
void (*q[127])() = { N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,
f33,f34,f35,f36,f37,f38,f39,f40,f41,f42,f43,f44,f45,f46,f47,fN,fN,fN,fN,fN,fN,fN,fN,fN,fN,
f58,f59,f60,f61,f62,f63,f64,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,
f91,f92,f93,f94,f95,f96,f97,f98,f99,f100,f101,f102,f103,f104,f105,f106,f107,f108,f109,f110,f111,f112,f113,
f114,f115,f116,f117,f118,f119,f120,f121,f122,f123,f124,f125,f126 };
void R(int x) { s=(s<sb)?sb:s; r=rb; p=x; while (cb<=p) { u=st.b[p++]; q[u](); } }
void H(char* s) { FILE *fp=fopen("h.txt", "at"); if (fp) { fprintf(fp, "%s", s); fclose(fp); } }
void L() { char *z = &st.b[m]; printf("\ns2>"); fgets(z, 128, stdin); H(z); R(m); }
void main(int argc, char *argv[]) {
    m=cb; for (int i = 0; i < 2500; i++) { st.i[i] = 0; }
    if (argc > 1) {
        FILE *fp = fopen(argv[1], "rt"); if (fp) {
            while ((c = fgetc(fp)) != EOF) { if (btw(c,32,126)) { st.b[m++]=c; } };
        fclose(fp); R(cb); } else { printf("file?"); }
    }
    while (1) { L(); };
}
