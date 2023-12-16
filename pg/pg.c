#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CODE_SZ       0x7FFF
#define VARS_SZ       0xFFFF
#define DICT_SZ         8192
#define LPRIM            SYS
#define STK_SZ            64
#define btwi(a,b,c)   ((b<=a) && (a<=c))
#define comma(x)      code[here++]=(x)
#define NCASE         goto next; case
#define RCASE         return; case
#define CheckSP       if (sp<0) sp=0
#define RetIf0        if (*(++w)==0) return 0

#define here          code[0]
#define last          code[1]
#define base          code[2]
#define state         code[3]
#define TOS           stk[sp].i
#define NOS           stk[sp-1].i
#define PUSH(x)       stk[++sp].i=(x)
#define POP()         stk[sp--].i
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]

typedef long cell;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef union { double f; cell i; } SE_T;
typedef struct { ushort xt; byte fl; byte ln; char nm[28]; } DE_T;

SE_T stk[STK_SZ], rstk[STK_SZ];
DE_T dict[DICT_SZ];
static int pc, sp, rsp, lsp, lstk[60];
static char tib[128];
static ushort wc, code[CODE_SZ+1];

enum {
    STOP, LIT1, LIT2, EXIT, DUP, SWAP, DROP, FOR, INDEX, NEXT,   //  0 ->  9
    AND, OR, XOR, ADD, SUB, MUL, DIV, LT, EQ, GT,                // 10 -> 19
    CLK, OVER, WDS, JMP, JMPZ, JMPNZ, EMIT, DOT, COLON, SEMI,    // 20 -> 29
    IMM, FET, STO, SYS                                           // 30 -> 33
};

DE_T *addWord(const char *wd);
cell fetch2(ushort *a) { return *(cell*)(a); }
void makeImm() { dict[last-1].fl=1; }
void addPrim(const char *wd, ushort prim) { addWord(wd)->xt = prim; }

void words() {
    int n=0, t=0;
    for (int e=last-1; 0<=e; e--) {
        DE_T *de = (DE_T*)&dict[e];
        printf("%s(%d,%d)\t", de->nm, (int)de->xt&0x7FF, (int)de->fl); ++n; if (n%10==0) printf("\n");
    }
}

void execSys() {
    // printf("-sys:%d-",(int)TOS);
    switch (POP()) {
        case  0: exit(0);
        RCASE 1: addWord(0); state=1;
        RCASE 2: if (LPRIM<code[here-1]) { code[here-1] &= 0x07FF; }
                 else { comma(EXIT); }
                 state = 0;
        RCASE 3: makeImm();
    }
}

void Exec(int start) {
    cell t;
	pc = start;
    next:
    wc = code[pc++];
    switch(wc) {
    case  STOP:  return;
    NCASE LIT1:  PUSH(code[pc++]);
    NCASE LIT2:  PUSH(fetch2(&code[pc])); pc += sizeof(cell)/2;
    NCASE EXIT:  if (rsp) { pc = rstk[rsp--].i; } else { return; }
    NCASE DUP:   t=TOS; PUSH(t);
    NCASE SWAP : t=TOS; TOS=NOS; NOS=t;
    NCASE DROP : --sp; CheckSP;
    NCASE FOR:   lsp+=3; L2=pc; L1=POP(); L0=0;
    NCASE INDEX: PUSH(L0);
    NCASE NEXT:  if (++L0<L1) { pc=L2; } else { lsp-=3; } if (lsp<0) lsp=0;
    NCASE AND:   t = POP(); TOS &= t;
    NCASE OR:    t = POP(); TOS |= t;
    NCASE XOR:   t = POP(); TOS ^= t;
    NCASE ADD:   t = POP(); TOS += t;
    NCASE SUB:   t = POP(); TOS -= t;
    NCASE MUL:   t = POP(); TOS *= t;
    NCASE DIV:   t = POP(); TOS /= t;
    NCASE LT:    t = POP(); TOS = (TOS < t);
    NCASE EQ:    t = POP(); TOS = (TOS == t);
    NCASE GT:    t = POP(); TOS = (TOS > t);
    NCASE CLK:   PUSH(clock());
    NCASE OVER : t = NOS; PUSH(t);
    NCASE WDS:   words();
    NCASE JMP:   pc=code[pc];
    NCASE JMPZ:  if (POP()==0) { pc=code[pc]; } else { ++pc; }
    NCASE JMPNZ: if (POP()) { pc=code[pc]; } else { ++pc; }
    NCASE EMIT:  printf("%c", (char)POP());
    NCASE DOT:   printf("%zd", (size_t)POP()); CheckSP;
    NCASE COLON: // addWord(0); state = 1;
    NCASE SEMI:  //if (LPRIM<code[here-1]) { code[here-1] &= 0x07FF; }
                 //else { comma(EXIT); } state = 0;
    NCASE IMM:   // makeImm();
    NCASE FET:   TOS = code[TOS];
    NCASE STO:   code[TOS] = (short)NOS; sp-=2; CheckSP;
    NCASE SYS:   execSys();
    default: {
            if (wc & 0x8000) { rstk[++rsp].i = pc; }
            pc = (wc & 0x7FFF );
            goto next;
        }
    }
}

