// MinForth.cpp : An extremely memory conscious Forth interpreter
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define ADDR_SZ   2
#define CELL_SZ   4
#define MEM_SZ   (64*1024)
#define CODE_SZ  (24*1024)
#define STK_SZ    15

typedef unsigned char byte;
typedef short WORD;

#if CELL_SZ == 2
typedef short CELL;
typedef unsigned short UCELL;
#else
typedef long CELL;
typedef unsigned long UCELL;
#endif

#if ADDR_SZ == 2
typedef unsigned short ADDR;
#else
typedef unsigned long ADDR;
#endif

typedef struct {
    ADDR next;
    ADDR xt;
    byte flags;
    char name[16];
} dict_t;

typedef struct {
    ADDR pc;
    ADDR last;
    ADDR here;
    ADDR vhere;
    CELL base;
    CELL state;
    byte ir;
    byte dsp;
    byte rsp;
    byte unused1;
    CELL dstack[STK_SZ + 1];
    CELL rstack[STK_SZ + 1];
    byte mem[MEM_SZ];
} sys_t;

sys_t sys;
char word[32];

#define PC   sys.pc
#define IR   sys.ir
#define DSTK sys.dstack
#define RSTK sys.rstack
#define DSP  sys.dsp
#define RSP  sys.rsp
#define MEM  sys.mem
#define BASE sys.base
#define STATE sys.state
#define HERE sys.here
#define LHERE sys.lhere
#define VHERE sys.vhere
#define LAST sys.last

#define BYTE_AT(x) (MEM[x])
#define STR_AT(x) (char *)&MEM[x]
#define DP_AT(x) (dict_t *)&MEM[x]

WORD GET_WORD(ADDR l) { return BYTE_AT(l) | (BYTE_AT(l+1) << 8); }
void SET_WORD(ADDR l, WORD v) { BYTE_AT(l) = (v & 0xff); BYTE_AT(l + 1) = (byte)(v >> 8); }

long GET_LONG(ADDR l) { return GET_WORD(l) | (GET_WORD(l+2) << 16); }
void SET_LONG(ADDR l, CELL v) { SET_WORD(l, v & 0xFFFF); SET_WORD(l+2, (WORD)(v >> 16)); }

#if CELL_SZ == 2
CELL GET_CELL(ADDR l) { return GET_WORD(l); }
void SET_CELL(ADDR l, CELL v) { SET_WORD(l, v); }
#else
CELL GET_CELL(ADDR l) { return GET_LONG(l); }
void SET_CELL(ADDR l, CELL v) { SET_LONG(l, v); }
#endif

#if ADDR_SZ == 2
ADDR GET_ADDR(ADDR l) { return GET_WORD(l); }
void SET_ADDR(ADDR l, ADDR v) { SET_WORD(l, v); }
#else
ADDR GET_ADDR(ADDR l) { return GET_WORD(l); }
void SET_ADDR(ADDR l, ADDR v) { SET_LONG(l, v); }
#endif

#define T DSTK[DSP]
#define R RSTK[RSP]
#define SN(x) (x & STK_SZ)

#define CCOMMA(x) MEM[HERE++] = x
#define VCCOMMA(x) MEM[VHERE++] = x

void push(CELL v) { DSP = SN(DSP + 1);  DSTK[DSP] = v; }
CELL pop() { CELL x = T; DSP = SN(DSP - 1);  return x; }

void rpush(CELL v) { RSP = SN(RSP + 1);  RSTK[RSP] = v; }
CELL rpop() { CELL x = R; RSP = SN(RSP - 1);  return x; }

inline void cComma() { CCOMMA((byte)pop()); }

void Comma(CELL v) {
    SET_CELL(HERE, v);
    HERE += 2;
}

byte strEq(const char* x, const char* y) {
    while (*x && *y && (*x == *y)) {
        ++x; ++y;
    }
    return (*x || *y) ? 0 : 1;
}

int strLen(const char* str) {
    int l = 0;
    while (*str) {
        l++;
        str++;
    }
    return l;
}

void strCpy(char* d, const char* s) {
    while (*s) { *(d++) = *(s++); }
    *d = 0;
}

void doCreate(const char* name, byte f) {
    int len = strLen(name);
    ADDR x = (LAST) ? LAST : (MEM_SZ-4);
    x -= ((ADDR_SZ*2) + len + 2);
    while (x%4) { --x; }
    dict_t* dp = DP_AT(x);
    dp->next = LAST;
    dp->xt = HERE;
    dp->flags = f;
    strCpy(dp->name, name);
    LAST = x;
}

dict_t* doFind(const char* name) {
    dict_t* dp = DP_AT(LAST);
    while (dp) {
        if (strEq(dp->name, name)) { return dp; }
        if (dp->next) { dp = DP_AT(dp->next); }
        else { dp = NULL; }
    }
    return dp;
}

void doWords() {
    dict_t* dp = DP_AT(LAST);
    while (dp) {
        printf("%s ", dp->name);
        if (dp->next) { dp = DP_AT(dp->next); }
        else { dp = NULL; }
    }
}

void str2Here(const char* str) {
    while (*str) {
        CCOMMA(*(str++));
    }
}

int isDigit(char c) {
    if (('0' <= c) && (c <= '9')) { return c - '0'; }
    return -1;
}

void reset() {
    DSP = 0;
    RSP = 0;
    BASE = 10;
    HERE = 2;
    LAST = 0;
    VHERE = CODE_SZ;
}

