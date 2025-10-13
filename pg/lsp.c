/* Chris Curl, MIT license. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYMBOLS_SZ   1000
#define CODE_SZ     10000
#define HEAP_SZ     10000
#define SRC_SZ      50000

#define BTWI(n,l,h) ((l<=n)&&(n<=h))
#define BCASE break; case
#define DBG(...)
// #define DBG(...) printf(__VA_ARGS__)

typedef struct { char type, isReg, name[22], asm_name[8], *strVal; int sz; } SYM_T;
typedef struct { int op, a1, a2; char *s1, *s2; } INST_T;

// Tokens - NOTE: the first 8 must match the words list in tc.c
enum {
    NO_TOK, ELSE_TOK, IF_TOK, WHILE_TOK, VOID_TOK, INT_TOK, CHAR_TOK, RET_TOK
    , TOK_THEN, TOK_END, TOK_BEGIN, TOK_UNTIL, TOK_AGAIN, TOK_DEF, TOK_SYS
    , TOK_AND, TOK_OR, TOK_XOR
    , TOK_LBRA, TOK_RBRA, TOK_LPAR, TOK_RPAR, TOK_LARR, TOK_RARR, TOK_COMMA
    , TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_INC, TOK_DEC, TOK_PLEQ
    , TOK_AMP, TOK_LT, TOK_EQ, TOK_GT, TOK_NEQ, TOK_LOC, TOK_LOCSA, TOK_LOCSD
    , TOK_SET, TOK_NUM, TOK_ID, TOK_FUNC, TOK_STR
    , TOK_SEMI, EOI
};

// NOTE: these have to be in sync with the first <x> entries in the list of tokens
char *words[] = { "", "else", "if" , "while", "void", "int", "char" , "return",
    "then", "end", "begin", "until", "again", "def", "sys",
    "and", "or", "xor",
    NULL
};

int ch = 32, tok, int_val, tok_len, hHere = 0, numSymbols = 0;
char id_name[256], cur_line[256] = {0}, heap[HEAP_SZ];
int cur_off = 0, cur_lnum = 0, is_eof = 0, codeSz = 0;
SYM_T symbols[SYMBOLS_SZ];
INST_T code[CODE_SZ];
FILE *input_fp = NULL;
char srcBuf[SRC_SZ];

void statements(int endTok);

//---------------------------------------------------------------------------
// Utilities
void msg(int fatal, char *s) {
    fprintf(stderr, "\n%s at(%d, %d)", s, cur_lnum, cur_off);
    fprintf(stderr, "\n%s", cur_line);
    for (int i = 2; i < cur_off; i++) { fprintf(stderr, " "); } fprintf(stderr, "^\n");
    if (fatal) { exit(1); }
}

void syntax_error() { msg(1, "syntax error"); }
int isAlpha(int ch) { return BTWI(ch, 'A', 'Z') || BTWI(ch, 'a', 'z') || (ch == '_'); }
int isNum(int ch) { return BTWI(ch, '0', '9'); }
int isAlphaNum(int ch) { return isAlpha(ch) || isNum(ch); }
char *symName(int si) { return symbols[si].name; }
char *asmName(int si) { return symbols[si].asm_name; }
char *hAlloc(int sz) { hHere += sz; return &heap[hHere-sz]; }
int g2(int op, int a1, int a2) {
    DBG(";g<%d,%d,%d>\n", op, a1, a2);
    INST_T *x = &code[++codeSz];
    x->op = op; x->a1 = a1; x->a2 = a2;
    return codeSz;
}
int g1(int op, int a1) { return g2(op, a1, 0); }
int g(int op) { return g2(op, 0, 0); }

void next_line() {
    cur_off = 0;
    cur_lnum++;
    if (fgets(cur_line, 256, input_fp) != cur_line) {
        cur_line[0] = 0;
        is_eof = 1;
    }
    // printf("; %s", cur_line);
}

void next_ch() {
    if (is_eof) { ch = EOF; return; }
    if (cur_line[cur_off] == 0) {
        next_line();
        if (is_eof) { ch = EOF; return; }
    }
    ch = cur_line[cur_off++];
    if (ch == 9) { ch = cur_line[cur_off-1] = 32; }
}

/*---------------------------------------------------------------------------*/
/* Lexer */
void cn(int c, int t) { if (ch == c) { tok = t; next_ch(); } }

