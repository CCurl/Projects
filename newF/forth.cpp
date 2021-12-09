#include "newF.h"

addr HERE, VHERE, VHERE_T, USER_END;
DICT_T *LAST;
CELL BASE, STATE;

#define INLINE    0x01
#define IMMEDIATE 0x02
#define PUSH_C    1
#define PUSH_W    2
#define PUSH_L    4
#define CALL      5
#define JUMP      6
#define BRANCH0   7

void p(const char *x) { printString(x); }

bool strEquals(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) { return 0; }
        ++str1;
        ++str2;
    }
    return (*str1 == *str2) ? 1 : 0;
}

int strLen(const char *x) {
    int l = 0;
    while (*x) { ++x; ++l; }
    return l;
}

void strCpy(char *t, const char *f) {
    while (*f) { *(t++) = *(f++); }
    *t = 0;
}

void vCComma(byte x) {
    *(VHERE++) = x;
}

void vComma(CELL x) {
    setCell(VHERE, x);
    VHERE += CELL_SZ;
}

void cComma(byte x) {
    *(HERE++) = x;
}

void wComma(CELL x) {
    setWord(HERE, x);
    HERE += 2;
}

void comma(CELL x) {
    setCell(HERE, x);
    HERE += CELL_SZ;
}

CELL getSeed() {
    return (CELL)GetTickCount();
}

void create(const char *word) {
    DICT_T *dp = LAST-1;
    dp->XT = HERE;
    dp->flags = 0;
    dp->len = strLen(word);
    strCpy(dp->name, word);
    LAST = dp;
    // printStringF("\n%s created at %p, XT=%ld", word, dp, (long)dp->XT);
}

DICT_T *find(char *word) {
    DICT_T *dp = (DICT_T *)LAST;
    while ((addr)dp < USER_END) {
        if (strEquals(dp->name, word)) { return dp;  }
        ++dp;
    }
    return NULL;
}

void wordsl() {
    DICT_T* dp = (DICT_T*)LAST;
    addr to = HERE;
    while ((addr)dp < USER_END) {
        printStringF("\n%08ld %-15s ", (long)dp->XT, dp->name);
        for (addr i = dp->XT; i < to; i++) {
            char c = *i;
            if (BetweenI(c, 33, 126)) {
                printStringF("%c", c);
            } else {
                printStringF("(%d)", c);
                if (c == PUSH_C) { printStringF("(push %ld)", *(i + 1)); i += 1; }
                else if (c == PUSH_W) { printStringF("(push %ld)", getWord(i + 1)); i += 2; }
                else if (c == PUSH_L) { printStringF("(push %ld)", getCell(i + 1)); i += CELL_SZ; }
                else if (c == CALL) { printStringF("(call %ld)", getCell(i + 1)); i += CELL_SZ; }
                else if (c == JUMP) { printStringF("(jump %ld)", getCell(i + 1)); break; }
                else if (c == BRANCH0) { printStringF("(0branch %ld)", *(i + 1)); i += 1;  }
            }
        }
        if (dp->flags & INLINE) { printStringF("    (INLINE)"); }
        if (dp->flags & IMMEDIATE) { printStringF("    (IMMEDIATE)"); }
        to = (dp++)->XT;
    }
}

#define isBetween(x, a, b) ((a <= x) && (x <= b))

int isDigit(char c, int base) {
    if ((base == 2) && isBetween(c, '0', '1')) { return c - '0'; }
    if ((10 <= base) && isBetween(c, '0', '9')) { return c - '0'; }
    if ((base == 16) && isBetween(c, 'a', 'f')) { return c - 'a' + 10; }
    if ((base == 16) && isBetween(c, 'A', 'F')) { return c - 'A' + 10; }
    return -1;
}

bool isNum(char *word) {
    int base = BASE, isNeg = 0;
    push(0);
    if ((word[0] == '\'') && (word[2] == '\'') && (word[3] == 0)) {
        T = word[1];
        return 1;
    }

    if (word[0] == '$') { base = 16; ++word; }
    if (word[0] == '#') { base = 10; ++word; }
    if (word[0] == '%') { base =  2; ++word; }
    if ((word[0] == '-') && (base == 10)) { isNeg = 1; ++word; }

    while (*word) {
        if (*word == ' ') { return 1; }
        int d = isDigit(*word, base);
        if (d < 0) {
            pop();
            return 0;
        }
        T = (T*base)+d;
        word++;
    }
    if (isNeg) { T = -T; }
    return 1;
}

char *getWord(char *line, char *word) {
    char *x = word;
    while (*line == ' ') { ++line; }
    while ((*line) && (*line != ' ')) {
        *(word++) = *(line++);
    }
    *word = 0;
    return line;
}

