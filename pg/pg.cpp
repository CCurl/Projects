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

#define WDSZ        sizeof(long)
#define OPIR(x)     x.ir[3]
#define OPARG(x)    (x.wd&0x00ffffff)
#define u           OPIR(code[pc-1])

typedef union {
    unsigned long wd;
    unsigned char ir[WDSZ];
} op_t;

typedef struct {
    unsigned long addr;
    char attr[WDSZ];
    char name[16];
} dict_t;

unsigned long ir;
long stk[32], rstk[32], lstk[30], sp, rsp, lsp, t, here;
op_t code[0x00FFFFFF];

void run(long pc) {
    next:

    switch(OPIR(code[pc++])) {
        case 0: return;
        case 1: PS(OPARG(code[pc-1])); NEXT;
        case ' ': NEXT;
        case '!': printf("%c",u); NEXT;
        case '"': printf("%c",u); NEXT;
        case '#': t=S0; PS(t); NEXT;
        case '$': t=S0; S0=S1; S1=t; NEXT;
        case '%': t=S1; PS(t); NEXT;
        case '&': printf("%c",u); NEXT;
        case '\'': D1; NEXT;
        case '(': printf("%c",u); NEXT;
        case ')': printf("%c",u); NEXT;
        case '*': S1*=S0; D1; NEXT;
        case '+': S1+=S0; D1; NEXT;
        case ',': printf("%c",u); NEXT;
        case '-': S1-=S0; D1; NEXT;
        case '.': printf(" %ld",PP); NEXT;
        case '/': S1/=S0; D1; NEXT;
        case '0': case '1': case '2': case '3': 
        case '4': case '5': case '6': case '7': 
        case '8': case '9': printf("%c",u); NEXT;
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
        case 'N': printf("\n"); NEXT;
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
        case '[': lsp+=3; L0=PP; L1=PP; L2=pc; NEXT;
        case '\\': if (0<sp) sp--; NEXT;
        case ']': if (++L0<L1) { pc=L2; }
                else { lsp-=3; } NEXT;
        case '^': printf("%c",u); NEXT;
        case '_': printf("%c",u); NEXT;
        case '`': printf("%c",u); NEXT;
        case 'a': printf("%c",u); NEXT;
        case 'b': printf("%c",u); NEXT;
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
        case '}': if (PP) { pc=L2; } else { lsp -=3; } NEXT;
        case '~': printf("%c",u); NEXT;
    }
}

long compile(char op, long arg) {
    code[here].wd = arg;
    OPIR(code[here++]) = op;
    return here-1;
}

long parse(const char *src) {
    const char *cp = src;
    long st = here;
    while (*cp) {
        if (*cp < 33) { ++cp; continue; }
        if (BTW(*cp,'0','9')) {
            long num = 0;
            while BTW(*cp,'0','9') { num = (num*10) + *(cp++) - '0'; }
            compile(1, num);
            continue;
        }
        compile(*(cp++), 0);
    }
    compile(0,0);
    return st;
}

int main() {
    sp = rsp = lsp = 0;
    here = 0;
    run(parse("T500 1000#**0[]T$-.N"));
    here = 0;
    run(parse("T500 1000#**{d#}\\T$-.N"));
}
