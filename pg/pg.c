#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define CODE_SZ       0x7FFF
#define VARS_SZ       0xFFFF
#define DICT_SZ       0xFFF0
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
#define lex           code[5]
#define TOS           stk[sp].i
#define NOS           stk[sp-1].i
#define PUSH(x)       push(x)
#define POP()         pop()
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]

#if __LONG_MAX__ > __INT32_MAX__
#define CELL_SZ   8
#define FLT_T     double
#else
#define CELL_SZ   4
#define FLT_T     float
#endif

typedef long cell;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef union { FLT_T f; cell i; } SE_T;
typedef struct { ushort xt; byte sz, fl, lx, ln; char nm[32]; } DE_T;

SE_T stk[STK_SZ], rstk[STK_SZ];
byte dict[DICT_SZ+1];
byte vars[VARS_SZ];
static ushort sp, rsp, lsp;
cell lstk[60];
static char tib[128];
static ushort pc, wc, code[CODE_SZ+1];

enum {
    STOP, LIT1, LIT2, EXIT,
    DUP, SWAP, DROP, OVER,
    FET, CFET, WFET, STO, CSTO, WSTO,
    AND, OR, XOR,
    ADD, SUB, MUL, DIV,
    LT, EQ, GT,              
    JMP, JMPZ, JMPNZ,
    EMIT, DOT,
    FOR, INDEX, NEXT,
    COLON, SEMI, IMM, COMMA, CREATE, WDS,
    CLK, BYE
};

DE_T *addWord(const char *wd);
void push(cell x) { stk[++sp].i = x; }
cell pop() { return (0<sp) ? stk[sp--].i : 0; }
void makeImm() { DE_T *dp = (DE_T*)&dict[last]; dp->fl=1; }
void storeCell(cell a, cell val) { *(cell*)(a) = val; }
void storeWord(cell a, cell val) { *(ushort*)(a) = (ushort)val; }
cell fetchCell(cell a) { return *(cell*)(a); }
cell fetchWord(cell a) { return *(ushort*)(a); }

void commaCell(cell n) {
    storeCell((cell)&code[here], n);
    here += sizeof(cell)/2;
}

