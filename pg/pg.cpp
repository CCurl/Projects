// pg.cpp - a stack-based interpreter

// Windows PC (Visual Studio)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define isPC
#endif

#include <stdio.h>
#include <time.h>

extern void printString(const char *s);
extern void printChar(const char c);

#define MEM_SZ          64000
#define VARS_SZ        256000
#define STK_SZ             32
#define LSTK_SZ            30
#define NAME_LEN            9

enum { 
    STOP = 0,
    LIT1, LIT4, DOTS,
    IF, ELSE, THEN, 
    STORE, CSTORE, FETCH, CFETCH, 
    CALL, EXIT, JMP, JMPZ,
    DUP, SWAP, OVER, DROP,
    ADD, SUB, MULT, DIV, INC, DEC,
    LT, EQ, GT, NOT,
    DO, INDEX, LOOP, 
    BEGIN, WHILE, UNLOOP,
    EMIT, DOT, TIMER,
    CCOMMA, COMMA,
    CREATE, GETWORD, ISNUM, STATE0, STATE1,
    DEFINE, ENDWORD, IMMED, INLINE,
    NOOP, BYE
};

#define IS_IMMEDIATE  1
#define IS_INLINE     2

typedef struct { int op; int flg; const char *name; } opcode_t;
opcode_t opcodes[] = { 
    { BYE,       IS_INLINE,    "bye" },     { DOTS,    IS_INLINE, ".s" }
    , { IF,      IS_IMMEDIATE, "if" },      { ELSE,    IS_IMMEDIATE, "else" } , { THEN,    IS_IMMEDIATE, "then" }
    , { EXIT,    IS_INLINE,    "exit" }
    , { DEFINE,  IS_INLINE,    ":" },       { ENDWORD, IS_IMMEDIATE, ";" }
    , { INLINE,  IS_INLINE,    "inline" },  { IMMED,   IS_INLINE,    "immediate" }
    , { STATE0,  IS_IMMEDIATE, "[" } ,      { STATE1,  IS_INLINE,    "]" }
    , { CREATE,  IS_INLINE,    "create" }
    , { GETWORD, IS_INLINE,    "getword" }, { ISNUM,   IS_INLINE,    "number?" }
    , { TIMER,   IS_INLINE,    "timer" }
    , { DUP,     IS_INLINE,    "dup" },     { SWAP,    IS_INLINE,    "swap" }
    , { OVER,    IS_INLINE,    "over" },    { DROP,    IS_INLINE,    "drop" }
    , { EMIT,    IS_INLINE,    "emit" },    { DOT,     IS_INLINE,    "." }
    , { ADD,     IS_INLINE,    "+" },       { SUB,     IS_INLINE,    "-" }
    , { MULT,    IS_INLINE,    "*" },       { DIV,     IS_INLINE,    "/" }
    , { LT,      IS_INLINE,    "<" },       { EQ,      IS_INLINE,    "=" }
    , { GT,      IS_INLINE,    ">" },       { NOT,     IS_INLINE,    "0=" }
    , { INC,     IS_INLINE,    "1+" },      { DEC,     IS_INLINE,    "1-" }
    , { DO,      IS_INLINE,    "do" },      { INDEX,   IS_INLINE,    "i" }
    , { LOOP,    IS_INLINE,    "loop" },    { UNLOOP,  IS_INLINE,    "unloop" }
    , { BEGIN,   IS_INLINE,    "begin" },   { WHILE,   IS_INLINE,    "while" }
    , { STORE,   IS_INLINE,    "!" } ,      { CSTORE,  IS_INLINE,    "c!" }
    , { FETCH,   IS_INLINE,    "@" } ,      { CFETCH,  IS_INLINE,    "c@" }
    , { COMMA,   IS_INLINE,    "," } ,      { CCOMMA,  IS_INLINE,    "c," }
    , { 0, 0, 0 }
};

#define TOS (*sp)
#define NOS (*(sp-1))
#define PUSH(x) push((cell_t)(x))
#define DROP1 sp--
#define DROP2 sp-=2
#define RET(x) push(x); return;
#define NEXT goto next

