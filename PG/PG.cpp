// pg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include <Windows.h>

#define PGM_SZ 256
#define VAR_SZ 2560

typedef unsigned long CELL;
typedef void (*funcPtr)();
typedef struct {
    funcPtr xt;
    byte flags;
    byte sz;
    char name[14];
} DICT_T;

#define TOS stk[sp]
#define NOS stk[sp-1]
#define PUSH(x) stk[++sp]=x
#define POP stk[sp--]
#define DROP sp--
#define DROP2 sp-=2
#define RPUSH(x) rstk[++rsp]=x
#define RPOP rstk[rsp--]

int ip, here, sp, rsp, vhere, base, state;
funcPtr next;
CELL t, n, stk[32], rstk[32];
funcPtr pgm[PGM_SZ+1];
byte vars[VAR_SZ+1];

void doNOP() { }
void doBREAK() { /* TODO! */ }
void doRETURN() { if (rsp) { ip=RPOP; } else { ip=PGM_SZ; pgm[ip]=0; } }
void doCALL() { if (pgm[ip]!=doRETURN) { RPUSH(ip+1); } ip=(int)pgm[ip]; }
void doDUP() { t = TOS; PUSH(t); }
void doSWAP() { t = TOS; TOS = NOS; NOS = t; }
void doOVER() { t = NOS; PUSH(t); }
void doDROP() { if (sp) { DROP; } }
void doJMP() { ip = (CELL)pgm[ip]; }
void doDO() { RPUSH(ip); RPUSH(NOS); RPUSH(POP); DROP; }
void doI() { PUSH(rstk[rsp]); }
void doJ() { PUSH(rstk[rsp-4]); }
void doLOOP() { if (++rstk[rsp] < rstk[rsp-1]) { ip=rstk[rsp-2]; } else { rsp-=3; } }
void doBEGIN() { RPUSH(ip); }
void doWHILE() { if (POP) { ip=rstk[rsp]; } else { rsp--; } }
void doAGAIN() { ip=rstk[rsp]; }
void doLIT() { PUSH((CELL)pgm[ip++]); }
void do0BRANCH() { t = (CELL)pgm[ip++]; if (POP == 0) { ip = t; } }
void doDOTD() { printf("%d", POP); }
void doDOTH() { printf("%x", POP); }
void doEMIT() { printf("%c", POP); }
void doSEMI() { pgm[here++] = doRETURN; state = 0; }
void doFETCH() { TOS=*(CELL*)&vars[TOS]; }
void doCFETCH() { TOS = vars[TOS]; }
void doSTORE() { *(CELL*)&vars[TOS] = NOS; DROP2; }
void doCSTORE() { vars[TOS] = (byte)NOS; DROP2; }
void doEQ() { NOS = (NOS == TOS) ? 1 : 0; DROP; }
void doLT() { NOS = (NOS <  TOS) ? 1 : 0; DROP; }
void doGT() { NOS = (NOS >  TOS) ? 1 : 0; DROP; }
void doADD() { NOS += TOS; DROP; }
void doSUB() { NOS -= TOS; DROP; }
void doMUL() { NOS *= TOS; DROP; }
void doDIV() { NOS /= TOS; DROP; }
void doAND() { NOS &= TOS; DROP; }
void doOR()  { NOS |= TOS; DROP; }
void doXOR() { NOS ^= TOS; DROP; }
void doCOM() { TOS = ~TOS; }

void doCREATE() {
    // TODO!!
}

void doFIND() {
    // TODO!!
    PUSH(0);
}

void init() {
    here = sp = rsp = 0;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)12345;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)65;
    pgm[here++] = doSWAP;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)444555;
    pgm[here++] = do0BRANCH;
    pgm[here++] = (funcPtr)333;
    pgm[here++] = doLIT;
    pgm[here++] = (funcPtr)1000;
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
    pgm[here++] = doEMIT;
    pgm[here++] = doEMIT;
    pgm[here++] = doDOTD;
    pgm[here++] = doEMIT;
    pgm[here++] = 0;
}

int main()
{
    init();
    ip = 0;
    next = pgm[ip++];
    while (next) { next(); next = pgm[ip++]; }
}
