// MinForth.cpp : An extremely memory conscious Forth interpreter
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "Shared.h"

char word[32];

byte strEq(const char* x, const char* y) {
    while (*x && *y && (*x == *y)) {
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
    int len = strLen(name);
    CELL n = LAST - (CELL_SZ + 1 + len + 1);
    if (n < 0) {
        printf("-DICT-OVERFLOW-"); return; 
    }
    dict_t* dp = DP_AT(n);
    dp = (dict_t *)align4((UCELL)dp);
    dp->xt = HERE;
    dp->flags = f;
    strCpy(dp->name, name);
    LAST = (UCELL)dp - (UCELL)DICT;
}

dict_t* doFind(const char* name) {
    CELL l = LAST;
    CELL end = DICT_SZ - 2;
    while (l && (l < end)) {
        dict_t *dp = DP_AT(l);
        if (strEq(dp->name, name)) { return dp; }
        l = getNext(l);
    }
    return NULL;
}

int isDigit(char c) {
    if (('0' <= c) && (c <= '9')) { return c - '0'; }
    return -1;
}

void reset() {
    DSP = 0;
    RSP = 0;
    BASE = 10;
    HERE = CELL_SZ;
    LAST = DICT_SZ;
    VHERE = 0;
}


char* in;
int getWord(char* wd, char delim) {
    while (*in && (*in == delim)) {
        ++in;
    }
    int l = 0;
    while (*in && (*in != delim)) {
        *(wd++) = *(in++);
        ++l;
    }
    *wd = 0;
    return l;
}

void execWord(dict_t* dp) {
    if (STATE == 1) {
        if (dp->flags == 2) { CComma((CELL)dp->xt); }
        else {
            CComma(':');
            Comma((CELL)dp->xt);
        }
    }
    else {
        if (dp->flags == 2) {
            CODE[HERE] = (byte)(dp->xt);
            CODE[HERE+1] = ';';
            run(HERE);
        }
        else {
            run(dp->xt);
        }
    }
}

int isNum(char* x) {
    // TODO: support neg
    // TODO: support other bases
    while (*x) {
        if (isDigit(*(x++)) == -1) { return 0; }
    }
    return 1;
}

void compileNumber(CELL num) {
    if ((0 <= num) && (num <= 0xFF)) {
        CComma(1);
        CComma(num & 0xff);
    }
    else if ((0x0100 <= num) && (num <= 0xFFFF)) {
        CComma(2);
        SET_WORD(DICT+HERE, (WORD)num);
        HERE += 2;
    }
    else {
        CComma(4);
        Comma(num);
    }
}

int doParseNum(char* wd) {
    // TODO: support neg
    if (!isNum(wd)) { return 0; }
    push(0);
    while (*wd) {
        T = (T * BASE) + isDigit(*wd);
        wd++;
    }
    return 1;
}

int doParseWord(char* wd) {
    dict_t* dp = doFind(wd);
    if (dp) {
        execWord(dp);
        return 1;
    }

    if (doParseNum(wd)) {
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
        CComma(';');
        return 1;
    }

    if (strEq(wd, "VARIABLE")) {
        if (getWord(wd, ' ')) {
            doCreate(wd, 0);
            compileNumber((UCELL)VHERE);
            CComma(';');
            VHERE += CELL_SZ;
            return 1;
        }
        else { return 0; }
    }

    return 0;
}

void doParse(const char* line) {
    in = (char*)line;
    int len = getWord(word, ' ');
    while (0 < len) {
        if (doParseWord(word) == 0) {
            return;
        }
        len = getWord(word, ' ');
    }
}

void doBuiltin(const char* name, byte op) {
    doCreate(name, 2);
    dict_t* dp = DP_AT(LAST);
    dp->xt = op;
}

void doOK() {
    int d = DSP;
    printf(" OK (");
    for (int d = 1; d <= DSP; d++) { printf("%s%d", (1 < d) ? " " : "", DSTK[d]); }
    printf(")\r\n");
}

void loop() {
    char* tib = STR_AT(VHERE+6);
    // FILE* fp = (input_fp) ? input_fp : stdin;
    FILE* fp = stdin;
    if (fp == stdin) { doOK(); }
    if (fgets(tib, 100, fp) == tib) {
        // if (fp == stdin) { doHistory(tib); }
        int l = strLen(tib) - 1;
        while ((0 < l) && (tib[l]) && (tib[l] < ' ')) { --l; }
        tib[l + 1] = 0;
        doParse(tib);
        return;
    }
    //if (input_fp) {
    //    fclose(input_fp);
    //    input_fp = NULL;
    //}
}

int main()
{
    char* cp;
    reset();
    printf("\r\nMinForth v0.0.1");
    printf("\r\nCODE: %lu, SIZE: %ld, HERE: %ld", (UCELL)CODE, (UCELL)CODE_SZ, HERE);
    printf("\r\nVARS: %p, SIZE: %ld, VHERE: %ld", VARS, (long)CODE_SZ, VHERE);
    printf("\r\nDICT: %p, SIZE: %ld, LAST: %ld", DICT, (long)DICT_SZ, LAST);
    printf("\r\nHello.");
    cp = STR_AT(VHERE + 6); sprintf(cp, ": CELL %d ; : ADDR %d ;", CELL_SZ, ADDR_SZ); doParse(cp);
    cp = STR_AT(VHERE + 6); sprintf(cp, ": (here) %lu ; : (last) %lu ;", (UCELL)&HERE, (UCELL)&LAST); doParse(cp);
    cp = STR_AT(VHERE + 6); sprintf(cp, ": (code) %lu ; : (vars) %lu ;", (UCELL)CODE, (UCELL)VARS); doParse(cp);
    doBuiltin("SWAP", '$');
    doBuiltin("DROP", '\\');
    doBuiltin("DUP", '#');
    doBuiltin("OVER", '%');
    doBuiltin("EMIT", ',');
    doBuiltin("WORDS", 'W');
    doBuiltin("CR", 'n');
    doBuiltin("(.)", '.');
    doBuiltin("+", '+');
    doBuiltin("-", '-');
    doBuiltin("*", '*');
    doBuiltin("/", '/');
    doBuiltin("C@", 'c');
    doBuiltin("C!", 'C');
    doBuiltin("A@", 'a');
    doBuiltin("A!", 'A');
    doBuiltin("@", '@');
    doBuiltin("!", '!');
    doBuiltin("BYE", 'Z');
    doBuiltin("ZZ", 'Z');
    doParse(": . 32 EMIT (.) ;");
    while (RSP != 99) {
        loop();
    }
}
