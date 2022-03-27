// MinForth.cpp : An extremely memory conscious Forth interpreter

#include "Shared.h"
#include <stdio.h>

typedef struct {
    const char *name;
    const char *op;
} PRIM_T;

// Words that directly map to VM operations
PRIM_T prims[] = {
    {"+","+"}, {"-","-"}, {"/","/"}, {"*","*"},{"/mod","&"},
    {"swap","$"}, {"drop","\\"}, {"over","%"}, {"dup","#"},
    {"emit",","}, {".","."}, {"space","b"}, {"cr","n"},
    {"=","="}, {"<","<"}, {">",">"}, {"0=","N"},
    {"<<","L"}, {">>","R"},
    {"@","@"}, {"c@","c"}, {"w@","w"}, {"!","!"}, {"c!","C"}, {"w!","W"},
    {"and","a"}, {"or","o"}, {"xor","x"}, {"com","~"}, {"not","N"},
    {"1+","I"}, {"1-","D"}, {"I", "i"}, {"edit","e"},
    {"leave",";"}, {"timer","t"}, {"bye","zZ"}, {"reset","Y"},
    {"+locs","p"}, {"-locs","q"}, 
    {"r0","r0"}, {"r1","r1"}, {"r2","r2"},{"r3","r3"}, {"r4","r4"},
    {"r5","r5"}, {"r6","r6"}, {"r7","r7"},{"r8","r8"}, {"r9","r9"},
    {"s0","s0"}, {"s1","s1"}, {"s2","s2"},{"s3","s3"}, {"s4","s4"},
    {"s5","s5"}, {"s6","s6"}, {"s7","s7"},{"s8","s8"}, {"s9","s9"},
    // {"ext",'G'}, {"exy",'R'}, // Extensions
    {0,0}
};

char word[32];
byte lastWasCall = 0, isBye = 0;
CELL HERE, LAST, STATE, VHERE;

void CComma(CELL v) { user[HERE++] = (byte)v; }
void Comma(CELL v) { SET_LONG(&user[HERE], v); HERE += CELL_SZ; }
void WComma(WORD v) { SET_WORD(&user[HERE], v); HERE += 2; }

byte strEq(const char* x, const char* y) {
    while (*x && *y && (*x == *y)) { ++x; ++y; }
    return (*x || *y) ? 0 : 1;
}

char lower(char c) {
    return betw(c,'A','Z') ? (c + 32) : c;
}

byte strEqI(const char* x, const char* y) {
    while (*x && *y) {
        if (lower(*x) != lower(*y)) { return 0; }
        ++x; ++y;
    }
    return (*x || *y) ? 0 : 1;
}

void strCpy(char* d, const char* s) {
    while (*s) { *(d++) = *(s++); }
    *d = 0;
}

void printStringF(const char* fmt, ...) {
    char* buf = (char*)&user[USER_SZ-128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 100, fmt, args);
    va_end(args);
    printString(buf);
}

void doCreate(const char* name, byte f) {
    DICT_T *dp = DP_AT(HERE);
    dp->prev = (byte)(HERE - LAST);
    dp->flags = f;
    strCpy(dp->name, name);
    LAST = HERE;
    HERE += strLen(name) + 3;
}

int doWords() {
    CELL l = (WORD)LAST;
    while (l) {
        DICT_T *dp = DP_AT(l);
        printString(dp->name);
        printChar(' ');
        if (l == dp->prev) break;
        l -= dp->prev;
    }
    return 1;
}

int doFind(const char* name) {
    CELL l = (WORD)LAST;
    while (l) {
        DICT_T *dp = DP_AT(l);
        if (strEq(dp->name, name)) { return l; }
        if (l == dp->prev) break;
        l -= dp->prev;
    }
    return -1;
}

char* in;
int getWord(char* wd, char delim) {
    while (*in && (*in == delim)) { ++in; }
    int l = 0;
    while (*in && (*in != delim)) {
        *(wd++) = *(in++);
        ++l;
    }
    *wd = 0;
    return l;
}

int strLen(const char* str) {
    const char* cp = str;
    while (*cp) { ++cp; }
    return cp - str;
}

WORD getXT(WORD l, DICT_T *dp) {
    return l + strLen(dp->name) + 3;
}

int execWord(WORD l) {
    DICT_T* dp = DP_AT(l);
    WORD xt = getXT(l, dp);
    if ((STATE == 1) && (dp->flags == 0)) {
        CComma(':');
        WComma(xt);
        lastWasCall = 1;
    } else { run(xt); }
    return 1;
}

void compileNumber(CELL num) {
    if ((0 <= num) && (num <= 0xFF)) {
        CComma(1);
        CComma(num & 0xff);
    }
    else if ((0x0100 <= num) && (num <= 0xFFFF)) {
        CComma(2);
        SET_WORD(UA(HERE), (WORD)num);
        HERE += 2;
    }
    else {
        CComma(4);
        Comma(num);
    }
}

