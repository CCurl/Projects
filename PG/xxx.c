#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define btw(a,b,c) ((b<=a) && (a<=c))
#define TOS st.i[s]
#define NOS st.i[s-1]
#define SZ 65536
union fib { float f[SZ/4]; int i[SZ/4]; char b[SZ]; };
static union fib st;
static int cb, h, sb, s, rb, r, p, u, t;
/*   */ void X() { ; }
/*   */ void f32() { printf("[%c]",u); }
/* ! */ void f33() { printf("[%c]",u); }
/* " */ void f34() { printf("[%c]",u); }
/* # */ void f35() { printf("[%c]",u); }
/* $ */ void f36() { printf("[%c]",u); }
/* % */ void f37() { printf("[%c]",u); }
/* & */ void f38() { printf("[%c]",u); }
/* ' */ void f39() { printf("[%c]",u); }
/* ( */ void f40() { printf("[%c]",u); }
/* ) */ void f41() { printf("[%c]",u); }
/* * */ void f42() { printf("[%c]",u); }
/* + */ void f43() { printf("[%c]",u); }
/* , */ void f44() { printf("[%c]",u); }
/* - */ void f45() { printf("[%c]",u); }
/* . */ void f46() { printf("[%c]",u); }
/* / */ void f47() { printf("[%c]",u); }
/* 0 */ void f48() { printf("[%c]",u); }
/* 1 */ void f49() { printf("[%c]",u); }
/* 2 */ void f50() { printf("[%c]",u); }
/* 3 */ void f51() { printf("[%c]",u); }
/* 4 */ void f52() { printf("[%c]",u); }
/* 5 */ void f53() { printf("[%c]",u); }
/* 6 */ void f54() { printf("[%c]",u); }
/* 7 */ void f55() { printf("[%c]",u); }
/* 8 */ void f56() { printf("[%c]",u); }
/* 9 */ void f57() { printf("[%c]",u); }
/* : */ void f58() { printf("[%c]",u); }
/* ; */ void f59() { printf("[%c]",u); }
/* < */ void f60() { printf("[%c]",u); }
/* = */ void f61() { printf("[%c]",u); }
/* > */ void f62() { printf("[%c]",u); }
/* ? */ void f63() { printf("[%c]",u); }
/* @ */ void f64() { printf("[%c]",u); }
/* A */ void f65() { printf("[%c]",u); }
/* B */ void f66() { printf("[%c]",u); }
/* C */ void f67() { printf("[%c]",u); }
/* D */ void f68() { printf("[%c]",u); }
/* E */ void f69() { printf("[%c]",u); }
/* F */ void f70() { printf("[%c]",u); }
/* G */ void f71() { printf("[%c]",u); }
/* H */ void f72() { printf("[%c]",u); }
/* I */ void f73() { printf("[%c]",u); }
/* J */ void f74() { printf("[%c]",u); }
/* K */ void f75() { printf("[%c]",u); }
/* L */ void f76() { printf("[%c]",u); }
/* M */ void f77() { printf("[%c]",u); }
/* N */ void f78() { printf("[%c]",u); }
/* O */ void f79() { printf("[%c]",u); }
/* P */ void f80() { printf("[%c]",u); }
/* Q */ void f81() { printf("[%c]",u); }
/* R */ void f82() { printf("[%c]",u); }
/* S */ void f83() { printf("[%c]",u); }
/* T */ void f84() { printf("[%c]",u); }
/* U */ void f85() { printf("[%c]",u); }
/* V */ void f86() { printf("[%c]",u); }
/* W */ void f87() { printf("[%c]",u); }
/* X */ void f88() { printf("[%c]",u); }
/* Y */ void f89() { printf("[%c]",u); }
/* Z */ void f90() { printf("[%c]",u); }
/* [ */ void f91() { printf("[%c]",u); }
/* \ */ void f92() { printf("[%c]",u); }
/* ] */ void f93() { printf("[%c]",u); }
/* ^ */ void f94() { printf("[%c]",u); }
/* _ */ void f95() { printf("[%c]",u); }
/* ` */ void f96() { printf("[%c]",u); }
/* a */ void f97() { printf("[%c]",u); }
/* b */ void f98() { printf("[%c]",u); }
/* c */ void f99() { printf("[%c]",u); }
/* d */ void f100() { printf("[%c]",u); }
/* e */ void f101() { printf("[%c]",u); }
/* f */ void f102() { printf("[%c]",u); }
/* g */ void f103() { printf("[%c]",u); }
/* h */ void f104() { printf("[%c]",u); }
/* i */ void f105() { printf("[%c]",u); }
/* j */ void f106() { printf("[%c]",u); }
/* k */ void f107() { printf("[%c]",u); }
/* l */ void f108() { printf("[%c]",u); }
/* m */ void f109() { printf("[%c]",u); }
/* n */ void f110() { printf("[%c]",u); }
/* o */ void f111() { printf("[%c]",u); }
/* p */ void f112() { printf("[%c]",u); }
/* q */ void f113() { printf("[%c]",u); }
/* r */ void f114() { printf("[%c]",u); }
/* s */ void f115() { printf("[%c]",u); }
/* t */ void f116() { printf("[%c]",u); }
/* u */ void f117() { printf("[%c]",u); }
/* v */ void f118() { printf("[%c]",u); }
/* w */ void f119() { printf("[%c]",u); }
/* x */ void f120() { printf("[%c]",u); }
/* y */ void f121() { printf("[%c]",u); }
/* z */ void f122() { printf("[%c]",u); }
/* { */ void f123() { printf("[%c]",u); }
/* | */ void f124() { printf("[%c]",u); }
/* } */ void f125() { printf("[%c]",u); }
/* ~ */ void f126() { printf("[%c]",u); }
void (*q[127])()={X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,
    f33,f34,f35,f36,f37,f38,f39,f40,f41,f42,f43,f44,f45,f46,f47,f48,f49,f50,f51,f52,
    f53,f54,f55,f56,f57,f58,f59,f60,f61,f62,f63,f64,f65,f66,f67,f68,f69,f70,f71,f72,f73,
    f74,f75,f76,f77,f78,f79,f80,f81,f82,f83,f84,f85,f86,f87,f88,f89,f90,f91,f92,f93,f94,
    f95,f96,f97,f98,f99,f100,f101,f102,f103,f104,f105,f106,f107,f108,f109,f110,f111,f112,
    f113,f114,f115,f116,f117,f118,f119,f120,f121,f122,f123,f124,f125,f126,X};
void I(() { sb=4; r=rb=64; for (t=0; t<(SZ/4); t++) { st.i[t]=0; } }
void R(int x) { s=(s<sb)?(sb-1):s; r=rb; p=x; while (p) { u=st.b[p++]; q[u](); } }
