#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CODE_MAX      0xFFFF
#define DICT_SZ      32*1024
#define STK_SZ            64
#define btwi(a,b,c)   ((b<=a) && (a<=c))
#define NCASE         goto next; case
#define RCASE         return; case
#define RETi0         if (w[1]==0) { return 0; } w++
#define Make2(l,h)    (h<<8)|l

#define TOS           stk[sp]
#define NOS           stk[sp-1]
#define PUSH(x)       stk[++sp]=(x)
#define POP()         stk[sp--]
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define HERE          sys[HEREA]
#define LAST          sys[LASTA]
#define BASE          sys[BASEA]
#define STATE         sys[STATEA]
#define IS_IMM        0x01
#define IS_PRIM       0x02
#define IS_INLINE     0x04

enum { HEREA=0, LASTA, BASEA, STATEA, U4, U5, U6, U7 };

typedef long cell;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef struct { ushort xt; byte fl; byte ln; char nm[28]; } DE_T;

ushort sys[U7+1];
cell stk[STK_SZ], rstk[STK_SZ], lstk[30];
DE_T dict[DICT_SZ];
byte code[CODE_MAX+1];
short sp, rsp, lsp, pc;

enum {
    STOP, LIT1, LIT2, P03, LIT4, DUP, SWAP, DROP, FOR, INDEX,    //  0 ->  9
    NEXT, ADD, SUB, MUL, DIV, CALL, JMP, JMPZ, LT, EQ,           // 10 -> 19
    GT, P21, P22, P23, P24, P25, P26, P27, P28, P29,             // 20 -> 29
    EXIT, SYSOP                                                  // 30 -> 31
};
enum {
    EMIT, DOT, CLK, COLON, SEMI, IMM, INL, BYE, S08, S09         //  0 ->  9
};

cell fetch2(byte *a) { return *(short*)(a); }
cell fetch4(byte *a) { return *(cell*)(a); }
void store2(byte *a, cell val) { *(short*)(a) = (short)(val); }
void store4(byte *a, cell val) { *(cell*)(a) = (cell)(val); }

void comma1(byte x)  { code[HERE++]=x; }
void comma2(short x) { store2(&code[HERE], x); HERE+=2; }
void comma4(cell x)  { store4(&code[HERE], x); HERE+=sizeof(cell); }

DE_T *addWord(const char *wd);
void makeImm() { dict[LAST-1].fl |= IS_IMM; }
void makeInl() { dict[LAST-1].fl |= IS_INLINE; }
void makePrim(char *nm, byte x, char y) {
	addWord(nm);
	if (0 <= y) { comma1(LIT1); comma1(y); }
	comma1(x);
	comma1(EXIT);
	dict[LAST-1].fl = IS_INLINE;
}

void SysOp() {
    switch(POP()) {
        case EMIT:   printf("%c", (char)POP());
        RCASE DOT:   printf("%ld", (long)POP());
        RCASE CLK:   PUSH(clock());
        RCASE COLON: addWord(0); STATE=1;
        RCASE SEMI:  comma1(EXIT); STATE=0;
        RCASE IMM:   makeImm();
        RCASE BYE:   printf("\n"); exit(0);
		return;
	}
}

void Exec(ushort start) {
    pc = start;
    cell t;
    next:
    switch(code[pc++]) {
        case  STOP:  return;
        NCASE LIT1:  PUSH((char)code[pc++]);
        NCASE LIT2:  PUSH(fetch2(&code[pc])); pc += 2;
        NCASE P03:
        NCASE LIT4:  PUSH(fetch4(&code[pc])); pc += sizeof(cell);
        NCASE DUP:   t=TOS; PUSH(t);
        NCASE SWAP:  t=TOS; TOS=NOS; NOS=t;
        NCASE DROP:  if (0<sp) { sp--; } else { sp=0; }
        NCASE FOR:   lsp += 3; L2=pc; L1=POP(); L0=0;
        NCASE INDEX: PUSH(L0);
        NCASE NEXT:  if (++L0<L1) { pc=L2; } else { lsp-=3; }
        NCASE ADD:   t=POP(); TOS+=t;
        NCASE SUB:   t=POP(); TOS-=t;
        NCASE MUL:   t=POP(); TOS*=t;
        NCASE DIV:   t=POP(); TOS/=t;
        NCASE CALL:  if (code[pc+2] != EXIT) { rstk[++rsp]=pc+2; }
        case  JMP:   pc=fetch2(&code[pc]);
        NCASE JMPZ:  if (POP()==0) { pc=fetch2(&code[pc]); } else { pc+=2; }
        NCASE LT:    t=POP(); TOS=(TOS<t)  ? 1 : 0;
        NCASE EQ:    t=POP(); TOS=(TOS==t) ? 1 : 0;
        NCASE GT:    t=POP(); TOS=(TOS>t)  ? 1 : 0;
        NCASE P21:
        NCASE P22:
        NCASE P23:
        NCASE P24:
        NCASE P25:
        NCASE P26:
        NCASE P27:
        NCASE P28:
        NCASE P29:
        NCASE EXIT:  if (0<rsp) { pc=rstk[rsp--]; } else { rsp=0; return; }
        NCASE SYSOP: SysOp();
            goto next;
        default:
            goto next;
    }
}