char wd[32], *toIn;
int nextWord() {
    int l = 0;
    while (*toIn && (*toIn<33)) { ++toIn; }
    while (*toIn && (*toIn>32)) { wd[l++] = *(toIn++); }
    wd[l] = 0;
    return l;
}

int lower(char c) { return btwi(c,'A','Z') ? c+32 : c; }
int strLen(const char *s) { int l=0; while (s[l]) { l++; } return l; }
int strEqI(const char *s, const char *d) {
    while (lower(*s) == lower(*d)) { if (*s==0) { return 1; } s++; d++; }
    return 0;
}


DE_T *addWord(const char *w) {
    if (!w) { nextWord(); w=wd; }
    int l = strLen(w);
    if (l==0) return (DE_T*)0;
    if (l>27) { l=27; }
    DE_T *de=(DE_T*)&dict[last++];
    de->xt = here | 0x8000;
    de->fl = 0;
    de->ln = l;
    for (int i = 0; i < l; i++) { de->nm[i] = w[i]; }
    de->nm[l]=0;
    // printf("-wd:%s,%d (%d)-", de->nm, here, de->xt);
    return de;
}

DE_T *findWord(const char *w) {
    if (!w) { nextWord(); w=wd; }
    int l = strLen(w);
    for (int e=last-1; 0<=e; e--) {
        DE_T *de = (DE_T*)&dict[e];
        if ((l==de->ln) && strEqI(de->nm, w)) { return de; }
    }

    return (DE_T*)0;
}

int isNum(const char *w, int b) {
    cell n=0, isNeg=0;
    if ((w[0]==39) && (w[2]==0)) { PUSH(w[1]); return 1; }
    if (w[0]=='%') { b=2; RetIf0; }
    if (w[0]=='#') { b=10; RetIf0; }
    if (w[0]=='$') { b=16; RetIf0; }
    if ((b==10) && (w[0]=='-')) { isNeg=1; RetIf0; }
    char c = *(w++);
    while (c) {
        n = (n*b);
        if ((b==2) && btwi(c,'0','1')) { n+=(c-'0'); }
        else if ((b>9)   && btwi(c,'0','9')) { n+=(c-'0'); }
        else if ((b==16) && btwi(c,'A','F')) { n+=(c-'A'+10); }
        else if ((b==16) && btwi(c,'a','f')) { n+=(c-'a'+10); }
        else return 0;
        c = *(w++);
    }
    if (isNeg) { n = -n; }
    PUSH(n);
    return 1;
}

int parseWord(char *w) {
    if (!w) { w = &wd[0]; }
    // printf("-pw:%s-",w);

    if (isNum(wd, base)) {
        long n = POP();
        if (btwi(n, 0, 0x7fff)) {
            comma(LIT1); comma((short)n);
        } else {
            comma(LIT2); comma(n & 0xffff); comma(n >> 16);
        }
        return 1;
    }

    DE_T *de = findWord(w);
    if (de) {
        if (de->fl == 1) {   // IMMEDIATE
            int h = here+100;
            code[h]=de->xt;
            code[h+1] = EXIT;
            Exec(h);
        } else {
            comma(de->xt);
        }
        return 1;
     }

    return 0;
}

