// MinForth.cpp : An extremely memory conscious Forth interpreter
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "Shared.h"

typedef struct {
    const char *name;
    char op;
} PRIM_T;

PRIM_T prims[] = {
    {"+",'+'}, {"-",'-'}, {"/",'/'}, {"*",'*'},{"/mod",'&'},
    {"swap",'$'}, {"drop",'\\'}, {"over",'%'}, {"dup",'#'},
    {"emit",','}, {".",'.'}, {"space",'b'}, {"cr",'n'},
    {"=",'='}, {"<",'<'}, {">",'>'}, {"0=",'N'},
    {"@",'@'}, {"c@",'c'}, {"w@",'w'}, {"!",'!'}, {"c!",'C'}, {"w!",'W'},
    {"and",'A'}, {"or",'O'}, {"xor",'X'}, {"com",'~'},
    {"1+",'I'}, {"1-",'D'}, {"I", 'i'},
    {"leave",';'}, {"timer",'t'}, {"bye",'Z'},
    {0,0}
};

#define betw(x, a, b) ((a<=x)&&(x<=b))

char word[32];
FILE* input_fp = NULL;
byte lastWasCall = 0;
WORD HERE, LAST;
CELL STATE, VHERE;

void printString(const char* cp) { printf("%s", cp); }
void printChar(char c) { printf("%c", c); }

void CComma(CELL v) { user[HERE++] = (byte)v; }
void Comma(CELL v) { SET_LONG(&user[HERE], v); HERE += CELL_SZ; }
void WComma(WORD v) { SET_WORD(&user[HERE], v); HERE += 2; }

byte strEq(const char* x, const char* y) {
    while (*x && *y && (*x == *y)) { ++x; ++y; }
    return (*x || *y) ? 0 : 1;
}

char lower(char c) {
    return (('A' <= c) && (c <= 'Z')) ? (c + 32) : c;
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

UCELL align4(UCELL x) {
    while (x % 4) { --x; }
    return x;
}

void doCreate(const char* name, byte f) {
    WORD prev = LAST;
    DICT_T *dp = DP_AT(HERE);
    dp->prev = (byte)(HERE - LAST);
    dp->flags = f;
    strCpy(dp->name, name);
    LAST = HERE;
    HERE += strLen(name) + 3;
}

int doFind(const char* name) {
    WORD l = (WORD)LAST;
    while (l) {
        DICT_T *dp = DP_AT(l);
        if (strEq(dp->name, name)) { return l; }
        if (l == dp->prev) break;
        l -= dp->prev;
    }
    return -1;
}

int isDigit(char c) {
    if (betw(c,'0','9')) { return c - '0'; }
    return -1;
}

void reset() {
    sp = 0;
    rsp = 0;
    BASE = 10;
    HERE = 0;
    LAST = 0;
    VHERE = (CELL)&vars[0];
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

void execWord(WORD l) {
    DICT_T* dp = DP_AT(l);
    WORD xt = getXT(l, dp);
    if ((STATE == 1) && (dp->flags == 0)) {
        CComma(':');
        WComma(xt);
        lastWasCall = 1;
    } else {
       run(xt);
    }
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
        if (strEqI(prims[i].name, wd)) {
            if (STATE) { CComma(prims[i].op); }
            else {
                user[HERE+1] = prims[i].op;
                user[HERE+2] = ';';
                run(HERE+1);
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
    if (0 <= l) {
        execWord((WORD)l);
        return 1;
    }

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

    if (strEq(wd, ";")) {
        STATE = 0;
        if (lwc && (user[HERE - 3] == ':')) { 
            user[HERE - 3] = 'J';
        } else {
            CComma(';');
        }
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
        SET_WORD(UA(tgt), HERE);
        return 1;
    }

    if (strEqI(wd, "THEN")) {
        CELL tgt = pop();
        SET_WORD(UA(tgt), HERE);
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
        SET_WORD(UA(tgt), HERE);
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
        SET_WORD(UA(tgt), HERE);
        return 1;
    }

    STATE = 0;
    printf("[%s]??", wd);
    return 0;
}

void doParse(const char* line) {
    in = (char*)line;
    int len = getWord(word, ' ');
    while (0 < len) {
        if (strEq(word, "//")) { return; }
        if (strEq(word, "\\")) { return; }
        if (doParseWord(word) == 0) {
            return;
        }
        len = getWord(word, ' ');
    }
}

void doOK() {
    printString(" OK (");
    for (int d = 1; d <= sp; d++) {
        if (1 < d) { printChar(' '); }
        printBase(stk[d], BASE);
    }
    printf(")\r\n");
}

void doSystemWords() {
    char* cp = (char *)(VHERE + 6);
    sprintf(cp, ": CELL %d ;", CELL_SZ);        doParse(cp);
    sprintf(cp, ": u %lu ;", (UCELL)user);      doParse(cp);
    sprintf(cp, ": h %lu ;", (UCELL)&HERE);     doParse(cp);
    sprintf(cp, ": v %lu ;", (UCELL)VHERE);     doParse(cp);
    sprintf(cp, ": base %lu ;", (UCELL)&BASE);  doParse(cp);
    // sprintf(cp, "h . cr");  doParse(cp);
}

void doHistory(const char* txt) {
    FILE* fp = fopen("history.txt", "at");
    if (fp) {
        fputs(txt, fp);
        fclose(fp);
    }
}

char *rtrim(char* str) {
    char* cp = str;
    while (*cp) { ++cp; }
    --cp;
    while ((str <= cp) && (*cp <= ' ')) { *(cp--) = 0; }
    return str;
}

void loop() {
    char* tib = (char *)(VHERE + 6);
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
    reset();
    doSystemWords();

    printf("\r\nMinForth v0.0.1");
    printf("\r\nCODE: %p, SIZE: %d, HERE: %ld", user, USER_SZ, HERE);
    printf("\r\nVARS: %p, SIZE: %ld, VHERE: %ld", vars, VARS_SZ, (UCELL)VHERE);

    printf("\r\nHello.");
    input_fp = fopen("sys.fs", "rt");

    while (rsp != 99) { loop(); }
}
