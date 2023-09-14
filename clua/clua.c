#define _CRT_
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "heap.h"

#define NCASE goto next; case
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
#define IR     *(pc-1)
char *pc;
long stk[32], rstk[32], lstk[30], sp, rsp, lsp, t, mh;

typedef struct val_t {
    char type;
    int refCount;
    union {
        double numVal;
        char *strVal;
        struct kv_t *tblVal;
    } val;
} VAL_T;

typedef struct kv_t { char *key; VAL_T val; } KV_T;

enum {
    NUM, STR, TBL, ARY, FUN, NIL, TTT, FFF, // types
    ADD, SUB, MUL, DIV // operations
};

int memFind(char *x) {
    return 0;
}

char nextCh() {
    return 0;
}

void run(const char *x) {
    pc = (char *)x;
    next:

    switch (*(pc++)) {
    case ' ':
    NCASE '!': printf("%c",IR);
    NCASE '"': printf("%c",IR);
    NCASE '#': t=S0; PS(t);
    NCASE '$': t=S0; S0=S1; S1=t;
    NCASE '%': t=S1; PS(t);
    NCASE '&': printf("%c",IR);
    NCASE 39: D1;
    NCASE '(': if (PP==0) { while (*(pc++)!=')') {} }
    NCASE ')': printf("%c",IR);
    NCASE '*': S1*=S0; D1;
    NCASE '+': S1+=S0; D1;
    NCASE ',': printf("%c",IR);
    NCASE '-': S1-=S0; D1;
    NCASE '.': printf(" %ld",PP);
    NCASE '/': S1/=S0; D1;
    case '0': case '1': case '2': case '3': 
    case '4': case '5': case '6': case '7': 
    case '8': case '9': PS(IR-'0');
        while (BTW(*pc,'0','9')) { S0=(S0*10)+(*(pc++)-'0'); }
    NCASE ':': printf("%c",IR);
    NCASE ';': printf("%c",IR);
    NCASE '<': S1=(S1<S0)?-1:0;  D1;
    NCASE '=': S1=(S1==S0)?-1:0; D1;
    NCASE '>': S1=(S1>S0)?-1:0;  D1;
    NCASE '?': printf("%c",IR);
    NCASE '@': printf("%c",IR);
    NCASE 'A': printf("%c",IR);
    NCASE 'B': printf("%c",IR);
    NCASE 'C': printf("%c",IR);
    NCASE 'D': printf("%c",IR);
    NCASE 'E': printf("%c",IR);
    NCASE 'F': printf("%c",IR);
    NCASE 'G': printf("%c",IR);
    NCASE 'H': printf("%c",IR);
    NCASE 'I': PS(L0);
    NCASE 'J': printf("%c",IR);
    NCASE 'K': printf("%c",IR);
    NCASE 'L': printf("%c",IR);
    NCASE 'M': printf("%c",IR);
    NCASE 'N': printf("%c",10);
    NCASE 'O': printf("%c",IR);
    NCASE 'P': printf("%c",IR);
    NCASE 'Q': exit(0);
    NCASE 'R': printf("%c",IR);
    NCASE 'S': printf("%c",IR);
    NCASE 'T': PS(clock());
    NCASE 'U': printf("%c",IR);
    NCASE 'V': printf("%c",IR);
    NCASE 'W': printf("%c",IR);
    NCASE 'X': printf("%c",IR);
    NCASE 'Y': printf("%c",IR);
    NCASE 'Z': printf("%c",IR);
    NCASE '[': lsp+=3; L0=PP; L1=PP; L2=(long)pc;
    NCASE '\\': if (0<sp) sp--;
    NCASE ']': if (++L0<L1) { pc=(char *)L2; }
        else { lsp-=3; }
    NCASE '^': printf("%c",IR);
    NCASE '_': printf("%c",IR);
    NCASE '`': printf("%c",IR);
    NCASE 'a': printf("%c",IR);
    NCASE 'b': printf("%c",IR);
    NCASE 'c': printf("%c",IR);
    NCASE 'd': --S0;
    NCASE 'e': printf("%c",IR);
    NCASE 'f': printf("%c",IR);
    NCASE 'g': printf("%c",IR);
    NCASE 'h': printf("%c",IR);
    NCASE 'i': ++S0;
    NCASE 'j': printf("%c",IR); 
    NCASE 'k': printf("%c",IR);
    NCASE 'l': printf("%c",IR);
    NCASE 'm': printf("%c",IR);
    NCASE 'n': printf("%c",IR);
    NCASE 'o': printf("%c",IR);
    NCASE 'p': printf("%c",IR);
    NCASE 'q': printf("%c",IR);
    NCASE 'r': printf("%c",IR);
    NCASE 's': printf("%c",IR);
    NCASE 't': printf("%c",IR);
    NCASE 'u': printf("%c",IR);
    NCASE 'v': printf("%c",IR);
    NCASE 'w': printf("%c",IR);
    NCASE 'x': printf("%c",IR);
    NCASE 'y': printf("%c",IR);
    NCASE 'z': printf("%c",IR);
    NCASE '{': lsp+=3; L2=(long)pc;
    NCASE '|': printf("%c",IR);
    NCASE '}': if (PP) { pc=(char*)L2; } else { lsp -=3; }
    NCASE '~': printf("%c",IR);
    }
}

char buf[1000];

void memtest() {
    hm_init(buf, sizeof(buf), 10);
    char *x0 = hm_malloc(2000); printf("x0: %p\n", x0);
    char *x1 = hm_malloc(20); printf("x1: %p\n", x1);
    char *x2 = hm_malloc(100); printf("x2: %p\n", x2);
    hm_free(x1);
    char *x3 = hm_malloc(10); printf("x3: %p\n", x3);
    hm_free(x3);
    char *x4 = hm_malloc(20); printf("x4: %p\n", x4);
    char *x5 = hm_malloc(50); printf("x5: %p\n", x5);
    hm_free(x1);
    hm_free(x4);
    hm_free(x3);
    hm_free(x2);
    hm_free(x5);
    x5 = hm_malloc(200); printf("x5: %p\n", x5);
    hm_free(0);
    hm_init(NULL, 0, 0);
    printf("\n");
}

extern int tok_err;
extern int tokParseOne();
extern int tokParseLine(const char *line);
extern char tokMsg[];
extern const char *tok_input;
extern void tokDumpAll();

int main() {
    memtest();

    char line[256];
    FILE *fp;
    fopen_s(&fp, "code.ccc", "rt");
    // fopen_s(&fp, "clua.c", "rt");
    // fopen_s(&fp, "tokenizer.c", "rt");
    // fopen_s(&fp, "heap.c", "rt");
    // fopen_s(&fp, "../lua/life.lua", "rt");
    while (fp && (fgets(line, sizeof(line), fp) == line)) {
        printf("%s", line);
        if (tokParseLine(line)) { break; }
    }
    if (fp) { fclose(fp); }
    tokDumpAll();
    if (tok_err) {
        printf("ERROR: [%s] at [%s]", tokMsg, tok_input);
    } else {
        printf("no errors");
    }

    sp = rsp = lsp = 0;
    //run("T500 1000#**0[]T$-.N");
    //run("T500 1000#**{d#}\\T$-.N");
}
