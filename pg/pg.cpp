#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NEXT goto next
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
long stk[32], rstk[32], lstk[30], sp, rsp, lsp, t;

void run(const char *x) {
    pc = (char *)x;
    printf("(exec): %s\n", x);
    next:
    u = *(pc++);

    switch(u) {
    case ' ': NEXT;
    case '!': printf("%c",u); NEXT;
    case '"': printf("%c",u); NEXT;
    case '#': t=S0; PS(t); NEXT;
    case '$': t=S0; S0=S1; S1=t; NEXT;
    case '%': t=S1; PS(t); NEXT;
    case '&': printf("%c",u); NEXT;
    case '\'': D1; NEXT;
    case '(': if (PP==0) { while (*(pc++)!=')') {} } NEXT;
    case ')': printf("%c",u); NEXT;
    case '*': S1*=S0; D1; NEXT;
    case '+': S1+=S0; D1; NEXT;
    case ',': printf("%c",u); NEXT;
    case '-': S1-=S0; D1; NEXT;
    case '.': printf("%ld",PP); NEXT;
    case '/': S1/=S0; D1; NEXT;
    case '0': case '1': case '2': case '3': 
    case '4': case '5': case '6': case '7': 
    case '8': case '9': PS(u-'0');
        while (BTW(*pc,'0','9')) { S0=(S0*10)+(*(pc++)-'0'); }
        NEXT;
    case ':': printf("%c",u); NEXT;
    case ';': printf("%c",u); NEXT;
    case '<': S1=(S1<S0)?-1:0;  D1; NEXT;
    case '=': S1=(S1==S0)?-1:0; D1; NEXT;
    case '>': S1=(S1>S0)?-1:0;  D1; NEXT;
    case '?': printf("%c",u); NEXT;
    case '@': printf("%c",u); NEXT;
    case 'A': printf("%c",u); NEXT;
    case 'B': printf("%c",u); NEXT;
    case 'C': printf("%c",u); NEXT;
    case 'D': printf("%c",u); NEXT;
    case 'E': printf("%c",u); NEXT;
    case 'F': printf("%c",u); NEXT;
    case 'G': printf("%c",u); NEXT;
    case 'H': printf("%c",u); NEXT;
    case 'I': PS(L0); NEXT;
    case 'J': printf("%c",u); NEXT;
    case 'K': printf("%c",u); NEXT;
    case 'L': printf("%c",u); NEXT;
    case 'M': printf("%c",u); NEXT;
    case 'N': printf("%c",10); NEXT;
    case 'O': printf("%c",u); NEXT;
    case 'P': printf("%c",u); NEXT;
    case 'Q': exit(0); NEXT;
    case 'R': printf("%c",u); NEXT;
    case 'S': printf("%c",u); NEXT;
    case 'T': PS(clock()); NEXT;
    case 'U': printf("%c",u); NEXT;
    case 'V': printf("%c",u); NEXT;
    case 'W': printf("%c",u); NEXT;
    case 'X': printf("%c",u); NEXT;
    case 'Y': printf("%c",u); NEXT;
    case 'Z': printf("%c",u); NEXT;
    case '[': lsp+=3; L0=PP; L1=PP; L2=(long)pc; NEXT;
    case '\\': if (0<sp) sp--; NEXT;
    case ']': if (++L0<L1) { pc=(char *)L2; }
        else { lsp-=3; } NEXT;
    case '^': printf("%c",u); NEXT;
    case '_': printf("%c",u); NEXT;
    case '`': printf("%c",u); NEXT;
    case 'a': printf("%c",u); NEXT;
    case 'b': printf(" "); NEXT;
    case 'c': printf("%c",u); NEXT;
    case 'd': --S0; NEXT;
    case 'e': printf("%c",u); NEXT;
    case 'f': printf("%c",u); NEXT;
    case 'g': printf("%c",u); NEXT;
    case 'h': printf("%c",u); NEXT;
    case 'i': ++S0; NEXT;
    case 'j': printf("%c",u); NEXT; 
    case 'k': printf("%c",u); NEXT;
    case 'l': printf("%c",u); NEXT;
    case 'm': printf("%c",u); NEXT;
    case 'n': printf("%c",u); NEXT;
    case 'o': printf("%c",u); NEXT;
    case 'p': printf("%c",u); NEXT;
    case 'q': printf("%c",u); NEXT;
    case 'r': printf("%c",u); NEXT;
    case 's': printf("%c",u); NEXT;
    case 't': printf("%c",u); NEXT;
    case 'u': printf("%c",u); NEXT;
    case 'v': printf("%c",u); NEXT;
    case 'w': printf("%c",u); NEXT;
    case 'x': printf("%c",u); NEXT;
    case 'y': printf("%c",u); NEXT;
    case 'z': printf("%c",u); NEXT;
    case '{': lsp+=3; L2=(long)pc; NEXT;
    case '|': printf("%c",u); NEXT;
    case '}': if (PP) { pc=(char*)L2; } else { lsp -=3; } NEXT;
    case '~': printf("%c",u); NEXT;
    }
}

int main() {
    sp = rsp = lsp = 0;
    run("20 0[T500 1000#**0[]T$-.b]N");
    run("20 0[T200 1000#**{d#}\\T$-.b]");
}
