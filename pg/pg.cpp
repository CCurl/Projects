#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define NEXT goto next
#define PS(x)  stk[++sp]=(x)
#define PP     stk[sp--]
#define TOS    stk[sp]
#define NOS    stk[sp-1]
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
#define L3     lstk[lsp-3]
#define BTW(a,b,c) ((b<=a)&&(a<=c))

char u, *pc;
long stk[32], rstk[32], lstk[30], reg[256], sp, rsp, lsp, t;
long locs[100], lb, t1, t2, t3;

void run(const char *x) {
    pc = (char *)x;
    printf("(exec): %s\n", x);
    u = '?';
    next:

    switch(*(pc++)) {
    case ' ': NEXT;
    case '!': printf("%c",u); NEXT;
    case '"': printf("%c",u); NEXT;
    case '#': t=TOS; PS(t); NEXT;
    case '$': t=TOS; TOS=NOS; NOS=t; NEXT;
    case '%': t=NOS; PS(t); NEXT;
    case '&': printf("%c",u); NEXT;
    case '\'': D1; NEXT;
    case '(': if (PP==0) { while (*(pc++)!=')') {} } NEXT;
    case ')': NEXT;
    case '*': NOS*=TOS; D1; NEXT;
    case '+': if (*(pc) == '+') { ++TOS; ++pc; }
            else { NOS += TOS; D1; }
            NEXT;
    case ',': printf("%c",(char)PP); NEXT;
    case '-': if (*(pc) == '-') { --TOS; ++pc; }
            else { NOS -= TOS; D1; }
            NEXT;
    case '.': printf("%ld",PP); NEXT;
    case '/': NOS/=TOS; D1; NEXT;
    case '0': case '1': case '2': case '3': 
    case '4': case '5': case '6': case '7': 
    case '8': case '9': PS(*(pc-1)-'0');
        while (BTW(*pc,'0','9')) { TOS=(TOS*10)+(*(pc++)-'0'); }
        NEXT;
    case ':': printf("%c",u); NEXT;
    case ';': printf("%c",u); NEXT;
    case '<': NOS=(NOS<TOS)?-1:0;  D1; NEXT;
    case '=': NOS=(NOS==TOS)?-1:0; D1; NEXT;
    case '>': NOS=(NOS>TOS)?-1:0;  D1; NEXT;
    case '?': printf("%c",u); NEXT;
    case '@': printf("%c",u); NEXT;
    case 'A': printf("%c",u); NEXT;
    case 'B': printf(" "); NEXT;
    case 'C': printf("%c",u); NEXT;
    case 'D': printf("%c",u); NEXT;
    case 'E': printf("%c",u); NEXT;
    case 'F': printf("%c",u); NEXT;
    case 'G': printf("%c",u); NEXT;
    case 'H': printf("%c",u); NEXT;
    case 'I': PS(L0); NEXT;
    case 'J': PS(L3); NEXT;
    case 'K': printf("%c",u); NEXT;
    case 'L': t1=*(pc++); t2=*(pc++); t3=*(pc++);
        if (++reg[t1] < reg[t2]) pc = (char*)reg[t3];
        NEXT;
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
    case '_': TOS=-TOS; NEXT;
    case '`': printf("%c",u); NEXT;
    case 'a': printf("%c",u); NEXT;
    case 'b': printf(" "); NEXT;
    case 'c': printf("%c",u); NEXT;
    case 'd': --reg[*(pc++)]; NEXT;
    case 'e': printf("%c",u); NEXT;
    case 'f': printf("%c",u); NEXT;
    case 'g': printf("%c",u); NEXT;
    case 'h': PS(0); while (1) {
            t=BTW(*pc,'0','9') ? *pc-'0' : -1;
            if ((t<0) && BTW(*pc,'A','F')) { t=*pc-'A'+10; }
            if (t<0) { break; }
            TOS=(TOS*16)+t; ++pc;
        } NEXT;
    case 'i': ++reg[*(pc++)]; NEXT;
    case 'j': printf("%c",u); NEXT; 
    case 'k': printf("%c",u); NEXT;
    case 'l': u = *(pc++); if (u=='+') { lb+=(lb<90)?10:0; }
        else if (u=='-') { lb=(0<lb)?10:0; }
        NEXT;
    case 'm': printf("%c",u); NEXT;
    case 'n': printf("%c",u); NEXT;
    case 'o': printf("%c",u); NEXT;
    case 'p': printf("%c",u); NEXT;
    case 'q': printf("%c",u); NEXT;
    case 'r': PS(reg[*(pc++)]); NEXT;
    case 's': reg[*(pc++)] = PP; NEXT;
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
    sp = rsp = lsp = lb = 0;
    run("20 0[TsS 500000000 0[] TrS-.B]N");
    run("10 0[TsS 200000000{--#}\\TrS-.B]");
}
