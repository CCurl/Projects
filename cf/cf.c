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
CELL TOS;
FILE* input_fp = 0;

#define NOS         stk[sp]
#define L0          lstk[lsp]
#define L1          lstk[lsp-1]
#define L2          lstk[lsp-2]
#define NEXT        goto Next
#define BTW(x,l,h)  ((l<=x)&&(x<=h))

void push(CELL x) { sp=(sp+1)&STK_SZ; stk[sp]=TOS; TOS=x; }
CELL pop() { CELL x=TOS; TOS=stk[sp]; sp=(sp-1)&STK_SZ; return x; }

void rpush(CELL v) { rsp=(rsp+1)&STK_SZ; rstk[++rsp]=v; }
CELL rpop() { CELL x=rstk[rsp]; rsp=(rsp-1)&STK_SZ; return x; }

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

void CCOMMA(CELL x) { *(here++)=(byte)(x); }
void COMMA(CELL val) { CS(here, val); here += CELL_SZ; }

byte *doQuote(byte *pc) {
    while ((*pc) && (*pc != '"')) {
        char c = *(pc++);
        if (c == '%') {
            c = *(pc++);
            if (c == 'd') { printStringF("%d", pop()); }
            else if (c == 'x') { printStringF("%x", pop()); }
            else if (c == 'n') { printString("\r\n"); }
            else if (c == 'b') { printChar(' '); }
            else if (c == 'c') { printChar(pop()); }
            else { printChar(c); }
        } else { printChar(c); }
    } ++pc;
    return pc;
}

void run(byte *pc) {
    CELL t1;
Next:
    if (!betw(pc,&user[0],&user[USER_SZ-1])) {
        printString("-oob-"); return;
    }
    switch (*(pc++)) {
    case 0:                                                         return;
    case 32:                                                        return;
    case '"': pc=doQuote(pc);                                       NEXT;
    case '%': t1 = NOS; push(t1);                                   NEXT;
    case '#': t1 = TOS; push(t1);                                   NEXT;
    case '$': t1 = TOS; TOS = NOS; NOS = t1;                        NEXT;
    case '\\': sp = (0<sp) ? sp-1: 0;                               NEXT;
    case '+': t1 = pop(); TOS += t1;                                NEXT;
    case '-': t1 = pop(); TOS -= t1;                                NEXT;
    case '*': t1 = pop(); TOS *= t1;                                NEXT;
    case '/': t1 = pop(); TOS /= (t1 ? t1 : 1);                     NEXT;
    case '=': t1 = pop(); TOS = (TOS == t1) ? 1 : 0;                NEXT;
    case '<': t1 = pop(); TOS = (TOS < t1) ? 1 : 0;                 NEXT;
    case '>': t1 = pop(); TOS = (TOS > t1) ? 1 : 0;                 NEXT;
    case '.': printStringF("%d ", pop());                           NEXT;
    case '4': push(CF(pc)); pc += CELL_SZ;                          NEXT;
    case 'e': printChar(pop());                                     NEXT;
    case ',': COMMA(pop());                                         NEXT;
    case '^': if (*(pc+CELL_SZ)!=';') { rpush((CELL)(pc+CELL_SZ)); }    // fall-thru
    case 'j': pc=AFA(pc);                                           NEXT;
    case ';': pc=(char*)rpop(); if (!pc) { rsp=0; return; };        NEXT;
    case 'Q': if (pop()) { pc=AFA(pc); } else { pc+=CELL_SZ; }      NEXT;
    case 'q': if (pop()==0) { pc=AFA(pc); } else { pc+=CELL_SZ; }   NEXT;
    case 'b': t1=*(pc++); if (t1=='!') { BS(TOS,NOS); sp-=2; }
        else if (t1=='@') { TOS=BF(TOS); }
        else if (t1==',') { CCOMMA(pop()); }                        NEXT;
    case 'c': t1=*(pc++); if (t1=='!') { CS(TOS,NOS); sp-=2; }
        else if (t1=='@') { TOS=CF(TOS); }
        else if (t1==',') { COMMA(pop()); }                         NEXT;
    case 'd': t1=*(pc++)-'0'; --regs[t1+rb];                        NEXT;
    case 'i': t1=*(pc++)-'0'; ++regs[t1+rb];                        NEXT;
    case 'r': t1=*(pc++)-'0'; push(regs[t1+rb]);                    NEXT;
    case 's': t1=*(pc++)-'0'; regs[t1+rb]=pop();                    NEXT;
    case 't': push(clock());                                        NEXT;
    case '[': lsp += 3; L0=pop(); L1=pop(); L2=(CELL)pc;            NEXT;
    case ']': if (++L0 < L1) { pc=(byte*)L2; } else { lsp-=3; }     NEXT;
    case 'I': push(L0);                                             NEXT;
    default: ERR("-ir-");                                           return;
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
    if ((wd[0]=='#') && (wd[1]) && (!wd[2])) { push(wd[1]); return 1; }
    if (*wd == '$') { base = 16;  ++wd; }
    if (*wd == '%') { base = 2;  ++wd; lastCh = '1'; }
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
    // printStringF("-def:%s-", wd);
    --last;
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
        if ((l==dp->l) && (strcmp(dp->name, wd) == 0)) {
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
        CCOMMA('4');
        COMMA(pop());
        return 1;
    }
    if (doFind(wd)) {
        DICT_T* dp = (DICT_T*)pop();
        CCOMMA('^');
        COMMA((CELL)dp->xt);
        return 1;
    }
    ERR("-"); ERR(wd); ERR("?-");
    return 0;
}

int doInterpret(const char* wd) {
    // printStringF("-interp:%s-", wd);
    byte *cp = here;
    if (isNum(wd)) {
        CCOMMA('4');
        COMMA(pop());
        return 1;
    }
    if (doFind(wd)) {
        DICT_T* dp = (DICT_T*)pop();
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

int strEq(const char *x, const char *y) { return strcmp(x, y) == 0; }

int setMode(char *wd) {
    if (strEq(wd, "(")) { mode=COMMENT; return 1; }
    if (strEq(wd, ":")) { mode=DEFINE; return 1; }
    if (strEq(wd, "::")) { mode=COMPILE; return 1; }
    if (strEq(wd, "_")) { mode=INTERP; return 1; }
    return 0;
}

void doOuter(char* cp) {
    char buf[32];
    in = cp;
    while (1) {
        if (getWord(buf) == 0) { return; }
        if (setMode(buf)) { continue; }
        switch (mode) {
        case COMMENT:                                   break;
        case DEFINE:  doDefine(buf);                    break;
        case COMPILE: if (!doCompile(buf)) return;      break;
        case INTERP:  if (!doInterpret(buf)) return;    break;
        default: break;
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
    CCOMMA('4');
    COMMA(val);
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
    if (fp == stdin) printf(" ok\r\n");
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
