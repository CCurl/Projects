#include "newF.h"

addr HERE, VHERE, VHERE_T, USER_END;
char pad[96], *toIn;
DICT_T *LAST;
CELL STATE;

#define IMMEDIATE 1
#define INLINE    2

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

void cComma(byte x) {
    *(HERE++) = x;
}

void numComma(CELL x) {
    char b[16], *cp = b;
    sprintf(b, "%ld", x);
    while (*cp) { *(HERE++) = *(cp++); }
}

void comma(CELL x) {
    setCell(HERE, x);
    HERE += CELL_SZ;
}

CELL getSeed() {
    return (CELL)GetTickCount();
}

void create(const char *pad) {
    DICT_T *dp = LAST-1;
    dp->XT = HERE;
    dp->flags = 0;
    dp->len = strLen(pad);
    strCpy(dp->name, pad);
    LAST = dp;
    // printStringF("\n%s created at %p, XT=%ld", word, dp, (long)dp->XT);
}

DICT_T *find(char *pad) {
    DICT_T *dp = (DICT_T *)LAST;
    while ((addr)dp < USER_END) {
        if (strEquals(dp->name, pad)) { return dp;  }
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
            }
        }
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

bool isNum(char *pad) {
    int base = BASE, isNeg = 0;
    push(0);
    if ((pad[0] == '\'') && (pad[2] == '\'') && (pad[3] == 0)) {
        T = pad[1];
        return 1;
    }

    if (pad[0] == '$') { base = 16; ++pad; }
    if (pad[0] == '#') { base = 10; ++pad; }
    if (pad[0] == '%') { base =  2; ++pad; }
    if ((pad[0] == '-') && (base == 10)) { isNeg = 1; ++pad; }

    while (*pad) {
        if (*pad == ' ') { return 1; }
        int d = isDigit(*pad, base);
        if (d < 0) {
            pop();
            return 0;
        }
        T = (T*base)+d;
        pad++;
    }
    if (isNeg) { T = -T; }
    return 1;
}

char *getWord(char *line, char *wd) {
    while ((*line == 9) || (*line == ' ')) { ++line; }
    while (' ' < *line) {
        *(wd++) = *(line++);
    }
    *wd = 0;
    return line;
}

void parse(char *line) {
    toIn = line;
    BOOL isError = FALSE;
    BOOL lastWasCall = FALSE;
    while ((TRUE) && (!isError)) {
        if (VHERE_T < VHERE) { VHERE_T = VHERE; }
        toIn = getWord(toIn, pad);
        if (pad[0] == 0) {
            return;
        }
        if (strEquals(pad, "//")) { return; }
        if (strEquals(pad, "\\")) { return; }
        BOOL lwc = lastWasCall;
        lastWasCall = FALSE;
        DICT_T* dp = find(pad);
        if (dp) {
            if ((STATE == 0) || (dp->flags & IMMEDIATE)) {
                run(dp->XT);
            } else {
                if (dp->flags & INLINE) {
                    byte *x = dp->XT;
                    while (*x != ';') { cComma(*x); x++; }
                } else {
                    numComma((CELL)dp->XT);
                    cComma('X');
                    lastWasCall = 1;
                }
            }
            continue;
        }
        if (isNum(pad)) {
            if (STATE) {
                numComma(pop());
            }
            continue;
        }
        if (strEquals(pad, ":") && (STATE == 0)) {
            toIn = getWord(toIn, pad);
            if (pad[0]) {
                create(pad);
                STATE = 1;
                VHERE_T = VHERE;
            }
            continue;
        }
        if (strEquals(pad, ";") && (STATE == 1)) {
            cComma(';');
            STATE = 0;
            continue;
        }
        printStringF("'%s'??", pad);
        STATE = 0;
        isError = 1;
    }
}

void prim(const char *name, const char *code) {
    create(name);
    LAST->flags = INLINE;
    while (*code) { cComma(*(code++)); }
    cComma(';');
}

void forthInit() {
    static char src[32];
    HERE = &user[0];
    VHERE = &var[0];
    USER_END = (USER+USER_SZ-1);
    LAST = (DICT_T *)(USER_END+1);
    STATE = 0;
    BASE = 10;
    sprintf(src, ": cell %ld ;",      (CELL)CELL_SZ);         parse(src);    LAST->flags = INLINE;
    sprintf(src, ": addr %ld ;",      (CELL)ADDR_SZ);         parse(src);    LAST->flags = INLINE;
    sprintf(src, ": dentry-sz %ld ;", (CELL)sizeof(DICT_T));  parse(src);    LAST->flags = INLINE;
    sprintf(src, ": user %ld ;",      (CELL)USER);            parse(src);    LAST->flags = INLINE;
    sprintf(src, ": user-sz %ld ;",   (CELL)USER_SZ);         parse(src);    LAST->flags = INLINE;
    sprintf(src, ": vars %ld ;",      (CELL)VAR);             parse(src);    LAST->flags = INLINE;
    sprintf(src, ": vars-sz %ld ;",   (CELL)VARS_SZ);         parse(src);    LAST->flags = INLINE;
    sprintf(src, ": (here) %ld ;",    (CELL)&HERE);           parse(src);    LAST->flags = INLINE;
    sprintf(src, ": (vhere) %ld ;",   (CELL)&VHERE);          parse(src);    LAST->flags = INLINE;
    sprintf(src, ": (last) %ld ;",    (CELL)&LAST);           parse(src);    LAST->flags = INLINE;
    sprintf(src, ": base %ld ;",      (CELL)&BASE);           parse(src);    LAST->flags = INLINE;
    sprintf(src, ": state %ld ;",     (CELL)&STATE);          parse(src);    LAST->flags = INLINE;
    sprintf(src, ": user-end %ld ;",  (CELL)USER_END);        parse(src);    LAST->flags = INLINE;
    sprintf(src, ": >in %ld ;",       (CELL)&toIn);           parse(src);    LAST->flags = INLINE;
    sprintf(src, ": pad %ld ;",       (CELL)pad);             parse(src);    LAST->flags = INLINE;
    prim("swap", "$");
    prim("over", "%");
    prim("@", "@");
    prim("c@", "c");
    prim("!", "!");
    prim("c!", "C");
    prim("+", "+");
}