void next_token() {
    again:
    while (BTWI(ch,1,32)) { next_ch(); }
    switch (ch) {
    case EOF: tok = EOI; break;
    case '{': next_ch(); tok = TOK_LBRA;  break;
    case '}': next_ch(); tok = TOK_RBRA;  break;
    case '(': next_ch(); tok = TOK_LPAR;  break;
    case ')': next_ch(); tok = TOK_RPAR;  break;
    case '[': next_ch(); tok = TOK_LARR;  break;
    case ']': next_ch(); tok = TOK_RARR;  break;
    case '+': next_ch(); tok = TOK_PLUS; cn('+', TOK_INC);
        if (tok == TOK_PLUS) { cn('L', TOK_LOCSA); }
        break;
    case '-': next_ch(); tok = TOK_MINUS; cn('-', TOK_DEC);
        if (tok == TOK_MINUS) { cn('L', TOK_LOCSD); }
        break;
    case '*': next_ch(); tok = TOK_STAR;  break;
    case '/': next_ch(); tok = TOK_SLASH;
        if (ch == '/') { // Line comment?
            while ((ch) && (ch != 10) && (ch != EOF)) { next_ch(); }
            goto again;
        }
        break;
    case ';': next_ch(); tok = TOK_SEMI;  break;
    case '&': next_ch(); tok = TOK_AMP;   break;
    case ',': next_ch(); tok = TOK_COMMA; break;
    case '<': next_ch(); tok = TOK_LT;    break;
    case '>': next_ch(); tok = TOK_GT;    break;
    case '=': next_ch(); tok = TOK_SET; cn('=', TOK_EQ); break;
    case '"': tok = TOK_STR;
        tok_len = 0;
        next_ch();
        while (ch != '"') {
            if (ch == EOF) { syntax_error(); }
            if (ch  == '\\') { next_ch(); 
                if (ch == 'n') { ch = 10; } 
                else if (ch == 't') { ch = 9; } 
                else if (ch == 'r') { ch = 13; } 
                else if (ch == '\\') { ch = '\\'; } 
                else if (ch == '"') { ch = '"'; } 
                else { syntax_error(); }
            }
            id_name[tok_len++] = ch;
            next_ch();
        }
        id_name[tok_len] = 0;
        next_ch();
        break;
    case '\'': next_ch(); int_val = ch; next_ch();
        if (ch == '\'') { next_ch(); tok = TOK_NUM; } else { syntax_error(); }
        break;
    default:
        if (isNum(ch)) {
            int_val = 0; /* missing overflow check */
            while (isNum(ch)) { int_val = int_val  *10 + (ch - '0'); next_ch(); }
            tok = TOK_NUM;
        } else if (isAlpha(ch)) {
            tok_len = 0; /* missing overflow check */
            while (isAlphaNum(ch)) { id_name[tok_len++] = ch; next_ch(); }
            id_name[tok_len] = '\0';
            tok = 0;
            while ((words[tok] != NULL) && (strcmp(words[tok], id_name) != 0)) { tok++; }
            if (words[tok] == NULL) {
                tok = TOK_ID;
                if ((tok_len == 2) && (id_name[0] == 'L') && BTWI(id_name[1], '0', '9')) {
                    tok = TOK_LOC;
                    int_val = id_name[1] - '0';
                }
            }
        } else { syntax_error(); }
    }
}

void tokenShouldBe(int exp) {
    if (tok != exp) {
        fprintf(stderr, "\n-expected token [%d], not[%d]-", exp, tok);
        syntax_error();
    }
}

