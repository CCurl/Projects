// pg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include <inttypes.h>

#define PGM_SZ   65535
#define DICT_SZ    511
#define VAR_SZ   65535
#define SRC_SZ    8191
#define NAME_LEN    13

typedef unsigned char byte;
typedef long int32;
typedef int32 CELL;
typedef void (*funcPtr)();
typedef struct {
    CELL xt;
    byte flags;
    byte len;
    char name[NAME_LEN + 1];
} DICT_T;

#define TOS       stk[sp]
#define NOS       stk[sp-1]
#define PUSH(x)   stk[++sp]=x
#define POP       stk[sp--]
#define DROP      sp--
#define DROP2     sp-=2
#define RPUSH(x)  rstk[++rsp]=x
#define RPOP      rstk[rsp--]
#define LPUSH(x)  lstk[++lsp]=x
#define LPOP      lstk[lsp--]
#define FLG_IMM   0x80
#define FLG_PRIM  0x40

char sp, rsp, lsp, *toIn, src[SRC_SZ+1];
CELL stk[32], lstk[32];
int here, vhere, last, base, state, t, n;
funcPtr *ip, next;
funcPtr pgm[PGM_SZ + 1], *rstk[32];
byte vars[VAR_SZ+1];
DICT_T dict[DICT_SZ + 1];

int strLen(const char* src) {
    int len = 0;
    while (*(src++)) { ++len; }
    return len;
}

byte strCpy(char* dst, const char* src, byte max) {
    char* x = dst;
    int l = 0;
    if (max < 1) { max = 255; }
    while (*src && (l < max)) {
        *(x++) = *(src++);
        ++l;
    }
    *x = 0;
    return l;
}

int strCmp(const char *src, const char *dst) {
    while ((*src) && (*dst) && (*src==*dst)) { ++src; ++dst; }
    return (*src)-(*dst);
}

int strCmpN(const char *src, const char *dst, int nc) {
    while ((*src==*dst) && nc>0) { ++src; ++dst; --nc; }
    return (*src)-(*dst);
}

int getWord(char *wd) {
    int l = 0;
    while (*toIn && *toIn<33) { ++toIn; }
    while (*toIn && *toIn>32) { *(wd++) = *(toIn++); ++l; }
    *wd = 0;
    return l;
}

FILE *fOpen(const char *nm, const char *md) {
#ifdef _WIN32
    FILE *fp = NULL;
    fopen_s(&fp, nm, md);
    return fp;
#else
    return fopen(nm, md);
#endif
}