int parseLine(const char *ln) {
    int h=here, l=last, s=state;
    toIn = (char *)ln;
    // printf("-pl:%s-",ln);
    while (nextWord()) {
        if (!parseWord(wd)) {
            printf("-%s?-", wd);
            here=h;
            last=l;
            state=0;
            return 0;
        }
    }
    if ((l==last) && (h<here) && (state==0) && (s==0)) {
        comma(0);
        here=h;
        Exec(h);
    }
    return 1;
}

// REP - Read/Execute/Print (no Loop)
FILE *REP(FILE *fp) {
    if (!fp) { fp = stdin; }
    if ((fp == stdin) && (state==0)) { printf(" ok\n"); }
    if (fgets(tib, sizeof(tib), fp) == tib) {
        parseLine(tib);
        return fp;
    }
    if (fp == stdin) { exit(0); }
    fclose(fp);
    return NULL;
}

void addSysOp(const char *nm, int isImm, int sysOp) {
    addWord(nm); if (isImm) makeImm();
    comma(LIT1); comma(sysOp); comma(SYS); comma(EXIT);
}

void baseSys() {
    addSysOp("BYE", 0, 0);
    addSysOp(":",   1, 1);
    addSysOp(";",   1, 2);
    addSysOp("IMMEDIATE", 1, 3);

    addPrim("SYS",   SYS);
    addPrim("WORDS", WDS);
    addPrim("EXIT",  EXIT);
    addPrim("DUP",   DUP);
    addPrim("SWAP",  SWAP);
    addPrim("OVER",  OVER);
    addPrim("DROP",  DROP);
    addPrim("FOR",   FOR);
    addPrim("I",     INDEX);
    addPrim("NEXT",  NEXT);
    addPrim("AND",   AND);
    addPrim("OR",    OR);
    addPrim("XOR",   XOR);
    addPrim("EMIT",  EMIT);
    addPrim("(.)",   DOT);
    addPrim("+",     ADD);
    addPrim("-",     SUB);
    addPrim("*",     MUL);
    addPrim("/",     DIV);
    addPrim("<",     LT);
    addPrim("=",     EQ);
    addPrim(">",     GT);
    addPrim("@",     FET);
    addPrim("!",     STO);
    addPrim("CLOCK", CLK);
    // addPrim(":",     COLON); makeImm();
    // addPrim(";",     SEMI);  makeImm();
    // addPrim("IMMEDIATE",  IMM);  makeImm();
    // parseLine(": BYE 2 SYS ;");
    parseLine(": . (.) : space 32 emit ;");
    parseLine(": HERE 0 @ ; : LAST 1 @ ; : BASE 2 ; : STATE 3 ;");
    parseLine(": 1+ 1 + ;");
    parseLine(": , HERE ! HERE 1+ 0 ! ;");
    parseLine(": JMP, 23 , ;");
    parseLine(": JMPZ, 24 , ;");
    parseLine(": JMPNZ, 25 , ;");
    parseLine(": BEGIN HERE ; IMMEDIATE");
    parseLine(": AGAIN JMP, , ; IMMEDIATE");
    parseLine(": WHILE JMPNZ, , ; IMMEDIATE");
    parseLine(": UNTIL JMPZ, , ; IMMEDIATE");
    parseLine(": IF JMPZ, HERE 0 , ; IMMEDIATE");
    parseLine(": THEN HERE SWAP ! ; IMMEDIATE");
    parseLine(": .c here for i @ . next ;");
    parseLine(": .d for i over + @ . next drop ;");
}

void Init() {
    for (int t=0; t<CODE_SZ; t++) { code[t]=0; }
    sp = rsp = lsp = state = last = 0;
    base = 10;
    here = LPRIM+1;
    baseSys();
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc>1) { fp=fopen(argv[1], "rt"); }
    Init();
    while (1) { fp = REP(fp); }; // REPL
    return 0;
}