void expectToken(int exp) { tokenShouldBe(exp); next_token(); }
void expectNext(int exp) { next_token(); expectToken(exp); }
void nextShouldBe(int exp) { next_token(); tokenShouldBe(exp); }
void tokenShouldNotBe(int x) { if (tok == x) { syntax_error(); } }

/*---------------------------------------------------------------------------*/
/* Symbols:  'C', 'F', 'I', 'R', 'S', 'T' */
int findSymbol(char *name, char type) {
    for (int i = 0; i < numSymbols; i++) {
        SYM_T *x = &symbols[i];
        if (strcmp(x->name, name) == 0) {
            if (x->type == type) { return i; }
            else { return -1; }
        }
    }
    return -1;
}

int genSymbol(char *name, char type) {
    int i = findSymbol(name, type);
    if (0 <= i) { return i; }
    i = numSymbols++;
    SYM_T *x = &symbols[i];
    x->type = type;
    x->sz = 1;
    x->isReg = 0;
	x->asm_name[0] = 0;
    if (type != 'S') { strcpy(x->name, name); }
    if (type == 'R') { strcpy(x->asm_name, name); x->isReg = 1; x->type = 'I'; }
    else if (type == 'S') {
		x->strVal = hAlloc(strlen(name) + 1);
        strcpy(x->strVal, name);
        x->sz = strlen(name);
    }
	if (x->asm_name[0] == 0) { sprintf(x->asm_name, "%c%d", type, i); }
    return i;
}

int genTarget() {
    static int seq = 0;
    char buf[8];
    sprintf(buf, "T%d", ++seq);
    return genSymbol(buf, 'T');
}

char *varName(int si) {
    static char s[16];
    if (symbols[si].isReg) { return asmName(si); }
    if (symbols[si].type == 'F') { return asmName(si); }
    sprintf(s, "[%s]", asmName(si));
    return s;
}

// ---------------------------------------------------------------------------
// Code generation
enum {
    NOP, GETIMM, SETIMM, ADDROF, VALAT
    , GETLOC, SETLOC, INCLOC, DECLOC, LOCSA, LOCSD
    , GETVAR, SETVAR, SETVARC, INCVAR, DECVAR
    , SYSCALL, MOVAC
    , ADD, SUB, MUL, DIV
    , LT, GT, EQU, NEQ, CMP
    , AND, OR, XOR, NOT
    , DEFUN, TARGET, TEST, JMP, JZ, JNZ, CALL, RET
    , PUSH, POP
};

void opArg(INST_T *x, INST_T *y) {
    x->s1 = hAlloc(16);
    if (y->op == GETVAR) { sprintf(x->s1, "%s ; %s", varName(y->a1), symName(y->a1)); }
    if (y->op == GETIMM) { sprintf(x->s1, "%d", y->a1); }
    if (y->op == GETLOC) { sprintf(x->s1, "[EBP+%d]", y->a1*4); }
    x->s1[15]=0; y->op = 999; // Nothing
}