void doNOP() { }
void doBREAK() { /* TODO! */ }
void doEXIT() { if (rsp) { ip = RPOP; } else { ip = &pgm[PGM_SZ]; *ip = 0; } }
void doCOL() {
    t = *(int*)(ip++);
    if (*ip != doEXIT) { RPUSH(ip); }
    ip = &pgm[t];
}
void doEXEC() { /*TODO: funcPtr x = (funcPtr)POP; x(); */ }
void doDUP() { t = TOS; PUSH(t); }
void doSWAP() { t = TOS; TOS = NOS; NOS = t; }
void doOVER() { t = NOS; PUSH(t); }
void doDROP() { if (sp) { DROP; } }
void doJMP() { ip = (funcPtr*)*ip; }
void doDO() { RPUSH(ip); LPUSH(NOS); LPUSH(TOS); DROP2; }
void doI() { PUSH(lstk[lsp]); }
void doJ() { PUSH(lstk[lsp - 4]); }
void doLOOP() { if (++lstk[lsp] < lstk[lsp - 1]) { ip = rstk[rsp]; } else { rsp--; lsp -= 2; } }
void doBEGIN() { RPUSH(ip); }
void doWHILE() { if (POP) { ip = rstk[rsp]; } else { rsp--; } }
void doAGAIN() { ip = rstk[rsp]; }
void doLIT() { PUSH((CELL)*(ip++)); }
void do0BRANCH() { if (POP == 0) { ip = (funcPtr*)*ip; } else { ++ip; } }
void doDOTD() { printf("%ld ", POP); }
void doDOTH() { printf("%lx ", POP); }
void doEMIT() { printf("%c", (char)POP); }
void doSEMI() { pgm[here++] = doEXIT; state = 0; }
void doFETCH() { TOS = *(CELL*)&vars[TOS]; }
void doCFETCH() { TOS = vars[TOS]; }
void doSTORE() { *(CELL*)&vars[TOS] = NOS; DROP2; }
void doCSTORE() { vars[TOS] = (byte)NOS; DROP2; }
void doEQ() { NOS = (NOS == TOS) ? 1 : 0; DROP; }
void doLT() { NOS = (NOS < TOS) ? 1 : 0; DROP; }
void doGT() { NOS = (NOS > TOS) ? 1 : 0; DROP; }
void doADD() { NOS += TOS; DROP; }
void doSUB() { NOS -= TOS; DROP; }
void doMUL() { NOS *= TOS; DROP; }
void doDIV() { NOS /= TOS; DROP; }
void doMOD() { NOS %= TOS; DROP; }
void doAND() { NOS &= TOS; DROP; }
void doOR() { NOS |= TOS; DROP; }
void doXOR() { NOS ^= TOS; DROP; }
void doCOM() { TOS = ~TOS; }
void doLAST() { PUSH((CELL)&last); }
void doHERE() { PUSH((CELL)&here); }
void doWORDS() { for (int l = last; 0 <= l; l--) { printf("%s\t", dict[l].name); } }
void doIMMEDIATE() { dict[last].flags |= FLG_IMM; }
void doCREATE() {
    ++last;
    dict[last].len = strCpy(dict[last].name, (char*)POP, NAME_LEN);
    dict[last].xt = POP;
    dict[last].flags = 0;
}
void doFIND() {
    char *nm = (char*)TOS;
    int l = strLen(nm);
    TOS = 0;
    for (int i = last; 0 <= i; i--) {
        if (l != dict[i].len) { continue; }
        if (strCmp(nm, dict[i].name)) { continue; }
        TOS = dict[i].xt;
        PUSH(dict[i].flags);
        PUSH(1);
    }
}
void doIsNum() {
    char *wd = (char*)TOS, *end;
    intmax_t x = strtoimax(wd, &end, 0);
    if ((x == 0) && (strCmp(wd, "0"))) { TOS = 0; return; }
    TOS = (CELL)x;
    PUSH(1);
}
void doPARSE() {
    char *wd = (char*)POP;
    if (strCmp("[define]",wd)==0) {
        if (getWord(wd)) { PUSH(here); PUSH((CELL)wd); doCREATE(); }
        strCpy(wd, "[compile]", 16);
    }
    if (strCmp("[exec]", wd)==0) { state = 0; PUSH(1); return; }
    if (strCmp("[compile]", wd)==0) { state = 1; PUSH(1); return; }
    if (strCmp("[comment]", wd)==0) { state = 2; PUSH(1); return; }

    if (state==2) { PUSH(1); return; }
    
    PUSH((CELL)wd); doFIND();
    if (POP) {
        if (state==1) {
            if (TOS & FLG_PRIM) { pgm[here++]=(funcPtr)NOS; }
            else { pgm[here++]=doCOL; pgm[here++]=(funcPtr)NOS; }
        } else { doEXEC(); }
        DROP; TOS=1; return;
    }
    PUSH((CELL)wd); doIsNum();
    if (POP) {
        if (state==1) { pgm[here++]=doLIT; pgm[here++]=(funcPtr)POP; }
        PUSH(1); return;
    }
    printf("[%s]??", wd);
    state = 0;
    PUSH(0);
}
void doCOMPILE() {
    toIn = &src[0];
    char *wd = (char*)&vars[VAR_SZ-64];
    while (getWord(wd)) {
        PUSH((CELL)wd);
        doPARSE();
        if (POP==0) { return; }
    }
}
void doLOAD() {
    char nm[32];
#ifdef _WIN32
    sprintf_s(nm, 16, "blk-%03ld.4th", POP);
#else 
    sprintf(nm, "blk-%03ld.4th", POP);
#endif
    FILE *fp=fOpen(nm, "rb");
    if (fp) { fread(src, 1, SRC_SZ, fp); fclose(fp); }
    else { printf("-nf-"); }
}