void run(ADDR start) {
    PC = start;
    int rdepth = 0;
    CELL t1, t2;
    while (1) {
        IR = MEM[PC++];
        switch (IR) {
        case 0: return;
        case ' ': break;
        case 1: push(BYTE_AT(PC++));         break;
        case 2: push(GET_WORD(PC)); PC += 2; break;
        case 4: push(GET_LONG(PC)); PC += 4; break;
        case '#': push(T);                   break;
        case '\\': pop();                    break;
        case '+': t1 = pop(); T += t1;       break;
        case '-': t1 = pop(); T -= t1;       break;
        case '*': t1 = pop(); T *= t1;       break;
        case '/': t1 = pop(); T /= t1;       break;
        case '.': printf("%d", pop());       break;
        case ',': printf("%c", (byte)pop()); break;
        case 'b': printf(" ");               break;
        case 'c': T = MEM[T];                break;
        case 'C': t1 = pop(); t2 = pop();
            MEM[t1] = (byte)t2;
            break;
        case '@': if (T < (MEM_SZ - CELL_SZ)) { T = GET_CELL((ADDR)T); } break;
        case '!': t1 = pop(); t2 = pop();
            SET_CELL((ADDR)t1, t2);
            break;
        case 'n': printf("\r\n");            break;
        case 'W': doWords();                 break;
        case '$': t1 = pop(); t2 = pop();    // SWAP
            push(t1); push(t2);
            break;
        case '%': t1 = pop(); t2 = T;        // OVER
            push(t1); push(t2);
            break;
        case ':': rpush(PC + 2); PC = GET_ADDR((ADDR)PC);
            ++rdepth;
            break;
        case ';': if (rdepth < 1) { return; }
                PC = (ADDR)rpop();
                --rdepth;
                break;
        case 'Z': RSP = 99; return;
        }
    }
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
    if (STATE) {
        if (dp->flags == 2) { CCOMMA((byte)dp->xt); }
        else {
            CCOMMA(':');
            Comma(dp->xt);
        }
    }
    else {
        if (dp->flags == 2) {
            MEM[HERE] = ((byte)dp->xt);
            MEM[HERE + 1] = ';';
            run(HERE);
        }
        else {
            run(dp->xt);
        }
    }
}

int isNum(char* x) {
    while (*x) {
        if (isDigit(*(x++)) == -1) { return 0; }
    }
    return 1;
}

void compileNumber(CELL num) {
    if ((0 <= num) && (num <= 0xFF)) {
        CCOMMA(1);
        CCOMMA(num & 0xff);
    }
    else if ((0x0100 <= num) && (num <= 0xFFFF)) {
        CCOMMA(2);
        SET_WORD(HERE, (WORD)num);
        HERE += 2;
    }
    else {
        CCOMMA(4);
        SET_LONG(HERE, (WORD)num);
        HERE += 4;
    }
}

int doParseNum(char* wd) {
    // TODO: support neg
    // TODO: support other bases
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
        CCOMMA(';');
        return 1;
    }

    if (strEq(wd, "VARIABLE")) {
        if (getWord(wd, ' ')) {
            doCreate(wd, 0);
            compileNumber(VHERE);
            CCOMMA(';');
            SET_CELL(VHERE, 0);
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

void doBuiltin(const char* name, const char* code) {
    doCreate(name, 2);
    dict_t* dp = (dict_t*)&MEM[LAST];
    dp->xt = code[0];
}

void doOK() {
    int d = DSP;
    printf(" OK (");
    for (int d = 1; d <= DSP; d++) { printf("%s%d", (1 < d) ? " " : "", DSTK[d]); }
    printf(")\r\n");
}

void loop() {
    char* tib = STR_AT(VHERE);
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
    ADDR x;
    printf("mem usage: %d\n", sizeof(sys));
    reset();
    char *cp = STR_AT(HERE+4); sprintf(cp, ": CELL %d ; : ADDR %d ;", CELL_SZ, ADDR_SZ);
    doParse(cp);
    doParse("");
    doBuiltin("SWAP", "$");
    doBuiltin("DROP", "\\");
    doBuiltin("DUP", "#");
    doBuiltin("OVER", "%");
    doBuiltin("EMIT", ",");
    doBuiltin("WORDS", "W");
    doBuiltin("CR", "n");
    doBuiltin("(.)", ".");
    doBuiltin("+", "+");
    doBuiltin("-", "-");
    doBuiltin("*", "*");
    doBuiltin("/", "/");
    doBuiltin("C@", "c");
    doBuiltin("C!", "C");
    doBuiltin("@", "@");
    doBuiltin("!", "!");
    doBuiltin("BYE", "Z");
    doBuiltin("ZZ", "Z");
    x = HERE; str2Here(": . 32 EMIT (.) ;"); HERE = x; doParse(STR_AT(x));
    // x = HERE; str2Here(""); HERE = x; run(HERE);
    printf("\r\nMinForth v0.0.1");
    printf("\r\nMEM: %ld, CODE: %ld, HERE: %ld", MEM_SZ, CODE_SZ, HERE);
    printf("\r\nVARS: %ld, VHERE: %ld", CODE_SZ, VHERE);
    printf("\r\nLAST: %d", LAST);
    printf("\r\nHello.");
    while (RSP != 99) {
        loop();
    }
}