void optimizeCode() {
    for (int i = 1; i <= codeSz; i++) {
        INST_T *x = &code[i], *y = &code[i+1];
        // DBG("\n; optimize: %3d: op=%d, a1=%d", i, x->op, x->a1);
        if (x->op == ADD)      { opArg(x, y); }
        else if (x->op == SUB) { opArg(x, y); }
        else if (x->op == MUL) { opArg(x, y); }
        else if (x->op == DIV) { opArg(x, y); }
        else if (x->op == LT)  { opArg(x, y); }
        else if (x->op == GT)  { opArg(x, y); }
        else if (x->op == EQU) { opArg(x, y); }
        else if (x->op == NEQ) { opArg(x, y); }
        else if (x->op == AND) { opArg(x, y); }
        else if (x->op == OR)  { opArg(x, y); }
        else if (x->op == XOR) { opArg(x, y); }
        //else if (x->op == CMP) { opArg(x, y); }
        //else if ((x->op == GETIMM) && (y->op == SETVAR)) {
            // x->op = SETIMM; x->a2 = x->a1; x->a1 = y->a1;
            // printf(" *** GETIMM->SETIMM *** (%d,%d,%d)", x->op, x->a1, x->a2);
            // y->op = 999; // Nothing
        //}
        else if ((x->op == GETVAR) && (x->a1 == 0)) {x->op = 999; }
        // else if ((x->op == SETVAR) && (x->a1 == 0)) {x->op = 999; }
        else if ((x->op == EQU) && (y->op == x->op)) {y->op = 999; }
        else if ((x->op == NEQ) && (y->op == x->op)) {y->op = 999; }
        else if ((x->op == LT)  && (y->op == x->op)) {y->op = 999; }
        else if ((x->op == GT)  && (y->op == x->op)) {y->op = 999; }
    }
    int nn = 0;
    for (int i = 1; i <= codeSz; i++) {
        INST_T *x = &code[i];
        if (x->op == 999) {
            ++nn;
            // for (int j = i; j < codeSz; j++) { code[j] = code[j+1]; }
            // codeSz--; i--;
        }
    }
    printf("; %d instructions removed\n", nn);
}

void opCmp(char *op, char *x) {
    printf("\n\tXOR  EDI, EDI");
    printf("\n\tCMP  EAX, %s", x);
    printf("\n\t%s  @F", op);
    printf("\n\tDEC  EDI");
    printf("\n@@:\tMOV  EAX, EDI");
}