void GotoXY(int x, int y) { printf("\x1B[%d;%dH", y, x); }
void CLS() { printf("\x1B[2J"); GotoXY(1, 1); }
void CursorOn() { printf("\x1B[?25h"); }
void CursorOff() { printf("\x1B[?25l"); }
void FG(int clr) { printf("\x1B[%d;40m", 30+clr); }

void doEdit() {
    printf("-ed-");
    int sz = strLen(src);
    char *cp = src;
    for (int i = 0; i < sz; i++) {
        if (*cp == '[') {
            if (strCmpN("[define]",  cp, 8)==0) { FG(33); cp += 8; continue; }
            if (strCmpN("[compile]", cp, 9)==0) { FG(34); cp += 9; continue; }
            if (strCmpN("[comment]", cp, 9)==0) { FG(35); cp += 9; continue; }
            if (strCmpN("[exec]",    cp, 6)==0) { FG(36); cp += 6; continue; }
        }
        printf("%c", *cp);
    }
}

void primCreate(const char* nm, funcPtr xt) {
    PUSH((CELL)xt);
    PUSH((CELL)nm);
    doCREATE();
    dict[last].flags = FLG_PRIM;
}

void init() {
    here = sp = rsp = lsp = 0;
    last = -1;
    for (int i = 0; i <= PGM_SZ; i++) { pgm[i] = 0; }
    for (int i = 0; i <= VAR_SZ; i++) { vars[i] = 0; }
    for (int i = 0; i <= SRC_SZ; i++) { src[i] = 0; }

    primCreate("RET", doEXIT);
    primCreate("EXEC", doEXEC);
    primCreate("DUP", doDUP);
    primCreate("SWAP", doSWAP);
    primCreate("OVER", doOVER);
    primCreate("DROP", doDROP);
    primCreate("JMP", doJMP);
    primCreate("DO", doDO);
    primCreate("I", doI);
    primCreate("J", doJ);
    primCreate("LOOP", doLOOP);
    primCreate("BEGIN", doBEGIN);
    primCreate("WHILE", doWHILE);
    primCreate("AGAIN", doAGAIN);
    primCreate(".", doDOTD);
    primCreate(".h", doDOTH);
    primCreate("EMIT", doEMIT);
    primCreate("@", doFETCH);
    primCreate("C@", doCFETCH);
    primCreate("!", doSTORE);
    primCreate("C!", doCSTORE);
    primCreate("=", doEQ);
    primCreate("<", doLT);
    primCreate(">", doGT);
    primCreate("+", doADD);
    primCreate("-", doSUB);
    primCreate("*", doMUL);
    primCreate("/", doDIV);
    primCreate("MOD", doMOD);
    primCreate("AND", doAND);
    primCreate("OR", doOR);
    primCreate("XOR", doXOR);
    primCreate("COM", doCOM);
    primCreate("LAST", doLAST);
    primCreate("HERE", doHERE);
    primCreate("WORDS", doWORDS);
    primCreate("IMMEDIATE", doIMMEDIATE);
    primCreate("EDIT", doEdit);
}

int main()
{
    init();
    PUSH(0);
    doLOAD();
    doCOMPILE();
    CELL xt = dict[last].xt;
    PUSH((CELL)"main");
    doFIND();
    if (POP) { DROP; xt = POP; }
    ip = (funcPtr*)&pgm[xt];
    while (*ip) { (*ip++)(); }
}
