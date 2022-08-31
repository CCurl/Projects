// pg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
// #include <time.h>

#define PGM_SZ   65535
#define DICT_SZ    511
#define VAR_SZ   65535
#define NAME_LEN    13

typedef unsigned long CELL;
typedef unsigned char byte;
typedef void (*funcPtr)();
typedef struct {
    CELL xt;
    byte flags;
    byte len;
    char name[NAME_LEN + 1];
} DICT_T;

#define TOS stk[sp]
#define NOS stk[sp-1]
#define PUSH(x) stk[++sp]=x
#define POP stk[sp--]
#define DROP sp--
#define DROP2 sp-=2
#define RPUSH(x) rstk[++rsp]=x
#define RPOP rstk[rsp--]
#define LPUSH(x) lstk[++lsp]=x
#define LPOP lstk[lsp--]

int here, sp, rsp, lsp, vhere, base, state, last;
funcPtr* ip, next;
CELL t, n, stk[32], lstk[32];
funcPtr pgm[PGM_SZ + 1], * rstk[32];
byte vars[VAR_SZ + 1];
DICT_T dict[DICT_SZ + 1];

void doNOP() { }
void doBREAK() { /* TODO! */ }
void doEXIT() { if (rsp) { ip = RPOP; } else { ip = &pgm[PGM_SZ]; *ip = 0; } }
void doCOL() { while ((*ip) && (*ip != doEXIT)) { (*ip++)(); } ip = RPOP; }
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
void doLIT() { PUSH((CELL) * (ip++)); }
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
void doIMMEDIATE() { dict[last].flags |= 0x80; }

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

int strCmp(char *src, const char *dst) {
    while ((*src) && (*dst) && (*src==*dst)) { ++src; ++dst; }
    return (*src)-(*dst);
}

void doCREATE() {
    ++last;
    dict[last].len = strCpy(dict[last].name, (char*)POP, NAME_LEN);
    dict[last].xt = POP;
    dict[last].flags = 0;
}

void primCreate(const char* nm, funcPtr xt) {
    PUSH((CELL)xt);
    PUSH((CELL)nm);
    doCREATE();
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

void init() {
    here = sp = rsp = lsp = 0;
    last = -1;
    for (int i = 0; i <= PGM_SZ; i++) { pgm[i] = 0; }

    primCreate("DUP", doDUP);
    primCreate("SWAP", doSWAP);
    primCreate("OVER", doOVER);
    //PUSH((CELL)"OVER");
    //doFIND();
    //if (POP) {
    //    doDOTD();
    //    doDOTH();
    //    printf("\n");
    //} else {
    //    printf("-nf-");
    //}

    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)12345;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)65;
    pgm[here++] = doSWAP;
    pgm[here++] = doDOTD;
    pgm[here++] = doEMIT;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)444555;
    pgm[here++] = do0BRANCH;
    pgm[here++] = 0;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)100;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)0;
    pgm[here++] = doDO;
    pgm[here++] = doI;
    pgm[here++] = doDOTD;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)32;
    pgm[here++] = doEMIT;
    pgm[here++] = doLOOP;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)10;
    pgm[here++] = doDUP;
    pgm[here++] = doDUP;
    pgm[here++] = doEMIT;
    pgm[here++] = doWORDS;
    pgm[here++] = doEMIT;
    pgm[here++] = 0;
}

int main()
{
    init();
    ip = &pgm[0];
    while (*ip) { (*ip++)(); }
}