char wd[32], *toIn, *tib;
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
    int ln = strLen(w);
    if (ln==0) return (DE_T*)0;
    if (ln>27) { ln=27; }
    DE_T *de=(DE_T*)&dict[LAST++];
    de->xt = HERE;
    de->fl = 0;
    de->ln = ln;
    for (int i = 0; i < ln; i++) { de->nm[i] = w[i]; }
    de->nm[ln]=0;
    return de;
}

DE_T *findWord(const char *w) {
    if (!w) { nextWord(); w=wd; }
    for (int e=LAST-1; 0<=e; e-- ) {
        DE_T *de = (DE_T*)&dict[e];
        if (strEqI(de->nm, w)) { return de; }
    } 

    return (DE_T*)0;
}

int isNum(const char *w, int b) {
    cell n=0, isNeg=0;
    if ((w[0]==39) && (w[2]==0)) { PUSH(w[1]); return 1; }
    if (w[0]=='%') { b=2; RETi0; }
    if (w[0]=='#') { b=10; RETi0; }
    if (w[0]=='$') { b=16; RETi0; }
    if ((b==10) && (w[0]=='-')) { isNeg=1; RETi0; }
    char c = *(w++);
    while (c) {
        n = (n*b);
        if ((b==2) && btwi(c,'0','1')) { n+=(c-'0'); }
        else if ((b==16) && btwi(c,'A','F')) { n+=(c-'A'+10); }
        else if ((b==16) && btwi(c,'a','f')) { n+=(c-'a'+10); }
        else if ((b>9) && btwi(c,'0','9')) { n+=(c-'0'); }
        else return 0;
        c = *(w++);
    }
    if (isNeg) { n = -n;; }
    PUSH(n);
    return 1;
}

int parseWord(char *w) {
    if (!w) { w = &wd[0]; }
    // printf("-pw:%s-",w);

    if (isNum(wd, BASE)) {
        cell n = POP();
        if (btwi(n, -127, 127)) {
            comma1(LIT1); comma1(n);
        } else if (btwi(n, -32767, 32767)) {
            comma1(LIT2); comma2(n);
        } else {
            comma1(LIT4); comma4(n);
        }
        return 1;
    }

    DE_T *de = findWord(w);
    if (de) {
        if (de->fl & IS_IMM) {
			code[HERE+100] = CALL;
			store2(&code[HERE+101], de->xt);
			code[HERE+103] = EXIT;
            Exec(HERE+100);
        } else {
            if (de->fl & IS_INLINE) {
				int x = de->xt;
                comma1(code[x++]);
				while (code[x] != EXIT) { comma1(code[x++]); }
			} else {
				comma1(CALL);
				comma2(de->xt);
			}
        }
        return 1;
     }

    return 0;
}

int parseLine(const char *ln) {
    int h=HERE, l=LAST;
    toIn = (char *)ln;
    while (nextWord()) {
        if (!parseWord(wd)) {
            printf("-%s?-", wd);
            HERE=h;
            LAST=l;
            STATE=0;
            return 0;
        }
    }
    if ((l==LAST) && (h<HERE) && (STATE==0)) {
        comma1(EXIT);
        HERE=h;
        Exec(h);
    }
    return 1;
}

// REP - Read/Execute/Print (no Loop)
FILE *REP(FILE *fp) {
    if (sp<0) { sp=0; }
    if (rsp<0) { rsp=0; }
    if (lsp<0) { lsp=0; }
    tib = (char*)&code[CODE_MAX-128];
    if (!fp) { fp = stdin; }
    if (fp == stdin) { printf(" ok\n"); }
    if (fgets(tib, 127, fp) == tib) {
        parseLine(tib);
        return fp;
    }
    if (fp == stdin) { exit(0); }
    fclose(fp);
    return NULL;
}

void Init() {
    sp = rsp = lsp = 0;
    HERE = STATE = LAST = 0;
    BASE = 10;
    for (int t=0; t<=CODE_MAX; t++) { code[t]=0; }
    makePrim(":",    SYSOP, COLON); makeImm();
    makePrim(";",    SYSOP, SEMI); makeImm();
    makePrim("INLINE",  SYSOP, INL); makeImm();
    makePrim("IMMEDIATE",  SYSOP, IMM); makeImm();
    makePrim("xQ",   SYSOP, BYE);
    makePrim("EXIT", EXIT, -1);
    makePrim("DUP",  DUP, -1);
    makePrim("SWAP", SWAP, -1);
    makePrim("DROP", DROP, -1);
    makePrim("FOR",  FOR, -1);
    makePrim("I",    INDEX, -1);
    makePrim("NEXT", NEXT, -1);
    makePrim("EMIT", SYSOP, EMIT);
    makePrim("(.)",  SYSOP, DOT);
    makePrim("+",    ADD, -1);
    makePrim("-",    SUB, -1);
    makePrim("*",    MUL, -1);
    makePrim("/",    DIV, -1);
    makePrim("CLOCK", SYSOP, CLK);
    makePrim("<",    LT, -1);
    makePrim("=",    EQ, -1);
    makePrim(">",    GT, -1);
    parseLine(": space 32 emit ; inline");
    parseLine(": cr 10 emit ; inline");
    parseLine(": . (.) space ; inline");
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc>1) { fp=fopen(argv[1], "rt"); }
    Init();
    while (1) { fp = REP(fp); }; // REPL
    return 0;
}
