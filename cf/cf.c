#include "cf.h"

#if _MSC_VER
#include <conio.h>
int key() { return _getch(); }
int qKey() { return _kbhit(); }
#elif LINUX
#include "linux.inc"
#endif

char *pc, *in, mode;
byte user[USER_SZ];
CELL stk[STK_SZ+1], rstk[STK_SZ+1], lstk[STK_SZ+1];
CELL regs[100];
int sp = 0, rsp = 0, lsp = 0, rb = 0;
byte *here;
DICT_T *last;
CELL TOS, A, B;
FILE* input_fp = 0;

#define NOS         stk[sp]
#define L0          lstk[lsp]
#define L1          lstk[lsp-1]
#define L2          lstk[lsp-2]
#define NEXT        goto Next
#define NCASE       NEXT; case
#define BCASE       break; case

void push(CELL x) { sp=(sp+1)&STK_SZ; stk[sp]=TOS; TOS=x; }
CELL pop() { CELL x=TOS; TOS=stk[sp]; sp=(sp-1)&STK_SZ; return x; }

void rpush(CELL v) { rstk[++rsp]=v; }
CELL rpop() { return (0<rsp) ? rstk[rsp--] : 0; }

void printString(const char *s) { printf("%s", s); }
void printChar(char c) { printf("%c", c); }

#define ERR(x) printString(x);

