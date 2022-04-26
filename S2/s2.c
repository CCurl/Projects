// S2.c, based on Sandor Schneider's STABLE
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define btw(a,b,c) ((b<=a) && (a<=c))
#define TOS st.i[s]
#define NOS st.i[s-1]
union flin { float f[2500]; int i[2500]; char b[10000]; }; static union flin st;
static char ex[80], t = 0, u, a, k = 0;
static int x = 0, i, c, v, m, r, p, n, s, f;
inline void push(int x) { st.i[++s] = x; } 
inline int pop() { return st.i[s--]; }
void N() {} // NOP
void f33() /* ! */ { st.i[TOS] = NOS; s -= 2; }
void f34() /* " */ { while (st.b[p] != '"') { putc(st.b[p++], stdout); } ++p; }
void f35() /* # */ { st.i[s + 1] = TOS; ++s; }
void f36() /* $ */ { t = TOS; TOS = NOS; NOS = t; }
void f37() /* % */ { push(NOS); }
void f38() /* & */ { t = TOS; TOS = NOS % t; NOS /= t; }
void f39() /* ' */ { push(st.b[p++]); }
void f40() /* ( */ { if (pop() == 0) { while (st.b[p++] != ')'); } }
void f41() /* ) */ {}
void f42() /* * */ { NOS *= TOS; s--; }
void f43() /* + */ { NOS += TOS; s--; }
void f44() /* , */ { putc(pop(),stdout); }
void f45() /* - */ { NOS -= TOS; s--; }
void f46() /* . */ { printf("%d", pop()); }
void f47() /* / */ { NOS /= TOS; s--; }
void fN() /* 0-9 */ { push(u-'0'); while (btw(st.b[p],'0','9')) { TOS=(TOS*10)+(st.b[p++]-'0'); } }
void f58() /* : */ { u=st.b[p++]; st.i[u]=p; while (st.b[p++]!=';') {} }
void f59() /* ; */ { p=st.i[r--]; }
void f60() /* < */ { NOS=(NOS<TOS)?1:0; s--; }
void f61() /* = */ { NOS=(NOS==TOS)?1:0; s--; }
void f62() /* > */ { NOS=(NOS>TOS)?1:0; s--; }
void f63() /* ? */ {}
void f64() /* @ */ { TOS = st.i[TOS]; }
void fF() /* A-Z */ { st.i[++r]=p; p=st.i[u]; }
void f91() /* [ */ { st.i[++r]=p; st.i[++r]=pop(); st.i[++r]=pop(); }
void f92() /* \ */ { --s; }
void f93() /* ] */ { ++st.i[r]; if (st.i[r]<=st.i[r-1]) { p=st.i[r-2]; } else { r-=3; } }
void f94() /* ^ */ { p = st.i[r--]; }
void f95() /* _ */ { TOS=-TOS; }
void f96() /* ` */ { char *y=ex; while (st.b[p]!='`') { *(y++)=st.b[p++]; } *y=0; system(ex); ++p; }
void f97() /* a */ {}
void f98() /* b */ { putc(' ', stdout); }
void f99() /* c */ { u = st.b[p++];
    if (u == '@') { TOS=st.b[TOS]; }
    if (u == '!') { st.b[TOS]=NOS; s-=2; } }
void f100() /* d */ { u=st.b[p++]; st.i[u+32]--; }
/* e-h */ void f101() {} void f102() {} void f103() {} void f104() {} /* e-h */
void f105() /* i */ { u=st.b[p++]; st.i[u+32]++; }
/* j-m */ void f106() {} void f107() {} void f108() {} void f109() {} /* j-m */
void f110() /* n */ { putc('\n',stdout); }
/* o-q */ void f111() {} void f112() {} void f113() {}
void f114() /* r */ { u = st.b[p++]; push(st.i[u+32]); }
void f115() /* s */ { u = st.b[p++]; st.i[u+32] = pop(); }
/* t-w */ void f116() {} void f117() {} void f118() {} void f119() {} /* t-w */
void f120() /* x */ { u = st.b[p++];
    if (u == 'U') { --r; }
    if (u == 'L') { --r; while (st.b[p++] != '}'); }
    if (u == 'I') { push(st.i[r]); }
    if (u == 'Q') { exit(0); }
}
/* y-z */ void f121() {} void f122() {} /* y-z */
void f123() /* { */ { st.i[++r] = p; if (TOS == 0) { while (st.b[p] != '}') { ++p; } } }
void f124() /* | */ {}
void f125() /* } */ { if (TOS) { p = st.i[r]; } else { --r; --s; } }
void f126() /* ~ */ {}
void (*q[127])() = { N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,
f33,f34,f35,f36,f37,f38,f39,f40,f41,f42,f43,f44,f45,f46,f47,fN,fN,fN,fN,fN,fN,fN,fN,fN,fN,
f58,f59,f60,f61,f62,f63,f64,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,
f91,f92,f93,f94,f95,f96,f97,f98,f99,f100,f101,f102,f103,f104,f105,f106,f107,f108,f109,f110,f111,f112,f113,
f114,f115,f116,f117,f118,f119,f120,f121,f122,f123,f124,f125,f126, };
void run(int sb) { s=(s<1)?0:s; p=sb; r=35; while ((st.b[p]) && (p >= sb)) { u = st.b[p++]; q[u](); } }
void loop() { printf(" s2\n"); fgets(&st.b[8000], 1000, stdin); run(8000); }
int main(int argc, char* argv[]) {
    for (i = 0; i < 2400; i++) { st.i[i] = 0; }
    if (argc > 1) {
        FILE* fp = fopen(argv[1], "rt"); if (!fp) { printf("file?"); exit(0); }
        p = 8000; while ((c = fgetc(fp)) != EOF) { if (btw(c,32,126)) { st.b[p++]=c; } };
        fclose(fp); run(8000);
    }
    while (1) { loop(); } return 0;
}
