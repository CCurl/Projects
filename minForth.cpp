// MinForth.cpp : An extremely memory conscious Forth interpreter
//

#include <stdio.h>

typedef unsigned char byte;
typedef short CELL;
typedef unsigned short UCELL;

#define MEM_SZ  300
#define STK_SZ    7
#define LH_INIT 100
#define VH_INIT  70

typedef struct {
    CELL next;
    CELL xt;
    byte flags;
    char name[16];
} dict_t;

typedef struct {
    CELL pc;
    CELL last;
    CELL here;
    CELL lhere;
    CELL vhere;
    byte ir;
    byte base;
    byte state;
    byte dsp;
    byte rsp;
    CELL dstack[STK_SZ+1];
    CELL rstack[STK_SZ+1];
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

#define GET_CELL(l) (MEM[l] | (MEM[l+1]<<8))
#define SET_CELL(l, v) MEM[l] = (v&0xff); MEM[l+1] = (v>>8)
#define BYTE_AT(x) (MEM[x])
#define STR_AT(x) (char *)&MEM[x]
#define DP_AT(x) (dict_t *)&MEM[x]

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
    dict_t* dp = DP_AT(LHERE);
    dp->next = LAST;
    dp->xt = HERE;
    dp->flags = f;
    strCpy(dp->name, name);
    LAST = LHERE;
    LHERE = strLen(name) + LHERE + 6;
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

CELL doNum(CELL pc) {
    int c = isDigit(MEM[pc]);
    while (0 <= c) {
        T = (T * 10) + c;
        c = isDigit(MEM[++pc]);
    }
    return pc;
}

void reset() {
    DSP = 0;
    RSP = 0;
    BASE = 10;
    HERE = 2;
    LAST = 0;
    LHERE = LH_INIT;
    VHERE = VH_INIT;
}

void run(CELL start) {
    PC = start;
    int rdepth = 0;
    CELL t1, t2;
    while (1) {
        IR = MEM[PC++];
        switch (IR) {
        case 0: return;
        case ' ': break;
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
        case '@': if (T < (MEM_SZ-1)) { T = GET_CELL(T); } break;
        case '!': t1 = pop(); t2 = pop();
            SET_CELL(t1, t2);
            break;
        case 'n': printf("\r\n");            break;
        case 'W': doWords();                 break;
        case '$': t1 = pop(); t2 = pop();    // SWAP
            push(t1); push(t2);
            break;
        case '%': t1 = pop(); t2 = T;        // OVER
            push(t1); push(t2);
            break;
        case ':': rpush(PC+2); PC = GET_CELL(PC);
            ++rdepth;
            break;
        case ';': if (rdepth < 1) { return; }
                PC = rpop();
                --rdepth;
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            push(isDigit(IR));
            PC = doNum(PC);
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
        } else {
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

int doParseNum(char *wd) {
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

char* doParseWord(char* wd) {
    dict_t* dp = doFind(wd);
    if (dp) {
        execWord(dp);
        return wd;
    }

    if (doParseNum(wd)) {
        // TODO change this
        if (STATE) { CCOMMA(' '); str2Here(wd); pop(); }
        return wd;
    }

    if (strEq(wd, ":")) {
        if (getWord(wd, ' ')) {
            doCreate(wd, 0);
            STATE = 1;
        } else { return 0; }
        return wd;
    }

    if (strEq(wd, ";")) {
        STATE = 0;
        CCOMMA(';');
        return wd;
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
    // printf("\r\nHERE: %d, LAST: %d, VHERE: %d", HERE, LAST, VHERE);
    printf(" OK (");
    for (int d = 1; d <= DSP; d++) { printf("%s%d", (1<d)?" ":"", DSTK[d]); }
    printf(")\r\n");
}

void loop() {
    char *tib = STR_AT(VHERE);
    // FILE* fp = (input_fp) ? input_fp : stdin;
    FILE* fp = stdin;
    if (fp == stdin) { doOK(); }
    if (fgets(tib, 100, fp) == tib) {
        // if (fp == stdin) { doHistory(tib); }
        int l = strLen(tib)-1;
        while ((0<l) && (tib[l]) && (tib[l] < ' ')) { --l; }
        tib[l+1] = 0;
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
    CELL x;
    printf("mem usage: %d\n", sizeof(sys));
    reset();
    doParse(": CELL 2 ;");
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
    printf("\r\nMinForth v0.0.1\r\n");
    while (RSP != 99) {
        loop();
    }
}