void outputCode() {
    printf("format ELF executable");
    printf("\n;================== code =====================");
    printf("\nsegment readable executable");
    printf("\nstart:\n\tLEA  EBP, [locals]");
    printf("\n\tCALL %s", asmName(findSymbol("main", 'F')));
    printf("\n;================== library ==================");
    printf("\nexit:\n\tMOV EAX, 1\n\tXOR EBX, EBX\n\tINT 0x80\n");
    printf("\n;=============================================");
/*
emit:
	MOV [reg_A], EAX	; EAX = value
	mov EAX, 4			; syscall: sys_write
	mov EBX, 0			; EBX = file handle (0=stdout)
	lea ECX, [reg_A]	; ECX = address of buffer
	mov EDX, 1			; EDX = length
	int 0x80
	RET
*/
    for (int i = 1; i <= codeSz; i++) {
        INST_T *x = &code[i];
        int op = x->op, a1 = x->a1, a2 = x->a2, t1;
        char *n1 = symName(a1), *v1 = varName(a1), *as1 = asmName(a1);
        DBG("\n; gen: %3d: op=%d, a1=%d, s1=%s", i, op, a1, v1);
        switch (op) {
            case  GETIMM:  printf("\n\tMOV  EAX, %d", a1);
            BCASE SETIMM:  printf("\n\tMOV  %s, %d ; %s", v1, a2, n1);
            BCASE ADDROF:  printf("\n\tLEA  EAX, %s; symbol %d", v1, a1);
            BCASE VALAT:   printf("\n\tMOVZX  EAX, BYTE [%s]", v1);
            BCASE GETVAR:  printf("\n\tMOV  EAX, %s ; %s", v1, n1);
            BCASE SETVAR:  printf("\n\tMOV  [%s], %s ; %s",
                               as1, a2==1?"AL":"EAX", n1);
            BCASE SETVARC: printf("\n\tMOV  [%s + ECX%s], %s ; %s",
                               as1, a2==1?"":"*4", a2==1?"AL":"EAX", n1);
            BCASE INCVAR:  printf("\n\tINC  %s ; %s", v1, n1);
            BCASE DECVAR:  printf("\n\tDEC  %s ; %s", v1, n1);
            BCASE GETLOC:  printf("\n\tMOV  EAX, [EBP+%d]", a1*4);
            BCASE SETLOC:  printf("\n\tMOV  [EBP+%d], EAX", a1*4);
            BCASE INCLOC:  printf("\n\tINC  DWORD [EBP+%d]", a1*4);
            BCASE DECLOC:  printf("\n\tDEC  DWORD [EBP+%d]", a1*4);
            BCASE SYSCALL: printf("\n\tMOV  EAX, %s", varName(findSymbol("A", 'I')));
                           printf("\n\tMOV  EBX, %s", varName(findSymbol("B", 'I')));
                           printf("\n\tMOV  ECX, %s", varName(findSymbol("C", 'I')));
                           printf("\n\tMOV  EDX, %s", varName(findSymbol("D", 'I')));
                           printf("\n\tINT  0x80");
            BCASE ADD:     printf("\n\tADD  EAX, %s", x->s1);
            BCASE MOVAC:   printf("\n\tMOV  ECX, EAX");
            BCASE SUB:     printf("\n\tSUB  EAX, %s", x->s1);
            BCASE MUL:     printf("\n\tIMUL EAX, %s", x->s1);
            BCASE DIV:     printf("\n\tCDQ\n\tIDIV %s", x->s1);
            BCASE EQU:     opCmp("JE ", x->s1);
            BCASE CMP:     printf("\n\tCMP  EAX, %d", x->a1);
            BCASE DEFUN:   printf("\n\n%s: ; %s", v1, n1);
            BCASE LOCSA:   printf("\n\tADD  EBP, 40");
            BCASE LOCSD:   printf("\n\tSUB  EBP, 40");
            BCASE CALL:    printf("\n\tCALL %s ; %s", v1, n1);
            BCASE RET:     printf("\n\tRET");
            BCASE TARGET:  printf("\n%s:", n1);
            BCASE TEST:    printf("\n\tCMP  EAX, 0");
            BCASE JMP:     printf("\n\tJMP  %s", n1);
            BCASE JZ:      printf("\n\tJZ   %s", n1);
            BCASE JNZ:     printf("\n\tJNZ  %s", n1);
            BCASE PUSH:    printf("\n\tPUSH E%cX", a1+'A');
            BCASE POP:     printf("\n\tPOP  E%cX", a1+'A');
        }
    }
    printf("\n;================== data =====================");
    printf("\nsegment readable writeable");
    printf("\n;=============================================");
    printf("\n; symbols: %d entries, %d used", SYMBOLS_SZ, numSymbols);
    printf("\n; ------------------------------------");

    // S goes first (db), then I and C because they are (rd/rb)
    for (int i = 0; i < numSymbols; i++) {
        SYM_T *x = &symbols[i];
        if (x->type == 'S') {
            printf("\n%s\t\t\tdb ", x->asm_name);
            for (int j = 0; j < x->sz; j++) { printf("%d,", x->strVal[j]); }
            printf("0");
        }
    }
    for (int i = 0; i < numSymbols; i++) {
        SYM_T *x = &symbols[i];
        if (x->type == 'I') {
            if (x->isReg) { } // printf("\n; %s\t\t\trd %-10d ; %s", x->asm_name, x->sz, x->name); }
            else { printf("\n%s\t\t\trd %-10d ; %s (%d)", x->asm_name, x->sz, x->name, i); }
        }
        else if (x->type == 'C') { printf("\n%s\t\t\trb %-10d ; %s", x->asm_name, x->sz, x->name); }
    }
    printf("\nlocals\t\trd 400\n");
    // printf("\n_sps\t\trd 26");
    // for (int i = 0; i < 26; i++) {
    //     printf("\nreg_%c\t\trd 32", 'A' + i);
    // }
}

//---------------------------------------------------------------------------
// Parser.