void words() {
    int cw = last;
    int n=0;
    while (cw < DICT_SZ) {
        DE_T *de = (DE_T*)&dict[cw];
        printf("%s ", de->nm);
        if (LASTPRIM < de->xt) printf("(%d)\t", de->xt & 0x7FFF);
        else  printf("(%d:%d)\t", de->xt, de->fl);
        ++n; if (n%8==0) printf("\n");
        cw += de->sz;
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
        NCASE LIT2:  PUSH(fetchCell((cell)&code[pc])); pc += sizeof(cell)/2;
        NCASE EXIT:  if (0<rsp) { pc = (ushort)rstk[rsp--].i; } else { return; }
        NCASE DUP:   t=TOS; PUSH(t);
        NCASE SWAP:  t=TOS; TOS=NOS; NOS=t;
        NCASE DROP:  pop();
        NCASE OVER : t = NOS; PUSH(t);
        NCASE FOR:   lsp+=3; L2=pc; L1=POP(); L0=0;
        NCASE INDEX: PUSH(L0);
        NCASE NEXT:  if (++L0<L1) { pc=(ushort)L2; } else { lsp-=3; if (lsp<0) lsp=0; }
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
        NCASE JMP:   pc=code[pc];
        NCASE JMPZ:  if (POP()==0) { pc=code[pc]; } else { ++pc; }
        NCASE JMPNZ: if (POP()) { pc=code[pc]; } else { ++pc; }
        NCASE EMIT:  printf("%c", (char)POP());
        NCASE DOT:   printf("%ld", POP());
        NCASE FET:   TOS = fetchCell(TOS);
        NCASE CFET:  TOS = *(byte *)TOS;
        NCASE WFET:  TOS = fetchWord(TOS);
        NCASE STO:   t=POP(); n=POP(); storeCell(t, n);
        NCASE CSTO:  t=POP(); n=POP(); *(byte*)t=(byte)n;
        NCASE WSTO:  t=POP(); n=POP(); storeWord(t, n);
        NCASE COLON: addWord(0); state = 1;
        NCASE SEMI : if (LASTPRIM < code[here-1]) { code[here-1] &= 0x07FF; } // Tail-call
            else { comma(EXIT); }
            state = 0;
        NCASE IMM : makeImm();
        NCASE COMMA : comma((ushort)POP());
        NCASE CREATE: { byte *b=(byte*)addWord(0); PUSH(b-dict); }
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
    while (btwi(*toIn,1,32)) { ++toIn; }
    while (btwi(*toIn,33,126)) { wd[l++] = *(toIn++); }
    wd[l] = 0;
    return l;
}

int lower(char c) { return btwi(c,'A','Z') ? c+32 : c; }
int strLen(const char *s) { int l=0; while (s[l]) { l++; } return l; }
int strEqI(const char *s, const char *d) {
    while (lower(*s) == lower(*d)) { if (*s==0) { return 1; } s++; d++; }
    return 0;
}
void strCpy(char *d, const char *s) {
    while (*s) { *(d++) = *(s++); }
    *(d) = 0;
}

DE_T *addWord(const char *w) {
    if (!w) { nextWord(); w=wd; }
    int ln = strLen(w);
    int sz = ln + 7;
    if (sz&1) { ++sz; }
    ushort newLast = last - sz;
    DE_T *dp = (DE_T*)&dict[newLast];
    dp->sz = sz;
    dp->xt = here | 0x8000;
    dp->fl = 0;
    dp->lx = lex;
    dp->ln = ln;
    strCpy(dp->nm, w);
    last = newLast;
    // printf("\n-add:%d,[%s],%d (%d)-", last, dp->nm, here, dp->xt&0x7FFF);
    return dp;
}

DE_T *findWord(const char *w) {
    if (!w) { nextWord(); w=wd; }
    // printf("\n-fw:(%s)-", w);
    int len = strLen(w);
    int cw = last;
    while (cw < DICT_SZ) {
        DE_T *dp = (DE_T*)&dict[cw];
        // printf("-%d,(%s)-", cw, dp->nm);
        if ((len==dp->ln) && strEqI(dp->nm, w)) { return dp; }
        cw += dp->sz;
    }
    return (DE_T*)0;
}

int isNum(const char *w, int b) {
    cell n=0, isNeg=0;
    if ((w[0]==39) && (w[2]==39) && (w[3]==0)) { PUSH(w[1]); return 1; }
    if (w[0]=='%') { b= 2; ++w; }
    if (w[0]=='#') { b=10; ++w; }
    if (w[0]=='$') { b=16; ++w; }
    if ((b==10) && (w[0]=='-')) { isNeg=1; ++w; }
    if (w[0]==0) { return 0; }
    char c = lower(*(w++));
    while (c) {
        n = (n*b);
        // if ((b==2) && btwi(c,'0','1')) { n+=(c-'0'); }
        if (btwi(c,'0','9') &&  btwi(c,'0','0'+b-1)) { n+=(c-'0'); }
        else if (btwi(c,'a','a'+b-11)) { n+=(c-'a'+10); }
        // else if ((b>10) && btwi(c,'a','a'+b-11)) { n+=(c-'a'+10); }
        else return 0;
        c = lower(*(w++));
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
            comma(LIT1); comma(n);
        } else {
            comma(LIT2);
            commaCell(n);
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
    addPrim("@",     FET);
    addPrim("C@",    CFET);
    addPrim("W@",    WFET);
    addPrim("!",     STO);
    addPrim("C!",    CSTO);
    addPrim("W!",    WSTO);
    addPrim(":",     COLON)->fl = 1;
    addPrim(";",     SEMI)->fl = 1;
    addPrim("IMMEDIATE", IMM)->fl = 1;
    //addPrim("IF",    IF)->fl = 1;
    //addPrim("THEN",  THEN)->fl = 1;
    addPrim(",",     COMMA);
    addPrim("WORDS", WDS);
    addPrim("CREATE", CREATE);
    addPrim("BYE",   BYE);

    addWord("CELL"); comma(LIT1); comma(CELL_SZ); comma(EXIT);
    addWord("CODE"); comma(LIT2); commaCell((cell)&code[0]); comma(EXIT);
    addWord("VARS"); comma(LIT2); commaCell((cell)&vars[0]); comma(EXIT);
    addWord("DICT"); comma(LIT2); commaCell((cell)&dict[0]); comma(EXIT);
    parseLine(": @C DUP + CODE + W@ ;");
    parseLine(": !C DUP + CODE + W! ;");
    parseLine(": (HERE)  0 ; : HERE  (HERE)  @C ;");
    parseLine(": (LAST)  1 ; : LAST  (LAST)  @C ;");
    parseLine(": (VHERE) 2 ; : VHERE (VHERE) @C ;");
    parseLine(": BASE    3 ; : STATE 4 ;");
    parseLine(": ALLOT VHERE + (VHERE) !C ;");
    parseLine(": @V  VARS + @ ;");
    parseLine(": C@V VARS + C@ ;");
    parseLine(": !V  VARS + ! ;");
    parseLine(": C!V VARS + C! ;");
    parseLine(": CELLS CELL * ;");
    parseLine(": JMP, 24 , ;");
    parseLine(": JMPZ, 25 , ;");
    parseLine(": JMPNZ, 26 , ;");
    parseLine(": BEGIN HERE ; IMMEDIATE");
    parseLine(": AGAIN JMP, , ; IMMEDIATE");
    parseLine(": WHILE JMPNZ, , ; IMMEDIATE");
    parseLine(": UNTIL JMPZ, , ; IMMEDIATE");
    parseLine(": IF JMPZ, HERE 0 , ; IMMEDIATE");
    parseLine(": THEN HERE SWAP !C ; IMMEDIATE");
    parseLine(": 1+ 1 + ;");
    parseLine(": space 32 emit ; : . (.) space ;");
    parseLine(": cr 13 emit 10 emit ;");
    parseLine("CELL ALLOT");
    parseLine(": a 0 @V ; : >a 0 !V ; : a+ a dup 1+ >a ;");
}

void Init() {
    for (int t=0; t<CODE_SZ; t++) { code[t]=0; }
    sp = rsp = lsp = state = 0;
    last = DICT_SZ;
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
