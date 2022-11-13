// itc.c : This is to play around with indirect threading, using a color-forth inspired paradigm
//
#include <Windows.h>
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
    char name[NAME_LEN+1];
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

#define STATE_EXEC     0
#define STATE_COMPILE  1
#define STATE_COMMENT  2
#define STATE_CREATE   3

#define FLG_IMM   0x80
#define FLG_PRIM  0x40

char *toIn, src[SRC_SZ + 1];
CELL stk[32], lstk[32];
int sp, rsp, lsp, here, vhere, last, base, state, t, n;
funcPtr *ip, next;
funcPtr pgm[PGM_SZ+1], *rstk[32];
byte vars[VAR_SZ+1];
DICT_T dict[DICT_SZ+1];

int strLen(const char* src) {
    int len = 0;
    while (*(src++)) { ++len; }
    return len;
}

byte strCpy(char* dst, const char* src, byte max) {
    int l = 0;
    if (max < 1) { max = 255; }
    while (*src && (l < max)) {
        *(dst++) = *(src++);
        ++l;
    }
    *dst = 0;
    return l;
}

int strCmp(const char *src, const char *dst) {
    while ((*src) && (*dst) && (*src==*dst)) { ++src; ++dst; }
    return (*src)-(*dst);
}

int strCmpN(const char *src, const char *dst, int nc) {
    while ((*src==*dst) && nc>0) { ++src; ++dst; --nc; }
    return nc == 0 ? 0 : 1;
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

byte running;
void run(funcPtr *start) {
    ip = start;
    running = 1;
    while (*ip) { (*ip++)(); }
    running = 0;
}

void doEXIT() { if (rsp) { ip = RPOP; } else { ip = &pgm[PGM_SZ]; *ip = 0; } }
void doCOL() { t = *(int*)(ip++); if (*ip != doEXIT) { RPUSH(ip); } ip = &pgm[t]; }
void doEXEC() {
    t = POP;
    funcPtr x = (funcPtr)POP;
    if (t & FLG_PRIM) { x(); }
    else {
        n = PGM_SZ-3;
        pgm[n] = doCOL;
        pgm[n+1] = x;
        pgm[n+2] = doEXIT;
        if (running) { RPUSH(ip); ip = &pgm[n]; }
        else { run(&pgm[n]); }
    }
}
void doNOP() { }
void doBREAK() { /* TODO! */ }
void doDUP() { t = TOS; PUSH(t); }
void doSWAP() { t = TOS; TOS = NOS; NOS = t; }
void doOVER() { t = NOS; PUSH(t); }
void doDROP() { if (sp) { DROP; } }
void doJMP() { ip = (funcPtr*)*ip; }
void doDO() { RPUSH(ip); LPUSH(NOS); LPUSH(TOS); DROP2; }
void doI() { PUSH(lstk[lsp]); }
void doJ() { PUSH(lstk[lsp-2]); }
void doLOOP() { if (++lstk[lsp] < lstk[lsp-1]) { ip = rstk[rsp]; } else { rsp--; lsp-=2; } }
void doUNLOOP() { if (1 < lsp) { lsp-=2; rsp--; } }
void doBEGIN() { RPUSH(ip); LPUSH(0); LPUSH(0); }
void doWHILE() { if (POP) { ip = rstk[rsp]; } else { rsp--; lsp -= 2; } }
void doUNTIL() { if (POP == 0) { ip = rstk[rsp]; } else { rsp--; lsp -= 2; } }
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
void doTIMER() { PUSH(GetTickCount()); }
void doLAST() { PUSH((CELL)&last); }
void doHERE() { PUSH((CELL)&here); }
void doWORDS() { for (int l = last; 0 <= l; l--) { printf("%s\t", dict[l].name); } }
void doIMMEDIATE() { dict[last].flags |= FLG_IMM; }
void doWORD() {
    char* wd = (char*)TOS;
    TOS = 0;
    while (*toIn && *toIn < 33) { ++toIn; }
    while (*toIn && *toIn > 32) { *(wd++) = *(toIn++); ++TOS; }
    *wd = 0;
}
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
    if (strCmp("[exec]",    wd) == 0) { state = STATE_EXEC;    PUSH(1); return; }
    if (strCmp("[compile]", wd) == 0) { state = STATE_COMPILE; PUSH(1); return; }
    if (strCmp("[comment]", wd) == 0) { state = STATE_COMMENT; PUSH(1); return; }
    if (strCmp("[create]",  wd) == 0) { state = STATE_CREATE;  PUSH(1); return; }

    if (state == STATE_COMMENT) { PUSH(1); return; }

    if (state == STATE_CREATE) {
        PUSH(here); PUSH((CELL)wd); doCREATE();
        PUSH(1); return;
    }
    
    PUSH((CELL)wd); doFIND();
    if (POP) {
        if (state==STATE_COMPILE) {
            if (TOS & FLG_PRIM) { pgm[here++]=(funcPtr)NOS; }
            else { pgm[here++]=doCOL; pgm[here++]=(funcPtr)NOS; }
            DROP2; 
        } else { doEXEC(); }
        PUSH(1); return;
    }

    PUSH((CELL)wd); doIsNum();
    if (POP) {
        if (state==STATE_COMPILE) { pgm[here++]=doLIT; pgm[here++]=(funcPtr)POP; }
        PUSH(1); return;
    }
    printf("[%s]??", wd);
    state = 0;
    PUSH(0);
}

int getWord(char *wd) {
    PUSH((CELL)wd);
    doWORD();
    return POP;
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
    int sz = strLen(src);
    for (int i = 0; i < sz; i++) {
        char *cp = &src[i], c = *cp;
        if (c == '[') {
            if (strCmpN("[create]",  cp, 8)==0) { FG(1); i += 8; continue; }
            if (strCmpN("[compile]", cp, 9)==0) { FG(3); i += 9; continue; }
            if (strCmpN("[comment]", cp, 9)==0) { FG(2); i += 9; continue; }
            if (strCmpN("[exec]",    cp, 6)==0) { FG(6); i += 6; continue; }
        }
        printf("%c", c);
    }
}

void primCreate(const char* nm, funcPtr xt) {
    PUSH((CELL)xt);
    PUSH((CELL)nm);
    doCREATE();
    dict[last].flags = FLG_PRIM;
}

void init() {
    here = sp = rsp = lsp = running = 0;
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
    primCreate("UNLOOP", doUNLOOP);
    primCreate("BEGIN", doBEGIN);
    primCreate("WHILE", doWHILE);
    primCreate("UNTIL", doUNTIL);
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
    primCreate("TIMER", doTIMER);
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
}
