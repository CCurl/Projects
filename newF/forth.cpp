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
    sprintf(b, "h%lx", x);
    while (*cp) { *(HERE++) = *(cp++); }
}

void comma(CELL x) {
    setCell(HERE, x);
    HERE += CELL_SZ;
}

CELL getSeed() {
    return (CELL)GetTickCount();
}

ushort lastFunc = 0;
void create(char *pad) {
    if (11 < strLen(pad)) { pad[13] = 0; }
    DICT_T *dp = LAST-1;
    dp->func = lastFunc;
    func[lastFunc++] = HERE;
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

byte *funcName(ushort num, byte *name) {
    name[0] = num / (26 * 26) + 'A';
    name[1] = num / (26) + 'A';
    name[2] = num % 26 + 'A';
    name[3] = 0;
    return name;
}

inline void makeImmediate() { LAST->func |= 0x8000; }
inline void makeInline() { LAST->func |= 0x4000; }

void parse(const char *line) {
    toIn = (char *)line;
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
        DICT_T* dp = find(pad);
        if (dp) {
            int imm = (dp->func & 0x8000) ? 1 : 0;
            int inl = (dp->func & 0x4000) ? 1 : 0;
            ushort funcNum = dp->func & 0x3fff;
            byte x[4];
            funcName(funcNum, (byte *)x);
            if ((STATE == 0) || (imm)) {
                run((addr)&x);
            } else {
                if (inl) {
                    addr cp = func[funcNum];
                    while (*cp != ';') { cComma(*(cp++));  }
                } else { 
                    addr cp = x;
                    while (*cp) { cComma(*(cp++)); }
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
        if (strEquals(pad, "inline") && (STATE == 0)) {
            makeInline();
            continue;
        }
        if (strEquals(pad, "immediate") && (STATE == 0)) {
            makeImmediate();
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

void parseF(const char* fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    parse(buf);
}

void prim(const char* name, const char* code) {
    create((char*)name);
    makeInline();
    while (*code) { cComma(*(code++)); }
    cComma(';');
}

void baseSystem() {
    static char src[32];
    HERE = &user[0];
    VHERE = &var[0];
    USER_END = (USER + USER_SZ - 1);
    LAST = (DICT_T*)(USER_END + 1);
    STATE = 0;
    BASE = 10;
    parseF(": cell %ld ;      inline", (CELL)CELL_SZ);
    parseF(": addr %ld ;      inline", (CELL)ADDR_SZ);
    parseF(": dentry-sz %ld ; inline", (CELL)sizeof(DICT_T));
    parseF(": user-sz %ld ;   inline", (CELL)USER_SZ);
    parseF(": vars-sz %ld ;   inline", (CELL)VARS_SZ);
    parseF(": user %ld ;", (CELL)USER);
    parseF(": vars %ld ;", (CELL)VAR);
    parseF(": (here) %ld ;", (CELL)&HERE);
    parseF(": (vhere) %ld ;", (CELL)&VHERE);
    parseF(": (last) %ld ;", (CELL)&LAST);
    parseF(": base %ld ;", (CELL)&BASE);
    parseF(": state %ld ;", (CELL)&STATE);
    parseF(": user-end %ld ;", (CELL)USER_END);
    parseF(": >in %ld ;", (CELL)&toIn);
    parseF(": pad %ld ;", (CELL)pad);
    prim("swap", "$");
    prim("over", "%");
    prim("drop", "\\");
    prim("@", "@");
    prim("c@", "c@");
    prim("!", "!");
    prim("c!", "c!");
    prim("+", "+");
    prim("(.)", ".");
    prim("bye", "xQ");
    parse(": [ 0 state ! ; immediate");
        parse(": ] 1 state ! ;");
    parse(": nip swap drop ; inline");
}


void forthInit() {
    static char src[32];
    HERE = &user[0];
    VHERE = &var[0];
    USER_END = (USER + USER_SZ - 1);
    LAST = (DICT_T*)(USER_END + 1);
    STATE = 0;
    BASE = 10;
    baseSystem();
}
