#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CODE_SZ       0x7FFF
#define VARS_SZ       0xFFFF
#define DICT_SZ         8192
#define LASTPRIM         BYE
#define STK_SZ            64
#define btwi(a,b,c)   ((b<=a) && (a<=c))
#define comma(x)      code[here++]=(x)
#define NCASE         goto next; case
#define RCASE         return; case
#define RetIf0        if (*(++w)==0) return 0

#define here          code[0]
#define last          code[1]
#define vhere         code[2]
#define base          code[3]
#define state         code[4]
#define TOS           stk[sp].i
#define NOS           stk[sp-1].i
#define PUSH(x)       push(x)
#define POP()         pop()
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
static ushort sp, rsp, lsp;
cell lstk[60];
static char tib[128];
static ushort pc, wc, code[CODE_SZ+1];
byte vars[VARS_SZ];

enum {
    STOP, LIT1, LIT2, EXIT, DUP, SWAP, DROP, FOR, INDEX, NEXT,
    AND, OR, XOR, ADD, SUB, MUL, DIV, LT, EQ, GT,              
    CLK, OVER, JMP, JMPZ, JMPNZ, EMIT, DOT, FET, CFET, FETC, STO, CSTO, STOC,
    COLON, SEMI, IMM, IF, THEN, COMMA, WDS,
    BYE
};

DE_T *addWord(const char *wd);
void push(cell x) { stk[++sp].i = x; }
cell pop() { return (0<sp) ? stk[sp--].i : 0; }
void store2(byte *a, cell val) { *(cell*)(a) = val; }
cell fetch2(byte *a) { return *(cell*)(a); }
void makeImm() { dict[last-1].fl=1; }

void words() {
    int n=0, t=0;
    for (int e=last-1; 0<=e; e--) {
        DE_T *de = (DE_T*)&dict[e];
        printf("%s\t", de->nm);
        if (LASTPRIM < de->xt) printf("(%04X)\t", de->xt & 0x7FFF);
        else  printf("(%02X)\t", de->xt);
        ++n; if (n%8==0) printf("\n");
    }
}

void Exec(int start) {
    cell t, n;
    pc = start;
    next:
    wc = code[pc++];
    switch(wc) {
        case  STOP:  return;
        NCASE LIT1:  PUSH(code[pc++]);
        NCASE LIT2:  PUSH(fetch2((byte*)&code[pc])); pc += sizeof(cell)/2;
        NCASE EXIT:  if (0<rsp) { pc = (ushort)rstk[rsp--].i; } else { return; }
        NCASE DUP:   t=TOS; PUSH(t);
        NCASE SWAP:  t=TOS; TOS=NOS; NOS=t;
        NCASE DROP:  pop();
        NCASE FOR:   lsp+=3; L2=pc; L1=POP(); L0=0;
        NCASE INDEX: PUSH(L0);
        NCASE NEXT:  if (++L0<L1) { pc=(ushort)L2; } else { lsp-=3; } if (lsp<0) lsp=0;
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
        NCASE JMP:   pc=code[pc];
        NCASE JMPZ:  if (POP()==0) { pc=code[pc]; } else { ++pc; }
        NCASE JMPNZ: if (POP()) { pc=code[pc]; } else { ++pc; }
        NCASE EMIT:  printf("%c", (char)POP());
        NCASE DOT:   printf("%ld", POP());
        NCASE FET:   TOS = fetch2(&vars[TOS]);
        NCASE CFET:  TOS = vars[TOS];
        NCASE FETC:  TOS = code[TOS];
        NCASE STO:   t=POP(); n=POP(); store2(&vars[t], n);
        NCASE CSTO:  t=POP(); n=POP(); vars[t]=n;
        NCASE STOC:  t=POP(); n=POP(); code[t] = (short)n;
        NCASE COLON: addWord(0); state = 1;
        NCASE SEMI : if (LASTPRIM < code[here - 1]) { code[here - 1] &= 0x07FF; } // Tail-call
            else { comma(EXIT); }
            state = 0;
        NCASE IMM : makeImm();
        NCASE IF : comma(JMPZ); PUSH(here); comma(0);
        NCASE THEN : code[POP()] = here;
        NCASE COMMA : comma((ushort)POP());
        NCASE WDS : words();
        NCASE BYE:   exit(0);
        default:
            if (LASTPRIM < wc) {
                if (wc & 0x8000) { rstk[++rsp].i = pc; }
                pc = (wc & 0x7FFF);
            }
        goto next;
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

DE_T *addWord2(const char *w) {
    int l = strLen(w);
    int sz = 2 + 4 + l;
    while (sz&3) { ++sz; }
    DE_T *dp = ();
}

DE_T *findWord2(const char *w) {
    if (!w) { nextWord(); w=wd; }
    int l = strLen(w);
    for (int e=last-1; 0<=e; e--) {
        DE_T *de = (DE_T*)&dict[e];
        if ((l==de->ln) && strEqI(de->nm, w)) { return de; }
    }

    return (DE_T*)0;
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
            comma(LIT2); store2((byte*)&code[here], n); here += sizeof(cell)/2;
        }
        return 1;
    }

    DE_T *de = findWord(w);
    if (de) {
        if (de->fl == 1) {   // IMMEDIATE
            int h = here+100;
            code[h]   = de->xt;
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

DE_T *addPrim(const char* wd, ushort prim) {
    DE_T* w = addWord(wd);
    w->xt = prim;
    return w;
}

void baseSys() {
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
    addPrim("TIMER", CLK);
    addPrim("@C",    FETC);
    addPrim("@",     FET);
    addPrim("C@",    CFET);
    addPrim("!C",    STOC);
    addPrim("!",     STO);
    addPrim("C!",    CSTO);
    addPrim(":",     COLON)->fl = 1;
    addPrim(";",     SEMI)->fl = 1;
    addPrim("IMMEDIATE", IMM)->fl = 1;
    addPrim("IF",    IF)->fl = 1;
    addPrim("THEN",  THEN)->fl = 1;
    addPrim(",",     COMMA);
    addPrim("WORDS", WDS);
    addPrim("BYE",   BYE);

    parseLine(": (HERE)  0 ; : HERE  (HERE)  @C ;");
    parseLine(": (LAST)  1 ; : LAST  (LAST)  @C ;");
    parseLine(": (VHERE) 2 ; : VHERE (VHERE) @C ;");
    parseLine(": BASE    3 ; : STATE 4 ;");
    parseLine(": JMP, 22 , ;");
    parseLine(": JMPZ, 23 , ;");
    parseLine(": JMPNZ, 24 , ;");
    parseLine(": BEGIN HERE ; IMMEDIATE");
    parseLine(": AGAIN JMP, , ; IMMEDIATE");
    parseLine(": WHILE JMPNZ, , ; IMMEDIATE");
    parseLine(": UNTIL JMPZ, , ; IMMEDIATE");
    parseLine(": 1+ 1 + ;");
    parseLine(": space 32 emit ; : . (.) space ;");
    parseLine(": cr 13 emit 10 emit ;");
    parseLine(": s  8 @ ; : >s  8 ! ; : s+ s dup 1+ >s ;");
    parseLine(": d 16 @ ; : >d 16 ! ; : d+ d dup 1+ >d ;");
}

void Init() {
    for (int t=0; t<CODE_SZ; t++) { code[t]=0; }
    sp = rsp = lsp = state = last = 0;
    base = 10;
    here = LASTPRIM+1;
    baseSys();
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc>1) { fp=fopen(argv[1], "rt"); }
    Init();
    while (1) { fp = REP(fp); }; // REPL
    return 0;
}
