#include "cf.h"

#if _MSC_VER
#include <conio.h>
int key() { return _getch(); }
int qKey() { return _kbhit(); }
#elif IS_LINUX
#include "linux.inc"
#endif

//char *pc, *in, state;
//byte user[USER_SZ];
//cell_t stk[STK_SZ+1], rstk[STK_SZ+1], lstk[STK_SZ+1];
//cell_t regs[100];
//int sp = 0, rsp = 0, lsp = 0, rb = 0;
//byte *here;
//DICT_T *last;
//cell_t TOS, A, B;
//FILE* input_fp = 0;

//#define NEXT        goto Next
//#define NCASE       NEXT; case
//#define BCASE       break; case

void printString(const char *s) { printf("%s", s); }
void printChar(char c) { printf("%c", c); }

#define ERR(x) printString(x);

#ifdef NEEDS_ALIGN
    #define WFO(l) (user[l+1]*256 | user[l])        // WordFromOffset
    #define WFA(l) ((*(l+1)*256) | (*l))            // WordFromAddress

    void WSA(byte* l, ushort v) { *(l+1)=v/256; *l=v%256; }
    void WSO(ushort l, ushort v) { user[l+1]=v/256; user[l]=v%256; }

    cell_t LFO(ushort l) {
        cell_t x = user[l++];
        x += (user[l++]<<8);
        x += (user[l++]<<8);
        x += (user[l]<<8);
        return x;
    }

    void LSO(ushort l, cell_t v) {
        user[l++]=(v%0xff); v=v>>8;
        user[l++]=(v%0xff); v=v>>8;
        user[l++]=(v%0xff); v=v>>8;
        user[l] = (v%0xff);
    }
#else
cell_t Fetch(const char *a) { return *(cell_t*)(a); }
void Store(const char *a, cell_t v) { *(cell_t*)(a) = v; }
#endif

char *doUser(char *pc, char ir) { return pc; }
cell_t sysTime() { return (cell_t)clock(); }

int doCompile(const char* wd) {
    // printStringF("-com:%s-", wd);
    if (isNum(wd)) {
        cell_t val = pop();
        if (BTW(val,0,127)) {
            CComma(LIT1);
            CComma(val);
        } else {
            CComma(LIT);
            Comma(val);
        }
        return 1;
    }
    if (doFind(wd)) {
        cell_t f = pop();
        cell_t xt = pop();
        if (f == 2) {    // INLINE
            byte *x=(byte *)xt;
            // printf("-inl-%d-",*x);
            CComma(*x++);
            while (*x != EXIT) { CComma(*(x++)); }
        } else {
            CComma(CALL);
            Comma(xt);
        }
        return 1;
    }
	printStringF("-C:%s?-", wd);
    return 0;
}

int doInterpret(const char* wd) {
    // printStringF("-interp:%s-", wd);
    if (isNum(wd)) { return 1; }
    if (doFind(wd)) {
        char *cp = here;
        cell_t f=pop();
        cell_t xt=pop();
        //printf("-I:%s/%lx-",wd,xt);
        CComma(CALL);
        Comma(xt);
        CComma(EXIT);
        here = cp;
        Run(here);
        return 1;
    }
	printStringF("-I:%s?-", wd);
    return 0;
}

int doML(char *wd) {
	if (isNum(wd)) { CComma(pop()); return 1; }
	printStringF("-M:%s?-", wd);
	return 0;
}

cell_t lastState;
int setMode(char *wd) {
    if ((state==COMMENT) && !strEq(wd, ")")) { return 1; }
    if (strEq(wd, ":"))  { state=COMPILE; doDefine(0);  return 1; }
    if (strEq(wd, ":I")) { state=COMPILE; doDefine(0); last->f=2; return 1; }
    if (strEq(wd, ":M")) { state=MLMODE;  doDefine(0); last->f=2; return 1; }
    if (strEq(wd, "["))  { state=INTERP;  return 1; }
    if (strEq(wd, "]"))  { state=COMPILE; return 1; }
    if (strEq(wd, "("))  { lastState=state; state=COMMENT; return 1; }
    if (strEq(wd, ")"))  { state=lastState; return 1; }
    return 0;
}

void doOuter(char* cp) {
    char buf[32];
    in = cp;
    while (nextWord(buf)) {
        if (setMode(buf)) { continue; }
        //printf("-%d/%s-",state,buf);
        switch (state) {
        case COMMENT:
        BCASE DEFINE:  doDefine(buf);
        BCASE COMPILE: if (!doCompile(buf)) return;
        BCASE INTERP:  if (!doInterpret(buf)) return;
        BCASE MLMODE:  if (!doML(buf)) return;
        break; default: ERR("-state?-"); break;
        }
    }
}

void defNum(char *name, cell_t val) {
    doDefine(name);
    if (BTW(val,0,127)) {
        CComma(LIT1);
        CComma(val);
    } else {
        CComma(LIT);
        Comma(val);
    }
    CComma(EXIT);
}

void parseF(char *fmt, ...) {
    char *buf = (char*)last;
    buf -= 256;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 200, fmt, args);
    va_end(args);
    doOuter(buf);
}

void initVM() {
    vmInit();
    state = INTERP;
    char *c1i = ":I %s #%ld ;";
    char *c1n = ": %s #%ld ;";
    char *m1i = ":M %s #%ld 3";
    char *m2i = ":M %s #%ld #%ld 3";

    parseF(m1i, ";", EXIT);
    parseF(m1i, "@", FETCH);
    parseF(m1i, "C@", CFETCH);
    parseF(m1i, "+", ADD);
    parseF(m1i, "-", SUB);
    parseF(m1i, "*", MULT);
    parseF(m1i, "/MOD", SLMOD);
    parseF(m1i, "DUP", DUP);
    parseF(m1i, "SWAP", SWAP);
    parseF(m1i, "DROP", DROP);
    parseF(m1i, "DO", DO);
    parseF(m1i, "LOOP", LOOP);
    parseF(m2i, "(.)", SYS_OPS, DOT);
    parseF(m2i, "EMIT", SYS_OPS, EMIT);
    parseF(m2i, "CLOCK", SYS_OPS, TIMER);

    parseF(c1i, "cell", CELL_SZ);
    parseF(c1n, "(here)",(cell_t)&here);
    parseF(c1n, "(last)",(cell_t)&last);
    parseF(c1n, "vars",(cell_t)&vars[0]);
    parseF(c1n, "vars-end",(cell_t)&vars[VARS_SZ]);
    doOuter(":I NIP SWAP DROP ;");
    doOuter(":I / /MOD NIP ;");
}

int loop() {
    cell_t fp = input_fp;
    char buf[128];
    if (fp == 0) { fp = (cell_t)stdin; }
    if (fp == (cell_t)stdin) { printf(" ok\r\n"); state=INTERP; }
    if (fgets(buf, 128, (FILE *)fp) == buf) {
        if (strcmp(rTrim(buf), "bye") == 0) { return 0; }
        doOuter(buf);
        return 1;
    }
    if (input_fp == (cell_t)stdin) { return 0; }
    fclose((FILE*)input_fp);
    input_fp = 0;
    return 1;
}

int main(int argc, char **argv) {
    initVM();
    input_fp = (cell_t)fopen("block-000.cf","rt");
    while (loop()) {}
    return 1;
}
