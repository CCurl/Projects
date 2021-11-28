#include "newF.h"

static char src[96];
addr HERE;
DICT_T *LAST;
addr DICT_END;
CELL STATE;
CELL BASE;


void p(const char *x) { printString(x); }

bool strEquals(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) { return 0; }
        ++str1;
        ++str2;
    }
    return (*str1 == *str2) ? 1 : 0;
}

int strlen(const char *x) {
    int l = 0;
    while (*x) { ++x; ++l; }
    return l;
}

void strcpy(char *t, const char *f) {
    while (*f) { *(t++) = *(f++); }
    *t = 0;
}

void cComma(byte x) {
    // p("c,");
    *(HERE++) = x;
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
    dp->len = strlen(word);
    strcpy(dp->name, word);
    LAST = dp;
    // printStringF("\n%s created at %p, XT=%ld", word, dp, (long)dp->XT);
}

DICT_T *find(char *word) {
    DICT_T *dp = (DICT_T *)LAST;
    while ((addr)dp < DICT_END) {
        if (strEquals(dp->name, word)) { return dp;  }
        ++dp;
    }
    return NULL;
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
    while (1) {
        line = getWord(line, wd);
        if (wd[0] == 0) {
            return;
        }
        // p("-"); p(wd); p("-");
        DICT_T *dp = find(wd);
        if (dp) {
            // printStringF("found\n");
            if ((STATE == 0) || (dp->flags == 2)) {
                // printStringF("... call XT=%ld\n", (long)dp->XT);
                run(dp->XT);
                // printStringF("back\n");
            } else {
                if (dp->flags == 1) {
                    byte *x = dp->XT;
                    while (*x != ';') { cComma(*x); x++; }
                } else {
                    cComma(5);
                    comma((CELL)dp->XT);
                }
            }
            continue;
        }
        if (isNum(wd)) {
            if (STATE) {
                if ((T & 0xFF) == T) {
                    cComma(1);
                    cComma(pop());
                } else {
                    cComma(4);
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
            cComma(';');
            STATE = 0;
            continue;
        }
        if (strEquals(wd, "dd")) {
            for (addr i = USER; i < HERE; i++) {
                printStringF(" %d", *i);
            }
            STATE = 0;
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
    DICT_END = (HERE + USER_SZ);
    LAST = (DICT_T *)(DICT_END);
    STATE = 0;
    BASE = 10;
    sprintf(src, ": (here) %ld ;",  (CELL)&HERE);      parse(src);
    sprintf(src, ": (last) %ld ;",  (CELL)&LAST);      parse(src);
    sprintf(src, ": base %ld ;",    (CELL)&BASE);      parse(src);
    sprintf(src, ": state %ld ;",   (CELL)&STATE);     parse(src);
    sprintf(src, ": user %ld ;",    (CELL)HERE);       parse(src);
    sprintf(src, ": user-sz %ld ;", (CELL)USER_SZ);    parse(src);
    sprintf(src, ": cell %ld ;",    (CELL)CELL_SZ);    parse(src);
    sprintf(src, ": addr %ld ;",    (CELL)sizeof(addr));    parse(src);
    prim("swap", "$");
    prim("over", "%");
    prim("@", "@");
    prim("c@", "`@");
    prim("!", "!");
    prim("c!", "`!");
    prim("+", "+");
}