void printStringF(const char* fmt, ...) {
    char buf[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

#ifdef NEEDS_ALIGN
    #define WFO(l) (user[l+1]*256 | user[l])        // WordFromOffset
    #define WFA(l) ((*(l+1)*256) | (*l))            // WordFromAddress

    void WSA(byte* l, ushort v) { *(l+1)=v/256; *l=v%256; }
    void WSO(ushort l, ushort v) { user[l+1]=v/256; user[l]=v%256; }

    CELL LFO(ushort l) {
        CELL x = user[l++];
        x += (user[l++]<<8);
        x += (user[l++]<<8);
        x += (user[l]<<8);
        return x;
    }

    void LSO(ushort l, CELL v) {
        user[l++]=(v%0xff); v=v>>8;
        user[l++]=(v%0xff); v=v>>8;
        user[l++]=(v%0xff); v=v>>8;
        user[l] = (v%0xff);
    }
#else
    #define CF(a)     *(CELL*)(a)               // CellFetch
    #define CS(a, v)  *(CELL*)(a) = v           // CellSet
    #define BF(a)     *(byte*)(a)               // ByteFetch
    #define BS(a, v)  *(byte*)(a) = (byte)v     // ByteSet
    #define AFA(a)    *(byte**)(a)              // AddressFromAddress
#endif

void CCOMMA(CELL x) { BS(here, x); here += 1; }
void COMMA(CELL val) { CS(here, val); here += CELL_SZ; }

int strEq(const char *x, const char *y) { return strcmp(x, y) == 0; }

byte *doQuote(byte *y) {
    while ((*y) && (*y != '"')) {
        char c = *(y++);
        if (c == '%') {
            c = *(y++);
            if (c == 'd') { printStringF("%d", pop()); }
            else if (c == 'x') { printStringF("%x", pop()); }
            else if (c == 'n') { printString("\r\n"); }
            else if (c == 'b') { printChar(' '); }
            else if (c == 'e') { printChar(27); }
            else if (c == 'c') { printChar(pop()); }
            else { printChar(c); }
        } else { printChar(c); }
    } ++y;
    return y ? y+1 : y;
}

void run(byte *pc) {
    CELL t1;
Next:
    if (!betw(pc,&user[0],&user[USER_SZ-1])) {
        printString("-PC oob-"); return;
    }
    // printf("-ir:%c-",*pc);
    switch (*(pc++)) {
    case 0:    return;
    case 32:   return;
    case '"': pc=doQuote(pc);
    NCASE '!': t1=*(pc++); if (t1=='4') { t1=pop(); CS(t1,pop()); }
        else if (t1=='1') { t1=pop(); BS(t1,pop()); }
        else if (t1=='a') { CS(A,TOS); }
        else if (t1=='b') { CS(B,TOS); }
    NCASE '@': t1=*(pc++); if (t1=='4') { TOS=CF(TOS); }
        else if (t1=='1') { TOS=BF(TOS); }
        else if (t1=='a') { push(CF(A)); }
        else if (t1=='b') { push(CF(B)); }
    NCASE '%': t1 = NOS; push(t1);
    NCASE '#': t1 = TOS; push(t1);
    NCASE '$': t1 = TOS; TOS = NOS; NOS = t1;
    NCASE '\\': sp = (0<sp) ? sp-1: 0;
    NCASE '+': t1 = pop(); TOS += t1;
    NCASE '-': t1 = pop(); TOS -= t1;
    NCASE '*': t1 = pop(); TOS *= t1;
    NCASE '/': t1 = pop(); TOS /= (t1 ? t1 : 1);
    NCASE '=': t1 = pop(); TOS = (TOS == t1) ? 1 : 0;
    NCASE '<': t1 = pop(); TOS = (TOS < t1) ? 1 : 0;
    NCASE '>': t1 = pop(); TOS = (TOS > t1) ? 1 : 0;
    NCASE '.': printStringF("%ld ", pop());
    NCASE '1': push(BF(pc)); pc += 1;
    NCASE '4': push(CF(pc)); pc += CELL_SZ;
    NCASE 'e': printChar(pop());
    NCASE ',': t1=*(pc++); if (t1=='4') { COMMA(pop()); }
        else if (t1=='1') { CCOMMA(pop()); }
    NCASE '^': if (*(pc+CELL_SZ)!=';') { rpush((CELL)(pc+CELL_SZ)); }    // fall-thru
    case  'j': pc=AFA(pc);
    NCASE ';': pc=(byte*)rpop(); if (!pc) { rsp=0; return; };
    NCASE 'Q': if (pop()) { pc=AFA(pc); } else { pc+=CELL_SZ; }
    NCASE 'q': if (pop()==0) { pc=AFA(pc); } else { pc+=CELL_SZ; }
    NCASE 'b': t1=*(pc++); if (t1=='!') { BS(TOS,NOS); sp-=2; }
        else if (t1=='@') { TOS=BF(TOS); }
        else if (t1==',') { CCOMMA(pop()); }
    NCASE 'c': t1=*(pc++); if (t1=='!') { CS(TOS,NOS); sp-=2; }
        else if (t1=='@') { TOS=CF(TOS); }
        else if (t1==',') { COMMA(pop()); }
    NCASE 'd': t1=*(pc++)-'0'; --regs[t1+rb];
    NCASE 'i': t1=*(pc++)-'0'; ++regs[t1+rb];
    NCASE 'r': t1=*(pc++)-'0'; push(regs[t1+rb]);
    NCASE 's': t1=*(pc++)-'0'; regs[t1+rb]=pop();
    NCASE 't': push(clock());
    NCASE '[': lsp += 3; L0=pop(); L1=pop(); L2=(CELL)pc;
    NCASE ']': if (++L0 < L1) { pc=(byte*)L2; } else { lsp-=3; }
    NCASE 'I': push(L0);
    NEXT; default: ERR("-ir-");  return;
    }
}

int getWord(char *buf) {
    int l = 0;
    while (*in && (*in < 33)) { ++in; }
    while (*in && (*in > 32)) { buf[l++] = *(in++); }
    buf[l] = 0;
    return l;
}

int isNum(const char* wd) {
    CELL x = 0;
    int base = 10, isNeg = 0, lastCh = '9';
    if ((wd[0]==39) && (wd[1]) && (!wd[2])) { push(wd[1]); return 1; }
    if (*wd == '#') { base = 10; ++wd; }
    if (*wd == '$') { base = 16; ++wd; }
    if (*wd == '%') { base =  2; ++wd; lastCh = '1'; }
    if ((*wd == '-') && (base == 10)) { isNeg = 1;  ++wd; }
    if (*wd == 0) { return 0; }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (betw(c, '0', lastCh)) { t = c - '0'; }
        if ((base == 16) && (betw(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((base == 16) && (betw(c, 'a', 'a'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x*base) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

void doDefine(const char* wd) {
    // printStringF("\n-def:%s/%p-", wd, here);
    --last;
    last->f = 0;
    last->l = (byte)strlen(wd);
    last->xt = here;
    strcpy(last->name, wd);
}

// find (cp--[dp 1]|0)
int doFind(const char* wd) {
    // printStringF("-find:%s-", wd);
    int l = strlen(wd);
    DICT_T *dp = last;
    while (dp < (DICT_T*)&user[USER_SZ]) {
        if ((l == dp->l) && strEq(dp->name, wd)) {
            push((CELL)dp);
            return 1;
        }
        ++dp;
    }
    return 0;
}

int doCompile(const char* wd) {
    // printStringF("-com:%s-", wd);
    if (isNum(wd)) {
        CELL val = pop();
        if (betw(val,0,127)) {
            CCOMMA('1');
            CCOMMA(val);
        } else {
            CCOMMA('4');
            COMMA(val);
        }
        return 1;
    }
    if (doFind(wd)) {
        DICT_T* dp = (DICT_T*)pop();
        if (dp->f == 2) {    // INLINE
            byte *x=dp->xt;
            // printf("-inl-%d-",*x);
            CCOMMA(*x++);
            while (*x != ';') { CCOMMA(*(x++)); }
        } else {
            CCOMMA('^');
            COMMA((CELL)dp->xt);
        }
        return 1;
    }
    ERR("-"); ERR(wd); ERR("?-");
    return 0;
}

int doInterpret(const char* wd) {
    // printStringF("-interp:%s-", wd);
    if (isNum(wd)) { return 1; }
    if (doFind(wd)) {
        byte *cp = here;
        DICT_T* dp = (DICT_T*)pop();
        //printf("-I:%s/%p-",wd,dp->xt);
        CCOMMA('^');
        COMMA((CELL)dp->xt);
        CCOMMA(';');
        here = cp;
        run(here);
        return 1;
    }
    ERR("-"); ERR(wd); ERR("?-");
    return 0;
}

int doML(char *wd) {
	while (*wd) { CCOMMA(*(wd++)); }
	return 1;
}

int setMode(char *wd) {
    if (strEq(wd, ":"))  { mode=DEFINE;  return 1; }
    if (strEq(wd, "("))  { mode=COMMENT; return 1; }
    if (strEq(wd, ":C")) { mode=COMPILE; return 1; }
    if (strEq(wd, ":i")) { mode=INTERP;  return 1; }
    if (strEq(wd, ":M")) { mode=MLMODE;  return 1; }
    if (strEq(wd, ":I")) { last->f=2;    return 1; }  // Mark INLINE
    return 0;
}

void doOuter(char* cp) {
    char buf[32];
    in = cp;
    while (getWord(buf)) {
        if (setMode(buf)) { continue; }
        //printf("-%d/%s-",mode,buf);
        switch (mode) {
        case COMMENT:
        BCASE DEFINE:  doDefine(buf);
        BCASE COMPILE: if (!doCompile(buf)) return;
        BCASE INTERP:  if (!doInterpret(buf)) return;
        BCASE MLMODE:  if (!doML(buf)) return;
        break; default: ERR("-mode?-"); break;
        }
    }
}

char *rTrim(char *cp) {
    char *p = cp;
    while (*p) { p++; }
    while ((*p < 32) && (cp <= p)) { *(p--) = 0; }
    return cp;
}

void defNum(char *name, CELL val) {
    doDefine(name);
    if (betw(val,0,127)) {
        CCOMMA('1');
        CCOMMA(val);
    } else {
        CCOMMA('4');
        COMMA(val);
    }
    CCOMMA(';');
}

void initVM() {
    last = (DICT_T*)&user[USER_SZ];
    here = &user[0];
    sp = rsp = lsp = rb = 0;
    mode = INTERP;
    defNum("cell",CELL_SZ);
    defNum("(here)",(CELL)&here);
    defNum("(last)",(CELL)&last);
    defNum("user",(CELL)&user[0]);
    defNum("user-end",(CELL)&user[USER_SZ]);
}

int loop() {
    FILE* fp = input_fp;
    char buf[128];
    if (fp == 0) { fp = stdin; }
    if (fp == stdin) { printf(" ok\r\n"); mode=INTERP; }
    if (fgets(buf, 128, fp) == buf) {
        if (strcmp(rTrim(buf), "bye") == 0) { return 0; }
        doOuter(buf);
        return 1;
    }
    if (input_fp == stdin) { return 0; }
    fclose(input_fp);
    input_fp = 0;
    return 1;
}

int main(int argc, char **argv) {
    initVM();
    input_fp = fopen("block-000.cf","rt");
    while (loop()) {}
    return 1;
}
