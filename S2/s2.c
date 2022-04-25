#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> /* stable.c STABLE minimalist FORTH like VM */
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define btw(a,b,c) ((b<=a) && (a<=c))
union flin { float f[2500]; int i[2500]; char b[10000]; }; static union flin st;
static char ex[80], t = 0, u, a, k = 0;
static int x = 0, i, c, v, m, r, p, n, mx, s, f;
#define push(x) st.i[++s]=x
#define pop() st.i[s--]
#define TOS st.i[s]
#define NOS st.i[s-1]
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
void f58() /* : */ { u=st.b[p++]; st.i[f+u-'A']=p; while (st.b[p++]!=';') {} }
void f59() /* ; */ { p=st.i[r--]; }
void f60() /* < */ { NOS=(NOS<TOS)?1:0; s--; }
void f61() /* = */ { NOS=(NOS==TOS)?1:0; s--; }
void f62() /* > */ { NOS=(NOS>TOS)?1:0; s--; }
void f63() /* ? */ {}
void f64() /* @ */ {}
void fF() /* A-Z */ { st.i[++r]=p; p=st.i[f+u-'A']; }
void f91() /* [ */ { st.i[++r]=p; st.i[++r]=pop(); st.i[++r]=pop(); }
void f92() /* \ */ { --s; }
void f93() /* ] */ { ++st.i[r]; if (st.i[r]<=st.i[r-1]) { p=st.i[r-2]; } else { r-=3; } }
void f94() /* ^ */ { p = st.i[r--]; }
void f95() /* _ */ { TOS=-TOS; }
void f96() /* ` */ {}
void f97() /* a */ {}
void f98() /* b */ { putc(' ', stdout); }
void f99() /* c */ { u = st.b[p++];
    if (u == '@') { TOS=st.b[TOS]; }
    if (u == '!') { st.b[TOS]=NOS; s-=2; } }
void f100() /* d */ { u=st.b[p++]-'A'; st.i[u]--; }
void f101() /* e */ {}
void f102() /* f */ {}
void f103() /* g */ {}
void f104() /* h */ {}
void f105() /* i */ { u=st.b[p++]-'A'; st.i[u]++; }
void f106() /* j */ {}
void f107() /* k */ {}
void f108() /* l */ {}
void f109() /* m */ {}
void f110() /* n */ { putc('\n',stdout); }
void f111() /* o */ {}
void f112() /* p */ {}
void f113() /* q */ {}
void f114() /* r */ { u = st.b[p++] - 'A'; push(st.i[u]); }
void f115() /* s */ { u = st.b[p++] - 'A'; st.i[u] = pop(); }
void f116() /* t */ {}
void f117() /* u */ {}
void f118() /* v */ {}
void f119() /* w */ {}
void f120() /* x */ { u = st.b[p++];
    if (u == 'U') { --r; }
    if (u == 'L') { --r; while (st.b[p++] != '}'); }
    if (u == 'I') { push(st.i[r]); }
}
void f121() /* y */ {}
void f122() /* z */ {}
void f123() /* { */ { st.i[++r] = p; if (TOS == 0) { while (st.b[p] != '}') { ++p; } } }
void f124() /* | */ {}
void f125() /* } */ { if (TOS) { p = st.i[r]; } else { --r; --s; } }
void f126() /* ~ */ {}
void (*q[127])() = { N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,N,
f33,f34,f35,f36,f37,f38,f39,f40,f41,f42,f43,f44,f45,f46,f47,fN,fN,fN,fN,fN,fN,fN,fN,fN,fN,
f58,f59,f60,f61,f62,f63,f64,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,fF,
f91,f92,f93,f94,f95,f96,f97,f98,f99,f100,f101,f102,f103,f104,f105,f106,f107,f108,f109,f110,f111,f112,f113,
f114,f115,f116,f117,f118,f119,f120,f121,f122,f123,f124,f125,f126, };
int main(int argc, char* argv[]) {
    FILE* be;  for (i = 0; i < 2400; i++) { st.i[i] = 0; }
    if (strcmp(argv[1], "-") == 0) {
        p = 8000; while ((c = getc(stdin)) != EOF) { if (btw(c,32,126)) { st.b[p++]=c; } } mx = p;
    } else {
        if (!(be = fopen(argv[1], "r"))) { printf("file?\n"); return 0; }
        p = 8000; while ((c = fgetc(be)) != EOF) if (btw(c,32,126)) { st.b[p++]=c; } fclose(be); mx = p;
    }
    for (i = 0; i < argc; i++) if (i > 1) st.i[i + 95] = atoi(argv[i]);
    p = 8000; s = 140; f = 25; r = 55;
    while (p <= mx) { u = st.b[p++]; q[u]();} return 0;
}
