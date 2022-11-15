// pg.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// #include <windows.h>
#include <stdio.h>
#include <inttypes.h>
#include <time.h>

typedef uint8_t byte;
typedef int32_t CELL;
typedef void (*funcPtr)();

#define CELL_SZ       sizeof(CELL)
#define FUNCPTR_SZ    sizeof(funcPtr)
#define MEM_SZ          65536
#define PGM_SZ        (MEM_SZ/FUNCPTR_SZ)
#define DICT_SZ           511
#define VAR_SZ          65535
#define SRC_SZ           8191
#define STK_SZ             64
#define NAME_MAX           15

// NB: sizeof(DICT_T) should be a multiple of (CELL_SZ)
struct DICT_E {
    DICT_E *next;
    byte flags;
    char name[NAME_MAX];
};

#define TOS       stk.i[sp]
#define NOS       stk.i[sp-1]
#define PUSH(x)   stk.i[++sp]=(CELL)(x)
#define POP       stk.i[sp--]
#define DROP      sp--
#define DROP2     sp-=2
#define RTOS      stk.p[rsp]
#define RPUSH(x)  stk.p[--rsp]=x
#define RPOP      stk.p[rsp++]
#define LPUSH(x)  lstk[++lsp]=x
#define LPOP      lstk[lsp--]

#define STATE_EXEC     0
#define STATE_COMPILE  1
#define STATE_COMMENT  2
#define STATE_DEFINE   3

#define FLG_IMM   0x80
#define FLG_PRIM  0x40