int lastTerm; // 1=last was loc, 2=var
int term() {
    lastTerm = 0;
    if (tok == TOK_ID)  { lastTerm=2; g1(GETVAR, genSymbol(id_name, 'I')); return 1; }
    if (tok == TOK_NUM) { g1(GETIMM, int_val); return 1; }
    if (tok == TOK_LOC) { lastTerm=1; g1(GETLOC, int_val); return 1; }
    if (tok == TOK_STR) { g1(ADDROF, genSymbol(id_name, 'S')); return 1; }
    if (tok == TOK_AMP) {
        nextShouldBe(TOK_ID);
        int i = findSymbol(id_name, 'C');
        if (i < 0) { i = findSymbol(id_name, 'I'); }
        if (i < 0) { msg(1, "variable not defined!"); }
        g1(ADDROF, i); return 1;
    }
    if (tok == TOK_STAR) {
        nextShouldBe(TOK_ID);
        int i = findSymbol(id_name, 'C');
        if (i < 0) { i = findSymbol(id_name, 'I'); }
        if (i < 0) { msg(1, "variable not defined!"); }
        g1(VALAT, i); return 1;
    }
    return 0;
}

void next_term() {
    next_token();
    if (term()) { return; }
    syntax_error();
}

int evalOp(int id) {
    again:
    DBG("; op - tok=%d\n", tok);
    if (id == TOK_PLUS) { return id; }
    else if (id == TOK_MINUS) { return id; }
    else if (id == TOK_STAR)  { return id; }
    else if (id == TOK_SLASH) { return id; }
    else if (id == TOK_LT)    { return id; }
    else if (id == TOK_GT)    { return id; }
    else if (id == TOK_EQ)    { return id; }
    else if (id == TOK_NEQ)   { return id; }
    else if (id == TOK_AND)   { return id; }
    else if (id == TOK_OR)    { return id; }
    else if (id == TOK_XOR)   { return id; }
    else if (lastTerm == 1) {
            lastTerm = 0;
            if (tok == TOK_INC) { g1(INCLOC, code[codeSz].a1); }
            else if (tok == TOK_DEC) { g1(DECLOC, code[codeSz].a1); }
            else { return 0; }
            next_token(); goto again;
        }
    else if (lastTerm == 2) {
            lastTerm = 0;
            if (tok == TOK_INC) { g1(INCVAR, code[codeSz].a1); }
            else if (tok == TOK_DEC) { g1(DECVAR, code[codeSz].a1); }
            else { return 0; }
            next_token(); goto again;
        }
    lastTerm = 0;
    return 0;
}

void expr() {
    if (term() == 0) { return; }
    DBG("; expr - tok=%d\n", tok);
    next_token();
    int op = evalOp(tok);
    DBG("; expr - op=%d\n", op);
    while (op != 0) {
        if (op == TOK_PLUS)       { g(ADD);  next_term(); }
        else if (op == TOK_MINUS) { g(SUB);  next_term(); }
        else if (op == TOK_STAR)  { g(MUL);  next_term(); }
        else if (op == TOK_SLASH) { g(DIV);  next_term(); }
        else if (op == TOK_LT)    { g(LT);   next_term(); }
        else if (op == TOK_GT)    { g(GT);   next_term(); }
        else if (op == TOK_EQ)    { g(EQU);  next_term(); }
        else if (op == TOK_NEQ)   { g(NEQ);  next_term(); }
        else if (op == TOK_AND)   { g(AND);  next_term(); }
        else if (op == TOK_OR)    { g(OR);   next_term(); }
        else if (op == TOK_XOR)   { g(XOR);  next_term(); }
        else { syntax_error(); }
        next_token();
        op = evalOp(tok);
    }
}

void ifStmt() {
    int t1 = genTarget();
    expectNext(TOK_LPAR);
    expr();
    DBG("; if - tok=%d\n", tok);
    expectToken(TOK_RPAR);
    g1(CMP, 0); g1(JNZ, t1);
    statements(TOK_END);
    expectToken(TOK_END);
    g1(TARGET, t1);
}

void whileStmt() {
    int t1 = genTarget();
    int t2 = genTarget();
    g1(TARGET, t1);
    expectNext(TOK_LPAR);
    expr();
    expectToken(TOK_RPAR);
    g1(CMP, 0); g1(JZ, t2);
    DBG("; while - tok=%d\n", tok);
    statements(TOK_END);
    expectToken(TOK_END);
    g1(JMP, t1);
    g1(TARGET, t2);
}

