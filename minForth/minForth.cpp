// MinForth.cpp : An extremely memory conscious Forth interpreter

#include "Shared.h"

typedef struct {
    const char *name;
    const char *op;
} PRIM_T;

// Words that directly map to VM operations
PRIM_T prims[] = {
    {"+","+"}, {"-","-"}, {"/","/"}, {"*","*"},{"/mod","&"},
    {"swap","$"}, {"drop","\\"}, {"over","%"}, {"dup","#"},
    {"emit",","}, {"(.)","."}, {"space","b"}, {"cr","n"}, {"bl","k"},
    {"=","="}, {"<","<"}, {">",">"}, {"0=","N"},
    {"<<","L"}, {">>","R"}, {"type","Z"}, {".",".b"},
    {"@","@"}, {"c@","c"}, {"w@","w"}, {"!","!"}, {"c!","C"}, {"w!","W"},
    {"and","a"}, {"or","o"}, {"xor","x"}, {"com","~"}, {"not","N"},
    {"1+","i"}, {"1-","d"}, {"I", "I"}, {"+I", "m"}, {"execute","G"},
    {"leave",";"}, {"timer","t"}, {"reset","Y"}, {"break","^"},
    {"+tmps","p"}, {"-tmps","q"}, 
    {"bye","zZ"}, 
#if __BOARD__ == PC
    // Extensions
    {"load","zL"}, { "edit","zE" },
#elif __GAMEPAD__
    // Extensions
    { "gp-button","xGB" },
#endif
    {0,0}
};

char word[32], *in;
byte lastWasCall = 0, isBye = 0;
byte *VHERE, *VHERE2;
CELL HERE, LAST, STATE, tempWords[10];

void CComma(CELL v) { user[HERE++] = (byte)v; }
void Comma(CELL v) { SET_LONG(&user[HERE], v); HERE += CELL_SZ; }
void WComma(WORD v) { SET_WORD(&user[HERE], v); HERE += 2; }

char lower(char c) { return betw(c, 'A', 'Z') ? (c + 32) : c; }

byte strEq(const char* x, const char* y) {
    while (*x && *y && (*x == *y)) { ++x; ++y; }
    return (*x || *y) ? 0 : 1;
}

byte strEqI(const char* x, const char* y) {
    while (*x && *y) {
        if (lower(*x) != lower(*y)) { return 0; }
        ++x; ++y;
    }
    return (*x || *y) ? 0 : 1;
}

char *strCpy(char* d, const char* s) {
    while (*s) { *(d++) = *(s++); }
    *d = 0;
    return d;
}

int strLen(const char* str) {
    int l = 0;;
    while (*(str++)) { ++l; }
    return l;
}

void printStringF(const char* fmt, ...) {
    char* buf = (char*)&user[USER_SZ-64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 100, fmt, args);
    va_end(args);
    printString(buf);
}

BOOL isTempWord(const char* nm) {
    return ((nm[0] == 'T') && betw(nm[1], '0', '9') && (nm[2] == 0));
}

void doCreate(const char* name, byte f) {
    if (isTempWord(name)) {
        tempWords[name[1] - '0'] = HERE;
        return;
    }
    DICT_T *dp = DP_AT(HERE);
    dp->prev = (byte)(HERE - LAST);
    dp->flags = f;
    strCpy(dp->name, name);
    LAST = HERE;
    HERE += strLen(name) + 3;
}