union { float f[STK_SZ+1]; CELL i[STK_SZ+1]; CELL p[STK_SZ+1]; } stk;
union { char *b; funcPtr p[PGM_SZ+1]; CELL i[PGM_SZ+1]; } st;
char *toIn, *tib, *here, isBYE = 0;
CELL t, lstk[32], vhere, base, state, n, sp, rsp, lsp, ip;
DICT_E *last;

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
int strCmpC(const char *src, const char *dst) {
    for (int i=0; i <=src[0]; i++) { if (src[i] != dst[i]) { return 0; } }
    return 1;
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

int running;
void run(CELL start) {
    ip = start;
    running = 1;
    while (*(funcPtr*)ip) {
        funcPtr x = *(funcPtr*)(ip);
        ip += CELL_SZ;
        x();
    }
    running = 0;
}

void iComma(CELL v) { *(CELL*)here = v; here += sizeof(CELL); }
void iCComma(CELL v) { *(byte*)here = (byte)v; ++here; }

void align() {
    CELL mod = (CELL)here % FUNCPTR_SZ;
    if (mod) {
        while (++mod < FUNCPTR_SZ) { here++; }
    }
}
void doBYE() { isBYE = 1; }
void doEXIT() { if (rsp<STK_SZ) { ip = RPOP; } else { ip = 0; } }
void doCOL() {
    RPUSH(ip+CELL_SZ);
    // if (*(funcPtr*)ip != doEXIT) { RPUSH(ip); }
    ip = *(CELL*)(ip);
}
void doEXEC() {
    funcPtr xt = (funcPtr)POP;
    xt = *(funcPtr*)xt;
    xt();
}
void doNOP() { }
void doBREAK() { /* TODO! */ }
void doDUP() { t = TOS; PUSH(t); }
void doSWAP() { t = TOS; TOS = NOS; NOS = t; }
void doOVER() { t = NOS; PUSH(t); }
void doDROP() { if (sp) { DROP; } }
void doJMP() { ip = 0; }
void doDO() { RPUSH(ip); LPUSH(NOS); LPUSH(TOS); DROP2; }
void doI() { PUSH(lstk[lsp]); }
void doJ() { PUSH(lstk[lsp-2]); }
void doUNLOOP() { if (1 < lsp) { lsp-=2; rsp++; } }
void doLOOP() { if (++lstk[lsp] < lstk[lsp-1]) { ip = RTOS; } else { doUNLOOP(); } }
void doBEGIN() { RPUSH(ip); LPUSH(0); LPUSH(0); }
void doWHILE() { if (POP) { ip=RTOS; } else { doUNLOOP(); } }
void doUNTIL() { if (POP==0) { ip=RTOS; } else { doUNLOOP(); } }
void doAGAIN() { ip=RTOS; }
void doLIT() { PUSH(*(CELL*)(ip++)); }
void do0BRANCH() { if (POP == 0) { ip = 0; } else { ++ip; } }
void doDOTD() { printf("%ld ", POP); }
void doDOTH() { printf("%lx ", POP); }
void doEMIT() { printf("%c", (char)POP); }
void doSEMI() { iComma((CELL)doEXIT); state = 0; }
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
void doLAST() { PUSH((CELL)&last); }
void doHERE() { PUSH((CELL)&here); }
void doIMMEDIATE() { last->flags |= FLG_IMM; }
void doTimer() { PUSH(clock()); }
void doWORD() {
    byte ch = (char)TOS, *wd = (byte*)(here+32);
    TOS = (CELL)wd;
    wd[0] = 0;
    while (*toIn && *toIn == ch) { ++toIn; }
    while (*toIn && *toIn != ch) { wd[++wd[0]] = *(toIn++); }
    wd[wd[0]+1] = 0;
    if (*toIn == ch) { ++toIn;  }
}
void doCREATE() {
    PUSH(32);
    doWORD();
    char *wd = (char*)POP;
    if (wd[0] == 0) { PUSH(0); return; }
    DICT_E *dp = (DICT_E*)here;
    for (int i=0; i<=wd[0]; i++) { dp->name[i] = wd[i]; }
    dp->next = last;
    dp->flags = 0;
    last = dp;
    here += sizeof(DICT_E);
    PUSH(here);
}
void doCOLON() {
    doCREATE();
    state = STATE_COMPILE;
    iComma((CELL)doCOL);
}
void doSEMICOLON() {
    iComma((CELL)doEXIT);
    state = STATE_EXEC;
}
void doFIND() {
    char *nm = (char*)TOS;
    int l = nm[0];
    TOS = 0;
    DICT_E *dp = last;
    while (dp) {
        if (strCmpC(nm, dp->name) == 0) { dp = dp->next;  continue; }
        TOS = (CELL)(dp);
        TOS += sizeof(DICT_E);
        PUSH(dp->flags);
        PUSH(1);
        return;
    }
}
void doIsNum() {
    char *wd = (char*)TOS+1, *end;
    intmax_t x = strtoimax(wd, &end, 0);
    if ((x == 0) && (strCmp(wd, "0"))) { TOS = 0; return; }
    TOS = (CELL)x;
    PUSH(1);
}
void doPARSE() {
    DICT_E* dp = (DICT_E*)here;
    dp->next = last;
    char *wd = (char*)POP;
    PUSH((CELL)wd); doIsNum();
    if (POP) {
        if (state==STATE_COMPILE) { iComma((CELL)doLIT); iComma(POP); }
        PUSH(1); return;
    }

    PUSH((CELL)wd); doFIND();
    if (POP == 0) {
        printf("[%s]??", wd);
        state = 0;
        PUSH(0); return;
    }

    CELL t1 = POP;
    CELL xt = POP;
    funcPtr xa = *(funcPtr*)xt;
    if ((state == STATE_EXEC) || (t1 & FLG_IMM)) {
        if (!running) {
            t1 = PGM_SZ - 4;
            st.p[t1] = xa;
            st.i[t1 + 1] = 0;
            run((CELL)&st.i[t1]);
        }
        else { xa(); }
    }
    else { iComma((CELL)xt); }
    PUSH(1); return;
}

void doCOMPILE() {
    toIn = &tib[0];
    PUSH(32);
    doWORD();
    while (isBYE == 0) {
        char *wd = (char*)TOS;
        if (wd[0] == 0) { DROP; return; }
        doPARSE();
        if (POP==0) { return; }
        PUSH(32);
        doWORD();
    }
}
void doLOAD() {
    char nm[32];
    tib = (here + 128);
#ifdef _WIN32
    sprintf_s(nm, 16, "blk-%03ld.4th", POP);
#else 
    sprintf(nm, "blk-%03ld.4th", POP);
#endif
    FILE *fp=fOpen(nm, "rb");
    if (fp) {
        int n = fread(tib, 1, SRC_SZ, fp);
        fclose(fp);
        for (int i = 0; i < n; i++) { if (tib[i]<32) { tib[i]=32; } }
    }
    else { printf("-nf-"); }
}

void primCreate(const char *nm, funcPtr xt) {
    DICT_E *de = (DICT_E*)here;
    de->next = last;
    de->flags = FLG_PRIM;
    de->name[0] = strLen(nm);
    strCpy(&de->name[1],nm,0);
    last = de;
    here += sizeof(DICT_E);
    iComma((CELL)xt);
}

void init() {
    sp = lsp = running = 0;
    rsp = STK_SZ;
    last = 0;
    here = st.b = (char*)&st.p[0];
    for (int i = 0; i <= PGM_SZ; i++) { st.p[i] = 0; }

    primCreate("BYE", doBYE);
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
    primCreate("(LAST)", doLAST);
    primCreate("(HERE)", doHERE);
    primCreate("IMMEDIATE", doIMMEDIATE);
    primCreate("TIMER", doTimer);
    primCreate(":", doCOLON);
    primCreate(";", doSEMICOLON); doIMMEDIATE();
}

void iCompile(const char* src) {
    tib = (here + 64);
    strCpy(tib, src, 0);
    doCOMPILE();
}
int main()
{
    init();
    iCompile(": cr 13 EMIT 10 EMIT ;");
    iCompile("cr 12345 . cr");
    return 0;
}