int isNum(const char* wd) {
    CELL x = 0;
    int base = BASE, isNeg = 0, lastCh = '9';
    if ((wd[0]=='\'') && (wd[2]==wd[0]) && (wd[3]==0)) { push(wd[1]); return 1; }
    if (*wd == '#') { base = 10;  ++wd; }
    if (*wd == '$') { base = 16;  ++wd; }
    if (*wd == '%') { base = 2;  ++wd; lastCh = '1'; }
    if ((*wd == '-') && (base == 10)) { isNeg = 1;  ++wd; }
    if (*wd == 0) { return 0; }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (betw(c, '0', lastCh)) { t = c - '0'; }
        if ((9 < base) && (betw(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((9 < base) && (betw(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x * base) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

int doPrim(const char *wd) {
    for (int i = 0; prims[i].op; i++) {
        PRIM_T* p = &prims[i];
        if (strEqI(p->name, wd)) {
            if (STATE) {
                for (int j = 0; p->op[j]; j++) { CComma(p->op[j]); }
            } else {
                byte* x = UA(HERE + 10);
                for (int j = 0; p->op[j]; j++) { *(x++) = p->op[j]; }
                *x = ';';
                run((WORD)HERE+10);
            }
            return 1;
        }
    }
    return 0;
}

int doParseWord(char* wd) {
    byte lwc = lastWasCall;
    lastWasCall = 0;

    if (doPrim(wd)) { return 1; }

    int l = doFind(wd);
    if (0 <= l) { return execWord(l); }

    if (isNum(wd)) {
        if (STATE == 1) { compileNumber(pop()); }
        return 1;
    }

    if (strEq(wd, ":")) {
        if (getWord(wd, ' ')) {
            doCreate(wd, 0);
            STATE = 1;
        }
        else { return 0; }
        return 1;
    }

    if (strEq(wd, "(")) {
        while (*in && (*in != ')')) { ++in; }
        if (*in == ')') { ++in; }
        return 1;
    }

    if (strEq(wd, ";")) {
        STATE = 0;
        if (lwc && (user[HERE - 3] == ':')) { user[HERE - 3] = 'J'; }
        else { CComma(';'); }
        return 1;
    }

    if (strEqI(wd, "VARIABLE")) {
        if (getWord(wd, ' ')) {
            doCreate(wd, 0);
            compileNumber((UCELL)VHERE);
            CComma(';');
            VHERE += CELL_SZ;
            return 1;
        }
        else { return 0; }
    }

    if (strEqI(wd, "IMMEDIATE")) { DP_AT(LAST)->flags |= 1; return 1; }
    if (strEqI(wd, "ALLOT")) { VHERE += pop();              return 1; }
    if (strEqI(wd, "WORDS")) { return doWords(); }

    if (strEqI(wd, "IF")) {
        CComma('j');
        push(HERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "ELSE")) {
        CELL tgt = pop();
        CComma('J');
        push(HERE);
        WComma(0);
        SET_WORD(UA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "THEN")) {
        CELL tgt = pop();
        SET_WORD(UA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "FOR")) {
        CComma('[');
        push(HERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "NEXT")) {
        CComma(']');
        CELL tgt = pop();
        SET_WORD(UA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "BEGIN")) {
        CComma('{');
        push(HERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "WHILE")) {
        CComma('}');
        CELL tgt = pop();
        SET_WORD(UA(tgt), (WORD)HERE);
        return 1;
    }

    STATE = 0;
    printStringF("[%s]??", wd);
    return 0;
}

void doParse(const char* line) {
    in = (char*)line;
    int len = getWord(word, ' ');
    while (0 < len) {
        if (strEq(word, "//")) { return; }
        if (strEq(word, "\\")) { return; }
        if (doParseWord(word) == 0) { return; }
        len = getWord(word, ' ');
    }
}

void doOK() {
    if (STATE) { printString(" ... "); return; }
    printString(" OK (");
    for (int d = 1; d <= sp; d++) {
        if (1 < d) { printChar(' '); }
        printBase(stk[d], BASE);
    }
    printString(")\r\n");
}

char *rtrim(char* str) {
    char* cp = str;
    while (*cp) { ++cp; }
    --cp;
    while ((str <= cp) && (*cp <= ' ')) { *(cp--) = 0; }
    return str;
}

void systemWords() {
    char* cp = (char*)(VHERE + 6);
    sprintf(cp, ": CELL %d ;", CELL_SZ);        doParse(cp);
    sprintf(cp, ": u %lu ;", (UCELL)user);      doParse(cp);
    sprintf(cp, ": h %lu ;", (UCELL)&HERE);     doParse(cp);
    sprintf(cp, ": v %lu ;", (UCELL)&VHERE);    doParse(cp);
    sprintf(cp, ": base %lu ;", (UCELL)&BASE);  doParse(cp);
    printString("\r\nmyForth v0.0.1");
    printStringF("\r\nCODE: %p, SIZE: %ld, HERE: %ld", user, USER_SZ, HERE);
    printStringF("\r\nVARS: %p, SIZE: %ld, VHERE: %p", vars, VARS_SZ, (void *)VHERE);
    printString("\r\nHello.");
}

#if __BOARD__ == PC
FILE* input_fp = NULL;

WORD doExt(CELL ir, WORD pc) {
    switch (ir) {
    case 'G': printf("-works-");           break;
    case 'z': isBye = U(pc++) == 'Z';      break;
    default: printStringF("-unk ir: (%c)(%d)-", ir, ir);
    }
    return pc;
}

void printString(const char* cp) { printf("%s", cp); }
void printChar(char c) { printf("%c", c); }
int charAvailable(void) { return _kbhit(); }
int getChar(void) { return _getch(); }

CELL timer() { return GetTickCount(); }
void delay() { return Sleep(pop()); }

void doHistory(const char* txt) {
    FILE* fp = fopen("history.txt", "at");
    if (fp) {
        fputs(txt, fp);
        fclose(fp);
    }
}

void loop() {
    char* tib = (char*)(VHERE + 6);
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { doOK(); }
    if (fgets(tib, 100, fp) == tib) {
        if (fp == stdin) { doHistory(tib); }
        doParse(rtrim(tib));
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = NULL;
    }
}

int main()
{
    vmReset();

    input_fp = fopen("sys.fs", "rt");

    while (!isBye) { loop(); }
}

#endif