#define BTW(a,b,c) ((b<=a) && (a<=c))
#define clearTib fill(tib, 0, sizeof(tib))
#define PRINT1(a)     printString(a)
#define PRINT2(a,b)   PRINT1(a); PRINT1(b)
#define PRINT3(a,b,c) PRINT2(a,b); PRINT1(c)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]

#define BYTES(x)      mem.b[x]
#define CELLS(x)      mem.c[x]

typedef long cell_t;
typedef unsigned char byte;
union { cell_t c[MEM_SZ/sizeof(cell_t)]; char b[MEM_SZ]; } mem;
char vars[VARS_SZ], *vhere;
typedef struct { char f; char len; char name[NAME_LEN+1]; char *xt; } dict_t;

cell_t stk[STK_SZ], *sp, rsp;
char *rstk[STK_SZ];
cell_t state, base;
cell_t lstk[LSTK_SZ+1], lsp;
char *here, *pc, tib[128], *in;
dict_t *last;

#ifdef isPC
FILE *input_fp;
int isBye;
#endif

void push(cell_t x) { *(++sp) = (cell_t)(x); }
cell_t pop() { return *(sp--); }

void CComma(cell_t x) { *(here++) = (char)x; }
void Comma(cell_t x) { *(cell_t*)here = x; here += sizeof(cell_t); }

void Store(char *loc, cell_t x) { *(cell_t*)loc = x; }
cell_t Fetch(char *loc) { return *(cell_t*)loc; }

void fill(char *d, char val, int num) { for (int i=0; i<num; i++) { *(d++)=val; } }
char *strEnd(char *s) { while (*s) ++s; return s; }
void strCat(char *d, const char *s) { d=strEnd(d); while (*s) { *(d++)=*(s++); } *d=0; }
void strCpy(char *d, const char *s) { *d = 0; strCat(d, s); }
int strLen(char *d) { int len = 0; while (*d++) { ++len; } return len; }
int lower(int x) { return BTW(x,'A','Z') ? x+32: x; }

int strEq(char *d, char *s, int caseSensitive) {
    while (*s || *d) {
        if (caseSensitive) { if (*s != *d) return 0; }
        else { if (lower(*s) != lower(*d)) return 0; }
        s++; d++;
    }
    return -1;
}

void Create(char *w) {
    int l = strLen(w);
    --last;
    if (NAME_LEN < l) {
        PRINT2("-trunc:[", w);
        l=NAME_LEN; w[l]=0;
        PRINT3("->", w, "]-");
    }
    strCpy(last->name, w);
    last->len = l;
    last->xt = here;
    last->f = 0;
}

// ( nm--xt flags 1 )
// ( nm--0 )
void find() {
    char *nm = (char*)pop();
    dict_t *x = last;
    dict_t *end = (dict_t*)&BYTES(MEM_SZ);
    while (x < end) {
        if (strEq(nm, x->name, 0)) {
            PUSH(x->xt);
            push(x->f);
            RET(1);
        }
        ++x;
    }
    push(0);
}

// ( --n 1 )
// ( --0 )
void isDecimal(const char *wd) {
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { RET(0); }
    while (BTW(*wd, '0', '9')) { x = (x * 10) + (*(wd++) - '0'); }
    if (*wd) { RET(0); }
    push(isNeg ? -x : x);
    RET(1);
    // For Floating point support
    // if (*wd && (*wd != '.')) { RET(0); }
    // if (*wd == 0) { push(isNeg ? -x : x); RET(1); }
    // // Must be a '.', make it a float
    // ++wd;
    // float fx = (float)x, d = 10;
    // while (BTW(*wd, '0', '9')) { fx += (*(wd++) - '0') / d; d *= 10; }
    // if (*wd) { RET(0); }
    // push(0);
    // // FTOS = isNeg ? -fx : fx;
    // RET(1);
}