void parse(char *line) {
    char wd[32];
    BOOL isError = FALSE;
    BOOL lastWasCall = FALSE;
    while ((TRUE) && (!isError)) {
        if (VHERE_T < VHERE) { VHERE_T = VHERE; }
        line = getWord(line, wd);
        if (wd[0] == 0) { return; }
        if (strEquals(wd, "//")) { return; }
        if (strEquals(wd, "\\")) { return; }
        BOOL lwc = lastWasCall;
        lastWasCall = FALSE;
        DICT_T* dp = find(wd);
        if (dp) {
            if ((STATE == 0) || (dp->flags & IMMEDIATE)) {
                run(dp->XT);
            } else {
                if (dp->flags & INLINE) {
                    byte *x = dp->XT;
                    while (*x != ';') { cComma(*x); x++; }
                } else {
                    cComma(CALL);
                    comma((CELL)dp->XT);
                    lastWasCall = 1;
                }
            }
            continue;
        }
        if (isNum(wd)) {
            if (STATE) {
                if ((T & 0xFF) == T) {
                    cComma(PUSH_C);
                    cComma((byte)pop());
                } else if ((T & 0xFFFF) == T) {
                    cComma(PUSH_W);
                    wComma(pop());
                } else {
                    cComma(PUSH_L);
                    comma(pop());
                }
            }
            continue;
        }
        if (strEquals(wd, ":") && (STATE == 0)) {
            line = getWord(line, wd);
            if (wd[0]) {
                create(wd);
                STATE = 1;
                VHERE_T = VHERE;
            }
            continue;
        }
        if (strEquals(wd, ";") && (STATE == 1)) {
            if (lwc) { *(HERE-CELL_SZ-1) = JUMP; } 
            else { cComma(';'); }
            STATE = 0;
            continue;
        }
        if (strEquals(wd, "wordsl") && (STATE == 0)) {
            wordsl();
            continue;
        }
        if (strEquals(wd, "constant") && (STATE == 0)) {
            line = getWord(line, wd);
            if (wd[0]) {
                create(wd);
                cComma(PUSH_L);
                comma(pop());
                cComma(';');
            }
            continue;
        }
        if (strEquals(wd, "variable") && (STATE == 0)) {
            line = getWord(line, wd);
            if (wd[0]) {
                create(wd);
                cComma(PUSH_L);
                comma((CELL)VHERE);
                cComma(';');
                setCell(VHERE, 0);
                VHERE += CELL_SZ;
            }
            continue;
        }
        if (strEquals(wd, "value") && (STATE == 0)) {
            line = getWord(line, wd);
            if (wd[0]) {
                create(wd);
                cComma(PUSH_L);
                addr x = HERE;
                comma(0);
                cComma(';');

                char wd2[16];
                sprintf(wd2, "(%s)", wd);
                create(wd2);
                cComma(PUSH_L);
                comma((CELL)x);
                cComma(';');
            }
            continue;
        }
        if (strEquals(wd, ".\"")) {
            line++;
            if (STATE) {
                cComma('"');
                while ((*line) && (*line != '"')) { cComma(*(line++)); }
                cComma('"');
            }
            else {
                while ((*line) && (*line != '"')) { printChar(*(line++)); }
            }
            ++line;
            continue;
        }
        if (strEquals(wd, "\"")) {
            addr lenAddr = VHERE_T;
            push((CELL)lenAddr);
            *(VHERE_T++) = 0;
            line++;
            while ((*line) && (*line != '"')) { *(VHERE_T++) = *(line++); ++(*lenAddr); }
            *(VHERE_T++) = 0;
            ++line;
            if (STATE) {
                cComma(PUSH_L);
                comma((CELL)pop());
                VHERE = VHERE_T;
            }
            continue;
        }
        printStringF("'%s'??", wd);
        isError = 1;
    }
}

void prim(const char *name, const char *code) {
    create(name);
    LAST->flags = 1;
    while (*code) { cComma(*(code++)); }
    cComma(';');
}

void forthInit() {
    static char src[32];
    HERE = USER;
    VHERE = VAR;
    USER_END = (USER+USER_SZ-1);
    LAST = (DICT_T *)(USER_END+1);
    STATE = 0;
    BASE = 10;
    sprintf(src, ": cell %ld ;",      (CELL)CELL_SZ);         parse(src);    LAST->flags = INLINE;
    sprintf(src, ": addr %ld ;",      (CELL)sizeof(addr));    parse(src);    LAST->flags = INLINE;
    sprintf(src, ": dentry-sz %ld ;", (CELL)sizeof(DICT_T));  parse(src);    LAST->flags = INLINE;
    sprintf(src, ": user-sz %ld ;",   (CELL)USER_SZ);         parse(src);    LAST->flags = INLINE;
    sprintf(src, ": (here) %ld ;",    (CELL)&HERE);           parse(src);    LAST->flags = INLINE;
    sprintf(src, ": (vhere) %ld ;",   (CELL)&VHERE);          parse(src);    LAST->flags = INLINE;
    sprintf(src, ": (last) %ld ;",    (CELL)&LAST);           parse(src);    LAST->flags = INLINE;
    sprintf(src, ": base %ld ;",      (CELL)&BASE);           parse(src);    LAST->flags = INLINE;
    sprintf(src, ": state %ld ;",     (CELL)&STATE);          parse(src);    LAST->flags = INLINE;
    sprintf(src, ": user %ld ;",      (CELL)USER);            parse(src);    LAST->flags = INLINE;
    sprintf(src, ": vars %ld ;",      (CELL)VAR);             parse(src);    LAST->flags = INLINE;
    sprintf(src, ": user-end %ld ;",  (CELL)USER_END);        parse(src);    LAST->flags = INLINE;
    prim("swap", "$");
    prim("over", "%");
    prim("@", "@");
    prim("c@", "c");
    prim("!", "!");
    prim("c!", "C");
    prim("+", "+");
}