int doFind(const char* name) {
    // Temporary word?
    if (isTempWord(name) && (tempWords[name[1] - '0'])) {
        push(tempWords[name[1] - '0']);
        push(0);
        return 1;
    }
    CELL def = (WORD)LAST;
    while (def) {
        DICT_T* dp = DP_AT(def);
        if (strEq(dp->name, name)) {
            push(def + strLen(dp->name) + 3);
            push(dp->flags);
            return 1;
        }
        if (def == dp->prev) break;
        def -= dp->prev;
    }
    return 0;
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

int execWord() {
    CELL f = pop(), xt = pop();
    if ((STATE) && (f == 0)) {
        CComma(':');
        WComma((WORD)xt);
        lastWasCall = 1;
    } else { run((WORD)xt); }
    return 1;
}

int doNumber(int state) {
    if (state) {
        CELL num = pop();
        if ((num & 0xFF) == num) {
            CComma('1');
            CComma(num & 0xff);
        }
        else if ((num & 0xFFFF) == num) {
            CComma('2');
            WComma((WORD)num);
        }
        else {
            CComma('4');
            Comma(num);
        }
    }
    return 1;
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

int doPrim(const char* wd) {
    const char* vml = NULL;
    if (betw(wd[0],'r','s') && betw(wd[1],'0','9') && (!wd[2])) {
        vml = wd;
    }
    for (int i = 0; prims[i].op && (!vml); i++) {
        if (strEqI(prims[i].name, wd)) { vml = prims[i].op; }
    }
    if (!vml) { return 0; }
    if (STATE) {
        for (int j = 0; vml[j]; j++) { CComma(vml[j]); }
    } else {
        byte* x = UA(HERE + 10);
        for (int j = 0; vml[j]; j++) { *(x++) = vml[j]; }
        *x = ';';
        run((WORD)HERE + 10);
    }
    return 1;
}

int doQuote() {
    in++;
    if (STATE) { VHERE2 = VHERE; }
    push((CELL)VHERE2);
    while (*in && (*in != '"')) { *(VHERE2++) = *(in++); }
    *(VHERE2++) = 0;
    if (*in) { ++in; }
    if (STATE) {
        CComma('4');
        Comma(pop());
        VHERE = VHERE2;
    }
    return 1;
}

int doDotQuote() {
    doQuote();
    if (STATE) {
        CComma('Z');
    } else {
        byte* cp = UA(HERE+10);
        *(cp++) = 'Z';
        *(cp++) = ';';
        run((WORD)(HERE+10));
    }
    return 1;
}

int doParseWord(char* wd) {
    byte lwc = lastWasCall;
    lastWasCall = 0;

    if (doPrim(wd)) { return 1; }
    if (doFind(wd)) { return execWord(); }
    if (isNum(wd)) { return doNumber(STATE); }
    if (strEq(wd, ".\"")) { return doDotQuote(); }
    if (strEq(wd, "\"")) { return doQuote(); }

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
            push((CELL)VHERE);
            doNumber(1);
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

    if (strEqI(wd, "AGAIN")) {
        CComma('}');
        CELL tgt = pop();
        SET_WORD(UA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "WHILE")) {
        CComma('v');
        CELL tgt = pop();
        SET_WORD(UA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "UNTIL")) {
        CComma('u');
        CELL tgt = pop();
        SET_WORD(UA(tgt), (WORD)HERE);
        return 1;
    }

    if (strEqI(wd, "FORGET")) {
        HERE = LAST;
        LAST -= U(LAST);
        return 1;
    }

    STATE = 0;
    VHERE2 = VHERE;
    printStringF("[%s]??", wd);
    return 0;
}

void doParse(const char* line) {
    in = (char*)line;
    int len = getWord(word, ' ');
    while (0 < len) {
        if (VHERE2 < VHERE) { VHERE2 = VHERE; }
        if (strEq(word, "//")) { return; }
        if (strEq(word, "\\")) { return; }
        if (doParseWord(word) == 0) { return; }
        len = getWord(word, ' ');
    }
}

void doOK() {
    if (STATE) { printString(" ... "); return; }
    printString("\r\nOK (");
    for (int d = 1; d <= sp; d++) {
        if (1 < d) { printChar(' '); }
        printBase(stk[d], BASE);
    }
    printString(")>");
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
    sprintf(cp, ": x ;");                       doParse(cp);
    sprintf(cp, ": u %lu ;", (UCELL)user);      doParse(cp);
    sprintf(cp, ": usz %d ;", USER_SZ);         doParse(cp);
    sprintf(cp, ": vsz %d ;", VARS_SZ);         doParse(cp);
    sprintf(cp, ": ha %lu ;", (UCELL)&HERE);    doParse(cp);
    sprintf(cp, ": la %lu ;", (UCELL)&LAST);    doParse(cp);
    sprintf(cp, ": va %lu ;", (UCELL)&VHERE);   doParse(cp);
    sprintf(cp, ": vb %lu ;", (UCELL)vars);     doParse(cp);
    sprintf(cp, ": base %lu ;", (UCELL)&BASE);  doParse(cp);
}

#if __BOARD__ == PC
FILE* input_fp = NULL;
FILE* fpStk[10];
byte fpSP = 0;

void fpPush(FILE* v) { if (fpSP < 9) { fpStk[++fpSP] = v; } }
FILE* fpPop() { return (fpSP) ? fpStk[fpSP--] : 0 ; }

WORD doLoad(CELL ir, CELL pc) {
    char* fn = (char*)(VHERE + 100);
    sprintf(fn, "./block-%03d.4th", pop());
    FILE* fp = fopen(fn, "rt");
    if (fp) {
        if (input_fp) { fpPush(input_fp); }
        input_fp = fp;
    }
    else { return 0; }
    return 1;
}

WORD doExt(CELL ir, WORD pc) {
    switch (ir) {
    case 'z': ir = U(pc++);
        if (ir == 'E') { doEditor(); }
        if (ir == 'L') { pc = doLoad(ir, pc); }
        if (ir == 'Z') { isBye = 1; }
        break;
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
    char *tib = (char *)UA(HERE+32);
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (fp == stdin) { doOK(); }
    if (fgets(tib, 100, fp) == tib) {
        if (fp == stdin) { doHistory(tib); }
        doParse(rtrim(tib));
        return;
    }
    if (input_fp) {
        fclose(input_fp);
        input_fp = fpPop();
    }
}

int main()
{
    vmReset();
    push(0);
    doLoad(0,0);
    while (!isBye) { loop(); }
}

#endif
