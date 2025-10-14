#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NEXT   goto next
#define NCASE  NEXT; case
#define RCASE  return; case
#define PS(x)  stk[++sp]=(x)
#define PP     stk[sp--]
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

char u, *pc;
long stk[64], rstk[64], lstk[30], sp, rsp, lsp, t;

void ext() {
    switch(*(pc++)) {
		case '1':
		RCASE '2':
			return;
		default: return;
	}
}

void run(const char *x) {
    pc = (char *)x;
    next:
    u = *(pc++);
	// printf("-%d(%c)-\n", u, u);
    switch(u) {
    NCASE ' ':
    NCASE '!': printf("%c",u);
    NCASE '"': printf("%c",u);
    NCASE '#': t=S0; PS(t);
    NCASE '$': t=S0; S0=S1; S1=t;
    NCASE '%': t=S1; PS(t);
    NCASE '&': printf("%c",u);
    NCASE '\'': D1;
    NCASE '(': if (PP==0) { while (*(pc++)!=')') {} }
    NCASE ')': printf("%c",u);
    NCASE '*': S1*=S0; D1;
    NCASE '+': S1+=S0; D1;
    NCASE ',': printf("%c",u);
    NCASE '-': S1-=S0; D1;
    NCASE '.': printf(" %ld",PP);
    NCASE '/': S1/=S0; D1;
    NCASE '0': case '1': case '2': case '3': 
    case '4': case '5': case '6': case '7': 
    case '8': case '9': PS(u-'0');
        while (BTW(*pc,'0','9')) { S0=(S0*10)+(*(pc++)-'0'); }
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
    NCASE 'N': printf("%c",10);
    NCASE 'O': printf("%c",u);
    NCASE 'P': printf("%c",u);
    NCASE 'Q': exit(0);
    NCASE 'R': printf("%c",u);
    NCASE 'S': printf("%c",u);
    NCASE 'T': PS(clock()/1000);
    NCASE 'U': printf("%c",u);
    NCASE 'V': printf("%c",u);
    NCASE 'W': printf("%c",u);
    NCASE 'X': printf("%c",u);
    NCASE 'Y': printf("%c",u);
    NCASE 'Z': printf("%c",u);
    NCASE '[': lsp+=2; L0=PP; L1=(long)pc;
    NCASE '\\': if (0<sp) sp--;
    NCASE ']': if (--L0) { pc=(char *)L1; } else { lsp-=2; }
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
    NCASE 'l': printf("%c",u);
    NCASE 'm': printf("%c",u);
    NCASE 'n': printf("%c",u);
    NCASE 'o': printf("%c",u);
    NCASE 'p': printf("%c",u);
    NCASE 'q': printf("%c",u);
    NCASE 'r': printf("%c",u);
    NCASE 's': printf("%c",u);
    NCASE 't': printf("%c",u);
    NCASE 'u': printf("%c",u);
    NCASE 'v': printf("%c",u);
    NCASE 'w': printf("%c",u);
    NCASE 'x': ext();
    NCASE 'y': printf("%c",u);
    NCASE 'z': printf("%c",u);
    NCASE '{': lsp+=2; L1=(long)pc;
    NCASE '|': printf("%c",u);
    NCASE '}': if (S0) { pc=(char*)L1; } else { lsp -=2; }
    NCASE '~': printf("%c",u);
    }
}

int main() {
    sp = rsp = lsp = 0;
    run("T1000 1000#**[]T$-.N");
    run("T1000 1000#**{d}\\T$-.N");
}
