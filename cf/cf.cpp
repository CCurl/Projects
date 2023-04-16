#include "cf.h"

enum {
    STOP, NOOP, RET, CALL, BRANCH0, BRANCHG0,
    LIT1, LIT4, FETCH1, FETCH4, STORE1, STORE4
};

char *pc, *toIn;
byte user[USER_SZ];
CELL stk[STK_SZ+1];
CELL rstk[STK_SZ+1];
char sp = 0, rsp = 0;
ushort here, last, isOK;
CELL t1;

#define TOS stk[sp]
#define NOS stk[sp-1]
#define NEXT goto Next

void push(CELL v) { if (sp < STK_SZ) { stk[++sp] = v; } }
CELL pop() { return (sp) ? stk[sp--] : 0; }

void rpush(CELL v) { if (rsp < STK_SZ) { rstk[++rsp] = v; } }
CELL rpop() { return (rsp) ? rstk[rsp--] : 0; }

int charAvailable() { return _kbhit(); }
int getChar() { return _getch(); }
void printString(const char *s) { printf("%s", s); }
void printChar(char c) { printf("%c", c); }

#define ERR(x) printString(x); isOK = 0

void printStringF(const char* fmt, ...) {
    char buf[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

#define WFO(l) (user[l+1]*256 | user[l])
#define WFA(l) ((*(l+1)*256) | (*l))

#ifdef NEEDS_ALIGN
void wsa(byte* l, ushort v) { *(l+1)=v/256; *l=v%256; }
void wso(ushort l, ushort v) { user[l+1]=v/256; user[l]=v%256; }

CELL lfo(ushort l) {
    CELL x = user[l++];
    x += (user[l++]<<8);
    x += (user[l++]<<8);
    x += (user[l]<<8);
    return x;
}

void lso(ushort l, CELL v) {
    user[l++]=(v%0xff); v=v>>8;
    user[l++]=(v%0xff); v=v>>8;
    user[l++]=(v%0xff); v=v>>8;
    user[l] = (v%0xff);
}
#else
void wsa(byte* l, ushort v) { *(ushort*)l = v; }
void wso(ushort l, ushort v) { *(ushort*)&user[l] = v; }
CELL lfo(ushort l) { return *(CELL*)(&user[l]); }
void lso(ushort l, CELL v) { *(CELL*)(&user[l]) = v; }
#endif

#define CCM(x) user[here++]=(byte)(x)

void run(ushort pc) {
    isOK = 1;
    Next:
    byte ir = user[pc++];
    switch (ir) {
    case STOP: return;
    case NOOP: return;
    case '"': while ((user[pc]) && (user[pc] != '"')) {
            char c = user[pc++];
            if (c == '%') {
                c = user[pc++];
                if (c == 'd') { printStringF("%d", pop()); }
                else if (c == 'x') { printStringF("%x", pop()); }
                else if (c == 'n') { printString("\r\n"); }
                else if (c == 'b') { printChar(' '); }
                else if (c == 'c') { printChar(pop()); }
                else { printChar(c); }
            } else { printChar(c); }
        } ++pc;                                            NEXT;
    case '+': t1 = pop(); TOS += t1;                       NEXT;
    case '-': t1 = pop(); TOS -= t1;                       NEXT;
    case '*': t1 = pop(); TOS *= t1;                       NEXT;
    case '/': t1 = pop(); TOS /= (t1 ? t1 : 1);            NEXT;
    case '=': t1 = pop(); TOS = (TOS == t1) ? 1 : 0;       NEXT;
    case '<': t1 = pop(); TOS = (TOS < t1) ? 1 : 0;        NEXT;
    case '>': t1 = pop(); TOS = (TOS > t1) ? 1 : 0;        NEXT;
    case '.': printStringF("%d ", pop());                  NEXT;
    case ',': printChar(pop());                            NEXT;
    case 'c':  rpush(pc+2); pc = WFO(pc);                  NEXT;
    case 'l':  push(lfo(pc)); pc += CELL_SZ;               NEXT;
    case ';': t1 = rpop(); if (!t1) { rsp = 0; return; }
        pc = t1;                                           NEXT;
    default: ERR("-ir-");                                  return;
    }
}

char peekCh() { return *toIn; }

char getOneCh() { return *(toIn) ? *(toIn++) : 0; }

int getWord(char* buf) {
    char* b = buf;
    int l = 0;
    while (peekCh() == ' ') { getOneCh(); }
    while (' ' < peekCh()) {
        *(b++) = getOneCh();
        l++;
    }
    *b = 0;
    return l;
}

// find (cp--dp f?)
void doFind(const char* wd) {
    // printStringF("-find:%s-", wd);
    push(0);
    ushort cp = last;
    while (cp && (cp < USER_SZ)) {
        DICT_T* dp = (DICT_T*)&user[cp];
        if (dp->name[0] == 0) { return; }
        if (strcmp(dp->name, wd) == 0) {
            TOS = (CELL)dp;
            push(1);
            break;
        }
        cp += dp->sz;
    }
}

int isNum(const char* wd) {
    CELL x = 0;
    int base = 10, isNeg = 0, lastCh = '9';
    if (*wd == '$') { base = 16;  ++wd; }
    if (*wd == '%') { base = 2;  ++wd; lastCh = '1'; }
    if (*wd == '-') { isNeg = 1;  ++wd; }
    if (*wd == 0) { return 0; }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (betw(c, '0', lastCh)) { t = c - '0'; }
        if ((base == 16) && (betw(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((base == 16) && (betw(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if (t < 0) { return 0; }
        x = (x*base) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

void doDefine(const char* wd) {
    // printStringF("-def:%s-", wd);
    CCM(';');
    int sz = strlen(wd) + 4;
    while (sz % 4) { ++sz; }
    ushort newLast = last - sz;
    if (newLast <= here) { ERR("-df-"); return; }
    DICT_T* dp = (DICT_T*)&user[newLast];
    dp->sz = sz;
    dp->xt = here;
    strcpy(dp->name, wd);
    last = newLast;
}

void doCompile(const char* wd) {
    // printStringF("-com:%s-", wd);
    doFind(wd);
    if (pop()) {
        DICT_T* dp = (DICT_T*)pop();
        CCM('c');
        wso(here, dp->xt);
        here += 2;
        return;
    }
    if (isNum(wd)) {
        CELL x = pop();
        CCM('l');
        lso(here, x);
        here += CELL_SZ;
        return;
    }
    while (*wd) { CCM(*(wd++)); }
}

int doInterpret(const char* wd) {
    // printStringF("-int:%s-", wd);
    if (isNum(wd)) { return 1; }
    doFind(wd);
    if (pop()) {
        DICT_T* dp = (DICT_T*)pop();
        run(dp->xt);
        return 1;
    }
    if (strcmp(wd,"edit")==0) {
        doEditor();
        return 1;
    }
    byte* cp = &user[here];
    while (*wd) { *(cp++) = *(wd++); }
    *cp = ';';
    run(here);
    return 0;
}

void doAsm(const char* wd) {
    while (*wd) { CCM(*(wd++)); }
}

void doOuter(char* cp) {
    char buf[32];
    toIn = cp;
    int mode = INTERP;
    while (1) {
        char c = peekCh();
        if (!c) { return; }
        if (betw(c, 1, 7)) { mode = getOneCh(); continue; }
        if (getWord(buf) == 0) { continue; }
        switch (mode) {
        case DEFINE:  doDefine(buf);                    break;
        case COMPILE: doCompile(buf);                   break;
        case INTERP:  doInterpret(buf);                 break;
        case ASM:     doAsm(buf);                       break;
        case COMMENT:                                   break;
        default: break;
        }
    }
}

char* rtrim(char *cp) {
    char* p = cp;
    if (*p == 0) { return cp; }
    while (*(p+1)) { p++; }
    while ((*p < 32) && (cp <= p)) { *(p--) = 0; }
    return cp;
}

void initVM() {
    last = USER_SZ-1;
    here = 0;
}

int loop() {
    char buf[96];
    printf(" ok\r\n>");
    fgets(buf, 96, stdin);
    if (strcmp(rtrim(buf), "edit") == 0) {
        if (sp==0) { push(-1); }
        doEditor();
        initVM();
        doOuter(theBlock);
    } else if (strcmp(buf, "bye") == 0) {
        return 0;
    } else {
        doOuter(buf);
    }
    return 1;
}

int main(int argc, char **argv) {
    initVM();
    while (loop()) {}
    return 1;
}