// ( nm--n 1 )
// ( nm--0 )
void isNum() {
    char *wd = (char*)pop();
    if ((wd[0] == '\'') && (wd[2] == '\'') && (wd[3] == 0)) { push(wd[1]); RET(1); }
    int b = base, lastCh = '9';
    if (*wd == '#') { b = 10;  ++wd; }
    if (*wd == '$') { b = 16;  ++wd; }
    if (*wd == '%') { b = 2;  ++wd; lastCh = '1'; }
    if (b == 10) { isDecimal(wd); return; }
    if (b < 10) { lastCh = '0' + b - 1; }
    if (*wd == 0) { RET(0); }
    cell_t x = 0;
    while (*wd) {
        cell_t t = -1, c = *(wd++);
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((b == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((b == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { RET(0); }
        x = (x * b) + t;
    }
    push(x);
    RET(1);
}

void dotS(const char *pre, const char *post) {
    if (sp < stk) { sp = stk; PRINT1("-su-"); }
    if (&stk[STK_SZ] < sp) { sp = stk; PRINT1("-so-"); }
    if (pre) fputs(pre, stdout);
    putchar('(');
    for (cell_t x=0, *p=&stk[1];p<=sp;++p) {
        if (x)  printChar(' ');
        printf("%ld", *p);
        x = 1;
    }
    putchar(')');
    if (post) fputs(post, stdout);
}

void getInput() {
    clearTib;
    if (input_fp) {
        in = fgets(tib, sizeof(tib), input_fp);
        if (in != tib) { fclose(input_fp); input_fp = NULL; }
    }
    if (! input_fp) {
        dotS("\nok:", "> ");
        in = fgets(tib, sizeof(tib), stdin);
    }
}

// ( --addr len )
// ( --0 )
void getword() {
    while (*in && (*in < 33)) { ++in; }
    if (*in == 0) { RET(0); }
    PUSH(in);
    push(0);
    while (32 < *in) { ++in; ++TOS; }
    *(in++) = 0;
}

void resolve(cell_t t) {  Store((char *)t, (cell_t)here); }

void Run(char *y) {
    cell_t t1;
    pc = y;

next:
    switch (*(pc++)) {
    case STOP:                                                 return;
    case LIT1: push(*(pc++));                                   NEXT;
    case LIT4: push(*(cell_t*)pc); pc += sizeof(cell_t);        NEXT;
    case DOTS: dotS(0, 0);                                      NEXT;
    case IF: CComma(JMPZ); PUSH(here); Comma(0);                NEXT;
    case ELSE: t1=pop(); CComma(JMP); PUSH(here); Comma(0); resolve(t1);    NEXT;
    case THEN: resolve(pop());                                  NEXT;
    case CALL: if (*pc != EXIT) { rstk[++rsp] = (pc+sizeof(cell_t)); }
            pc = *(char**)pc;                                   NEXT;
    case EXIT: if (rsp<1) { rsp=0; return; } pc=rstk[rsp--];    NEXT;
    case JMP: pc = *(char**)pc;                                 NEXT;
    case JMPZ: if (pop()==0) { pc = *(char**)pc; }
            else { pc += sizeof(cell_t); }                      NEXT;
    case STORE: Store((char*)TOS, NOS); DROP2;                  NEXT;
    case CSTORE: *(char*)TOS = (char)NOS; DROP2;                NEXT;
    case FETCH: TOS = Fetch((char*)TOS);                        NEXT;
    case CFETCH: TOS = *(char*)TOS;                             NEXT;
    case COMMA: Comma(pop());                                   NEXT;
    case CCOMMA: CComma(pop());                                 NEXT;
    case DUP: push(TOS);                                        NEXT;
    case SWAP: t1=TOS; TOS=NOS; NOS=t1;                         NEXT;
    case OVER: push(NOS);                                       NEXT;
    case DROP: DROP1;                                           NEXT;
    case ADD: NOS += TOS; DROP1;                                NEXT;
    case SUB: NOS -= TOS; DROP1;                                NEXT;
    case MULT: NOS *= TOS; DROP1;                               NEXT;
    case DIV: NOS /= TOS; DROP1;                                NEXT;
    case LT: NOS = (NOS <  TOS) ? -1 : 0; DROP1;                NEXT;
    case EQ: NOS = (NOS == TOS) ? -1 : 0; DROP1;                NEXT;
    case GT: NOS = (NOS >  TOS) ? -1 : 0; DROP1;                NEXT;
    case NOT: TOS = (TOS) ? 0: -1;                              NEXT;
    case EMIT: printChar((char)pop());                          NEXT;
    case DOT: printf("%ld ", pop());                            NEXT;
    case TIMER: push(clock());                                  NEXT;
    case DEC: --TOS;                                            NEXT;
    case INC: ++TOS;                                            NEXT;
    case CREATE: getword(); if (pop()) Create((char*)pop());    NEXT;
    case DO: lsp+=3; L2=(cell_t)pc; L0=pop(); L1=pop();         NEXT;
    case LOOP: if (++L0<L1) { pc=(char*)L2; } else { lsp-=3; }; NEXT;
    case UNLOOP: if (0<lsp) { lsp-=3; };                        NEXT;
    case INDEX: push(L0);                                       NEXT;
    case BEGIN: lsp+=3; L0=(cell_t)pc;                          NEXT;
    case WHILE: if (pop()) { pc=(char*)L0; } else { lsp-=3; };  NEXT;
    case GETWORD: getword();                                    NEXT;
    case DEFINE: getword(); if (pop()) { Create((char*)pop()); state=1; }   NEXT;
    case ENDWORD: state=0; CComma(EXIT);                        NEXT;
    case STATE0: state=0;                                       NEXT;
    case STATE1: state=1;                                       NEXT;
    case ISNUM: isNum();                                        NEXT;
    case IMMED: last->f |= IS_IMMEDIATE;                        NEXT;
    case INLINE: last->f |= IS_INLINE;                          NEXT;
    case BYE: isBye=1;                                         return;
    default: printf("-[%d]?-",(int)*(pc-1));  break;
    }
}

void ParseWord() {
    char *w = (char*)TOS;
    isNum();
    if (pop()) {
        if (state) {
            if (BTW(TOS,0,255)) { CComma(LIT1); CComma(pop()); }
            else { CComma(LIT4); Comma(pop()); }
        }
        RET(1);
    }
    PUSH(w);
    find();
    if (pop()) {
        cell_t f = pop();
        char *xt = (char*)pop();
        if ((state == 0) || (f & IS_IMMEDIATE)) { Run(xt); RET(1); }
        if (f & IS_INLINE) {
            CComma(*(xt++));
            while ((*xt) && (*xt != EXIT)) { CComma(*(xt++)); }
        }
        else { CComma(CALL); Comma((cell_t)xt); }
        RET(1);
    }
    PRINT3("[", w, "]??");
    if (state) {
        ++last;
        state = 0;
    }
    base = 10;
    RET(0);
}

void ParseLine(char *x) {
    in = x;
    while (isBye == 0) {
        getword();
        if (pop() == 0) break;
        ParseWord();
        if (pop() == 0) return;
    }
}

void loadLine(const char *x) {
    in = tib;
    while (*x) { *(in++) = *(x++); }
    *in = 0;
    ParseLine(tib);
}

void loadNum(const char *name, cell_t addr) {
    clearTib;
    sprintf(tib, ": %s %ld ;", name, addr);
    ParseLine(tib);
}

void init() {
    here = &BYTES(0);
    vhere = &vars[0];
    last = (dict_t*)&BYTES(MEM_SZ);
    sp = stk;
    in = tib;
    base = 10;
    rsp = 0;
    opcode_t *op = opcodes;
    while (op->op) {
        Create((char*)op->name);
        last->f = op->flg;
        CComma(op->op);
        CComma(EXIT);
        ++op;
    }
    loadNum("mem-sz", MEM_SZ);
    loadNum("vars-sz", VARS_SZ);
    loadNum("(mem)", (cell_t)&mem.b[0]);
    loadNum("word-sz", sizeof(dict_t));
    loadNum("cell", sizeof(cell_t));
    loadNum("(last)", (cell_t)&last);
    loadNum("(last)", (cell_t)&last);
    loadNum("(here)", (cell_t)&here);
    loadNum("(vhere)", (cell_t)&vhere);
    loadNum(">in", (cell_t)&in);
    loadNum("state", (cell_t)&state);
    loadNum("base", (cell_t)&base);
}

#ifdef isPC
void printChar(const char c) { putchar(c); }
void printString(const char* s) { fputs(s, stdout); }

int main(int argc, char *argv[]) {
    // int r='A';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG(r++) = atoi(y); }
    init();
    input_fp = fopen("init.f", "rt");
    while (isBye == 0) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}
#endif
