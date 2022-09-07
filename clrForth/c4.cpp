// c4.cpp: My color-forth inspired forth system

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

#define PGM_SZ   65535
#define DICT_SZ    511
#define VAR_SZ   65535
#define SRC_SZ    8191
#define NAME_LEN    13

typedef void (*funcPtr)();
typedef unsigned char byte;
#if UINTPTR_MAX == 0xFFFFFFFF
typedef int32_t CELL;
#else
typedef int64_t CELL;
#endif
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
#define BTW(x, a, b)  ((a<=x)&&(x<=b))
#define RET(x)    PUSH(x); return

#define STATE_EXEC     0
#define STATE_COMPILE  1
#define STATE_COMMENT  2
#define STATE_CREATE   3
#define STATE_VAR      4
#define STATE_CONST    5

#define FLG_IMM   0x80
#define FLG_PRIM  0x40

char sp, rsp, lsp, *toIn, src[SRC_SZ+1];
CELL stk[32], lstk[32];
CELL here, vhere, last, base, state, t, n;
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
    while (ip && *ip) { (*ip++)(); }
    running = 0;
}

void doEXIT() { if (rsp) { ip = RPOP; } else { ip = &pgm[PGM_SZ]; *ip = 0; } }
void doDotS() {
    printf("( "); for (int i = 1; i<=sp; i++) { printf("%ld ", stk[i]); }
    printf(")");
}
void doCOL() { t = *(int*)(ip++); if (*ip != doEXIT) { RPUSH(ip); } ip = &pgm[t]; }
void doEXEC() {
    t = POP;
    funcPtr fp = (funcPtr)POP;
    if (t & FLG_PRIM) { fp(); }
    else {
        n = PGM_SZ-3;
        pgm[n] = doCOL;
        pgm[n+1] = fp;
        pgm[n+2] = doEXIT;
        if (running) { RPUSH(ip); ip = &pgm[n]; }
        else { run(&pgm[n]); }
    }
}
void doNOP() { }
void doCOMMA() { pgm[here++]=(funcPtr)POP; }
void doVCOMMA() { *(CELL*)&vars[here++] = POP; vhere += sizeof(CELL); }
void doCCOMMA() { vars[vhere++]=(byte)POP; }
void doBREAK() { /* TODO! */ }
void doDUP() { t = TOS; PUSH(t); }
void doSWAP() { t = TOS; TOS = NOS; NOS = t; }
void doOVER() { t = NOS; PUSH(t); }
void doDROP() { if (sp) { DROP; } }
void doJMP() { ip = (funcPtr*)*ip; }
void doDO() { RPUSH(ip); LPUSH(NOS); LPUSH(TOS); DROP2; }
void doI() { PUSH((0 < lsp) ? lstk[lsp] : 0); }
void doJ() { PUSH((2 < lsp) ? lstk[lsp-2] : 0); }
void doUNLOOP() { if (1 < lsp) { lsp -= 2; rsp--; } }
void doLOOP() { if (++lstk[lsp] < lstk[lsp-1]) { ip = rstk[rsp]; } else { doUNLOOP(); } }
void doBEGIN() { RPUSH(ip); LPUSH(0); LPUSH(0); }
void doWHILE() { if (POP) { ip=rstk[rsp]; } else { doUNLOOP(); } }
void doUNTIL() { if (POP==0) { ip=rstk[rsp]; } else { doUNLOOP(); } }
void doAGAIN() { ip = rstk[rsp]; }
void doLIT() { PUSH((CELL)*(ip++)); }
void do0BRANCH() { if (POP == 0) { ip = (funcPtr*)*ip; } else { ++ip; } }
void doDOTD() { printf("%ld ", POP); }
void doDOTH() { printf("%lx ", POP); }
void doEMIT() { printf("%c", (char)POP); }
void doSEMI() { pgm[here++] = doEXIT; state = 0; }
void doFETCH() { TOS = *(CELL*)TOS; }
void doCFETCH() { TOS = *(byte*)TOS; }
void doSTORE() { *(CELL*)TOS = NOS; DROP2; }
void doCSTORE() { *(byte*)TOS = (byte)NOS; DROP2; }
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
void doLAST() { PUSH((CELL)&dict[last]); }
void doTIMER() { PUSH(clock()); }
void doHERE() { PUSH((CELL)&pgm[here]); }
void doWORDS() { for (CELL l = last; 0 <= l; l--) { printf("%s\t", dict[l].name); } }
void doIMMEDIATE() { dict[last].flags |= FLG_IMM; }
void doCELL() { PUSH(sizeof(CELL)); }
void doIF() { if (state==STATE_COMPILE) { pgm[here++]=do0BRANCH; doHERE(); pgm[here++]=0; } }
void doTHEN() { if (state == STATE_COMPILE) { doHERE(); doSWAP(); doSTORE(); } }
void doELSE() {
    if (state == STATE_COMPILE) {
        pgm[here++] = doJMP;
        CELL x = here;
        pgm[here++] = 0;
        doHERE();
        doSWAP();
        doSTORE();
        PUSH((CELL)&pgm[x]);
    }
}
void doWORD() {
    byte *wd = &vars[vhere];
    TOS = (CELL)wd;
    while (*toIn && *toIn < 33) { ++toIn; }
    while (*toIn && *toIn > 32) { *(wd++) = *(toIn++); }
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
    for (CELL i = last; 0 <= i; i--) {
        if (l != dict[i].len) { continue; }
        if (strCmp(nm, dict[i].name)) { continue; }
        TOS = dict[i].xt;
        PUSH(dict[i].flags);
        RET(1);
    }
}
void doIsNum() {
    char *wd = (char*)POP;
    CELL x = 0, b = base, isNeg = 0, lastCh = '9';
    if ((wd[0] == '\'') && (wd[2] == wd[0]) && (wd[3] == 0)) { PUSH(wd[1]); RET(1); }
    if (*wd == '#') { b = 10;  ++wd; }
    else if (*wd == '$') { b = 16;  ++wd; }
    else if (*wd == '%') { b = 2;  ++wd; lastCh = '1'; }
    if (b < 10) { lastCh = '0' + b - 1; }
    if ((*wd == '-') && (b == 10)) { isNeg = 1;  ++wd; }
    if (*wd == 0) { RET(0); }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((b == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((b == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { RET(0); }
        x = (x * b) + t;
    }
    if (isNeg) { x = -x; }
    PUSH(x);
    RET(1);
}
void doBYE() { exit(0); }
void doPARSE() {
    char *wd = (char*)POP;
    if (strCmp("[exec]",    wd) == 0) { state = STATE_EXEC;    RET(1); }
    if (strCmp("[compile]", wd) == 0) { state = STATE_COMPILE; RET(1); }
    if (strCmp("[comment]", wd) == 0) { state = STATE_COMMENT; RET(1); }
    if (strCmp("[create]",  wd) == 0) { state = STATE_CREATE;  RET(1); }
    if (strCmp("[var]",     wd) == 0) { state = STATE_VAR;     RET(1); }
    if (strCmp("[const]",   wd) == 0) { state = STATE_CONST;   RET(1); }

    if (state == STATE_COMMENT) { RET(1); }

    if (state == STATE_CREATE) { PUSH(here); PUSH((CELL)wd); doCREATE(); RET(1); }

    if (state == STATE_VAR) {
        PUSH(here); PUSH((CELL)wd); doCREATE();
        pgm[here++] = doLIT;
        pgm[here++] = (funcPtr)&vars[vhere];
        pgm[here++] = doEXIT;
        vhere += sizeof(CELL);
        RET(1);
    }

    if (state == STATE_CONST) {
        PUSH(here); PUSH((CELL)wd); doCREATE();
        pgm[here++] = doLIT;
        pgm[here++] = (funcPtr)POP;
        pgm[here++] = doEXIT;
        RET(1);
    }

    PUSH((CELL)wd); doFIND();
    if (POP) {
        if ((state == STATE_COMPILE) && (TOS & FLG_IMM)) {
            int x = 1;
        }
        if ((state == STATE_EXEC) || (TOS & FLG_IMM)) {
                doEXEC();
        } else if (state==STATE_COMPILE) {
            if (TOS & FLG_PRIM) { pgm[here++] = (funcPtr)NOS; }
            else { pgm[here++] = doCOL; pgm[here++] = (funcPtr)NOS; }
            DROP2;
        }
        RET(1);
    }

    PUSH((CELL)wd); doIsNum();
    if (POP) {
        if (state==STATE_COMPILE) { pgm[here++]=doLIT; pgm[here++]=(funcPtr)POP; }
        RET(1);
    }
    printf("[%s]??", wd);
    state = 0;
    RET(0);
}

void doCOMPILE() {
    toIn = &src[0];
    while (1) {
        PUSH(32);
        doWORD();
        if (strLen((char*)TOS) == 0) { DROP; return; }
        doPARSE();
        if (POP==0) { return; }
    }
}
void doLOAD() {
    char nm[32];
#ifdef _WIN32
    sprintf_s(nm, 16, "blk-%03ld.c4", POP);
#else 
    sprintf(nm, "blk-%03ld.4th", POP);
#endif
    FILE *fp=fOpen(nm, "rb");
    if (fp) { size_t x=fread(src, 1, SRC_SZ, fp); fclose(fp); }
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
            if (strCmpN("[exec]",    cp, 6)==0) { FG(7); i += 6; continue; }
            if (strCmpN("[var]",     cp, 5)==0) { FG(6); i += 5; continue; }
            if (strCmpN("[const]",   cp, 7)==0) { FG(5); i += 7; continue; }
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
    base = 10;
    for (int i = 0; i <= PGM_SZ; i++) { pgm[i] = 0; }
    for (int i = 0; i <= VAR_SZ; i++) { vars[i] = 0; }
    for (int i = 0; i <= SRC_SZ; i++) { src[i] = 0; }

    primCreate("CELL", doCELL);
    primCreate("RET", doEXIT);
    primCreate("EXEC", doEXEC);
    primCreate("DUP", doDUP);
    primCreate("SWAP", doSWAP);
    primCreate("OVER", doOVER);
    primCreate("DROP", doDROP);
    primCreate("JMP", doJMP);
    primCreate("WORD", doWORD);
    primCreate("'", doFIND);
    primCreate("[']", doFIND); doIMMEDIATE();
    primCreate("DO", doDO);
    primCreate("I", doI);
    primCreate("J", doJ);
    primCreate("LOOP", doLOOP);
    primCreate("UNLOOP", doUNLOOP);
    primCreate("BEGIN", doBEGIN);
    primCreate("WHILE", doWHILE);
    primCreate("UNTIL", doUNTIL);
    primCreate("AGAIN", doAGAIN);
    primCreate("TIMER", doTIMER);
    primCreate(".", doDOTD);
    primCreate(".h", doDOTH);
    primCreate("EMIT", doEMIT);
    primCreate("@", doFETCH);
    primCreate("C@", doCFETCH);
    primCreate("!", doSTORE);
    primCreate("C!", doCSTORE);
    primCreate(",", doCOMMA);
    primCreate("V,", doVCOMMA);
    primCreate("C,", doCCOMMA);
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
    primCreate("IF", doIF); doIMMEDIATE();
    primCreate("ELSE", doELSE); doIMMEDIATE();
    primCreate("THEN", doTHEN); doIMMEDIATE();
    primCreate("EDIT", doEdit);
    primCreate(".S", doDotS);
    primCreate("BYE", doBYE);
}

int main()
{
    init();
    PUSH(0);
    doLOAD();
    doCOMPILE();
    while (1) {
        printf(" OK\n");
        fgets(src, SRC_SZ, stdin);
        state = STATE_EXEC;
        doCOMPILE();
    }
}