void idStmt() {
    int si = findSymbol(id_name, 'F');
    if (0 <= si) { // Function call
        expectNext(TOK_LPAR);
        expectToken(TOK_RPAR);
        g1(CALL, si);
        expectToken(TOK_SEMI);
        return;
    }
    si = findSymbol(id_name, 'I');
    if (si < 0) { si = findSymbol(id_name, 'C'); }
    if (si < 0) { msg(1, "variable not defined!"); }
    int sz = symbols[si].type == 'C' ? 1 : 4;
    int isArray = 0;
    next_token();
    if (tok == TOK_LARR) { // Array reference
        isArray = 1;
        next_token(); expr();
        expectToken(TOK_RARR);
        g1(PUSH, 0);    // Push the index
    }
    if (tok == TOK_SET) {
        next_token(); expr();
        if (isArray) { g1(POP, 2); g2(SETVARC, si, sz); }
        else { g2(SETVAR, si, sz); }
    }
    expectToken(TOK_SEMI);
}

void locStmt() {
    DBG("; loc - tok=%d, id_name=%s, si=%d\n", tok, id_name, si);
    int locNum = int_val;
    next_token();
    if (tok == TOK_SET) { next_token(); expr(); g1(SETLOC, locNum); }
    else if (tok == TOK_INC) { next_token(); g1(INCLOC, locNum); }
    else if (tok == TOK_DEC) { next_token(); g1(DECLOC, locNum); }
    else { syntax_error(); }
    expectToken(TOK_SEMI);
}

void statements(int endTok) {
    while (tok != endTok) {
        DBG("; stmts - tok=%d\n", tok);
        if (tok == IF_TOK)         { ifStmt(); }
        else if (tok == WHILE_TOK) { whileStmt(); }
        else if (tok == RET_TOK)   { expectNext(TOK_SEMI); g(RET); }
        else if (tok == TOK_ID)    { idStmt(); }
        else if (tok == TOK_LOC)   { locStmt(); }
        else if (tok == TOK_SYS)   { g(SYSCALL); expectNext(TOK_SEMI); }
        else if (tok == TOK_LOCSA) { g(LOCSA); expectNext(TOK_SEMI); }
        else if (tok == TOK_LOCSD) { g(LOCSD); expectNext(TOK_SEMI); }
        else { syntax_error(); }
    }
}

/*---------------------------------------------------------------------------*/
/* Top level definitions. */

void varDef(int type) {
    nextShouldBe(TOK_ID);
    int s = genSymbol(id_name, type);
    next_token();
    if (tok == TOK_SEMI) { return; }
    expectToken(TOK_LARR);
    expectToken(TOK_NUM);
    symbols[s].sz = int_val;
    expectToken(TOK_RARR);
}

void funcDef() {
    next_token();
    int s = genSymbol(id_name, 'F');
    g1(DEFUN, s);
    expectNext(TOK_LPAR);
    expectToken(TOK_RPAR);
    statements(TOK_END);
    expectToken(TOK_END);
    if (code[codeSz].op != RET) { g(RET); }
}

/*---------------------------------------------------------------------------*/
/* Main program. */
int main(int argc, char *argv[]) {
    const char *fn = (argc > 1) ? argv[1] : "test.jn";
    input_fp = stdin;
    if (fn) {
        input_fp = fopen(fn, "rt");
        if (!input_fp) { msg(1, "cannot open source file!"); }
    }

    genSymbol("EAX", 'R'); genSymbol("EBX", 'R');
	genSymbol("ECX", 'R'); genSymbol("EDX", 'R');
    next_token();

    while (tok != EOI) {
        if (tok == TOK_DEF) { funcDef(); }
        else if (tok == INT_TOK)  { varDef('I'); expectToken(TOK_SEMI); }
        else if (tok == CHAR_TOK) { varDef('C'); expectToken(TOK_SEMI); }
    }
    fclose(input_fp);
    optimizeCode();
    outputCode();
    return 0;
}
