#include "newF.h"

static char src[96];
addr HERE;
DICT_T *LAST;
addr USER_END;
CELL STATE;
CELL BASE;

#define INLINE    0x01
#define IMMEDIATE 0x02
#define PUSH_C    '1'
#define PUSH_W    '2'
#define PUSH_L    '4'
#define CALL      '5'
#define JUMP      '6'

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

void cComma(byte x) {
    // p("c,");
    *(HERE++) = x;
}

void wComma(CELL x) {
    // p(",");
    setWord(HERE, x);
    HERE += 2;
}

void comma(CELL x) {
    // p(",");
    setCell(HERE, x);
    HERE += CELL_SZ;
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
    DICT_T *dp = (DICT_T *)LAST;
    while ((addr)dp < USER_END) {
        printString("");
        printStringF("\n%08ld:", (long)dp->XT);
        for (addr i = dp->XT; *i != ';'; i++) {
            char c = *i;
            if (BetweenI(c, 33, 126)) {
                if (c == PUSH_W) { printStringF(" 2 (%ld)", getWord(i+1)); i += 2; }
                else if (c == PUSH_L) { printStringF(" 4 (%ld)",    getCell(i+1)); i += CELL_SZ; }
                else if (c == CALL)   { printStringF(" CALL (%ld)", getCell(i+1)); i += CELL_SZ; }
                else if (c == JUMP)   { printStringF(" JUMP (%ld)", getCell(i+1)); break; }
                else printStringF(" %c", c);
            } else {
                printStringF(" (%d)", c);
            }
        }
        printStringF(" ;    %s", dp->name);
        if (dp->flags & INLINE) { printStringF("    (INLINE)"); }
        if (dp->flags & IMMEDIATE) { printStringF("    (IMMEDIATE)"); }
        ++dp;
    }
}

bool isNum(char *word) {
    push(0);
    // p("."); p(word); p(".");
    if ( (word[0] == '\'') && (word[2] == '\'') && (word[3] == 0) ) {
        // p("YYY");
        T = word[1];
        return 1;
    }
    while (*word) {
        if (*word == ' ') { return 1; }
        if ((*word < '0') || ('9' < *word)) {
            pop();
            return 0;
        }
        T = (T*BASE)+(*word-'0');
        word++;
    }
    return 1;
}

char *getWord(char *line, char *word) {
    char *x = word;
    while (*line == ' ') { ++line; }
    while ((*line) && (*line != ' ')) {
        *(word++) = *(line++);
    }
    *word = 0;
    // p("-"); p(x); p("-");
    return line;
}

void parse(char *line) {
    // p("\n"); p(line);
    char wd[32];
    int lastWasCall = 0;
    while (1) {
        line = getWord(line, wd);
        if (wd[0] == 0) {
            return;
        }
        // p("-"); p(wd); p("-");
        DICT_T *dp = find(wd);
        int lwc = lastWasCall;
        lastWasCall = 0;
        if (dp) {
            // printStringF("found\n");
            if ((STATE == 0) || (dp->flags & IMMEDIATE)) {
                // printStringF("... call XT=%ld\n", (long)dp->XT);
                run(dp->XT);
                // printStringF("back\n");
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
        if (strEquals(wd, ":")) {
            line = getWord(line, wd);
            if (wd[0]) {
                create(wd);
                STATE = 1;
            }
            continue;
        }
        if (strEquals(wd, ";")) {
            if (lwc) {
                *(HERE-CELL_SZ-1) = JUMP;
            } else {
                cComma(';');
            }
            STATE = 0;
            continue;
        }
        if (strEquals(wd, "wordsl")) {
            wordsl();
            continue;
        }
        printStringF("[%s]??", wd);
    }
}

void prim(const char *name, const char *code) {
    create(name);
    LAST->flags = 1;
    while (*code) {
        cComma(*(code++));
    }
    cComma(';');
}

void forthInit() {
    HERE = USER;
    USER_END = (USER+USER_SZ-1);
    LAST = (DICT_T *)(USER_END+1);
    STATE = 0;
    BASE = 10;
    sprintf(src, ": cell %ld ;",      (CELL)CELL_SZ);         parse(src); LAST->flags = INLINE;
    sprintf(src, ": addr %ld ;",      (CELL)sizeof(addr));    parse(src); LAST->flags = INLINE;
    sprintf(src, ": dentry-sz %ld ;", (CELL)sizeof(DICT_T));  parse(src); LAST->flags = INLINE;
    sprintf(src, ": user-sz %ld ;",   (CELL)USER_SZ);         parse(src); LAST->flags = INLINE;
    sprintf(src, ": (here) %ld ;",    (CELL)&HERE);           parse(src); LAST->flags = INLINE;
    sprintf(src, ": (last) %ld ;",    (CELL)&LAST);           parse(src); LAST->flags = INLINE;
    sprintf(src, ": base %ld ;",      (CELL)&BASE);           parse(src); LAST->flags = INLINE;
    sprintf(src, ": state %ld ;",     (CELL)&STATE);          parse(src); LAST->flags = INLINE;
    sprintf(src, ": user %ld ;",      (CELL)USER);            parse(src); LAST->flags = INLINE;
    sprintf(src, ": user-end %ld ;",  (CELL)USER_END);        parse(src); LAST->flags = INLINE;
    prim("swap", "$");
    prim("over", "%");
    prim("@", "@");
    prim("c@", "`@");
    prim("!", "!");
    prim("c!", "`!");
    prim("+", "+");
}
