// Tokenizer - This syntax is very much like Lua

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#define PeekCh        (*tok_input)
#define PeekCh1       (*(tok_input+1))
#define SkipCh        tok_input++
#define NextCh        (tok_nextch())
#define BTW(x, l, h)  ((l<=x) && (x<=h))
#define Skip(n)       tok_input += n

enum { TOK_ID = 1, TOK_OP, TOK_KW, TOK_VAL,
    VAL_NUM, VAL_STR, VAL_NIL, VAL_T, VAL_F,
    OP_EQUALS, OP_LT, OP_GT, OP_LE, OP_GE,
    OP_NEQ, OP_LAND, OP_LOR, OP_COM,
    OP_ASSIGN, OP_INC, OP_DEC, OP_QM, OP_AMP,
    OP_ADD, OP_SUB, OP_MULT, OP_DIV,
    OP_AND, OP_OR, OP_XOR, OP_POW,
    OP_DOT, OP_COMMA, OP_MOD, OP_PND,
    LPAR, RPAR, LBRACKET, RBRACKET, LBRACE, RBRACE,
    KW_FUN, KW_IF, KW_ELSE, KW_ELSEIF, KW_THEN, KW_END,
    KW_DO, KW_WHILE, KW_UNTIL, KW_FOR, KW_NEXT, KW_RET,
    OP_COLON, OP_SEMI
};

#define maxTokens 10000

typedef struct {
    int type;
    char cval[32];
    char *string;
    double num;
    int id;
} TOK_T;

const char *tok_input;
char tokMsg[128];
int tok_err = 0, numTokens = 0;
TOK_T tokens[maxTokens+1];

void tokErr(const char *msg) {
    strcpy(tokMsg, msg);
    printf("ERROR: %s", msg);
    tok_err = 1;
}

const char tok_nextch() {
    return (PeekCh) ? *(SkipCh) : 0;
}

int oneOf(char ch, const char *chars) {
    for (int i=0; chars[i]; i++) {
        if (ch == chars[i]) { return 1; }
    }
    return 0;
}

int newToken(int type, char *name) {
    if (numTokens < maxTokens) {
        tokens[++numTokens].type = type;
        if (name) { strcpy(tokens[numTokens].cval, name); }
        return numTokens;
    }
    tokErr("out of tokens!");
    return 0;
}

int newKw(int id, char *name) {
    if (newToken(TOK_KW, name)) {
        tokens[numTokens].id = id;
    }
    return numTokens;
}

int newVal(int id, double num, char *str) {
    char *ty = "??";
    switch(id) {
        case VAL_NUM: ty = "num";    break;
        case VAL_T:   ty = "true";   break;
        case VAL_F:   ty = "false";  break;
        case VAL_STR: ty = "string"; break;
        case VAL_NIL: ty = "nil";    break;
        default: break;
    }
    if (newToken(TOK_VAL, ty)) {
        tokens[numTokens].id = id;
        tokens[numTokens].num = num;
        tokens[numTokens].string = str;
        return numTokens;
    }
    return 0;
}

int newOp(int id, char *name) {
    if (newToken(TOK_OP, name)) {
        tokens[numTokens].id = id;
        return numTokens;
    }
    return 0;
}

int strEq(const char *str1, const char *str2) {
    return strcmp(str1, str2) == 0;
}

int kwCheck(char *id) {
    // printf("-kw:[%s]-", id);
    if (strEq(id, "function")) { return newKw(KW_FUN,    id); }
    if (strEq(id, "return")  ) { return newKw(KW_RET,    id); }
    if (strEq(id, "if")      ) { return newKw(KW_IF,     id); }
    if (strEq(id, "then")    ) { return newKw(KW_THEN,   id); }
    if (strEq(id, "elseif")  ) { return newKw(KW_ELSEIF, id); }
    if (strEq(id, "else")    ) { return newKw(KW_ELSE,   id); }
    if (strEq(id, "end")     ) { return newKw(KW_END,    id); }
    if (strEq(id, "for")     ) { return newKw(KW_FOR,    id); }
    if (strEq(id, "next")    ) { return newKw(KW_NEXT,   id); }
    if (strEq(id, "do")      ) { return newKw(KW_DO,     id); }
    if (strEq(id, "while")   ) { return newKw(KW_WHILE,  id); }
    if (strEq(id, "until")   ) { return newKw(KW_UNTIL,  id); }
    if (strEq(id, "and")     ) { return newOp(OP_LAND,   id); }
    if (strEq(id, "or")      ) { return newOp(OP_LOR,    id); }
    if (strEq(id, "true")    ) { return newVal(VAL_T, 0, 0); }
    if (strEq(id, "false")   ) { return newVal(VAL_F, 0, 0); }
    if (strEq(id, "nil")     ) { return newVal(VAL_NIL, 0, 0); }
    return newToken(TOK_ID, id);
}

int toId() {
    static char id[32];
    int len = 0;
    id[len++] = NextCh;
    while (PeekCh) {
        if (BTW(PeekCh, 'A', 'Z') || BTW(PeekCh, 'a', 'z') || BTW(PeekCh, '0', '9') || (PeekCh == '_')) {
            id[len++] = NextCh;
        } else {
            break;
        }
    }
    id[len] = 0;
    return kwCheck(id);
}

int toNum() {
    double x = 0;
    double sign = 1;
    if (PeekCh == '-') { SkipCh; sign = -1; }
    while (BTW(PeekCh, '0', '9')) {
        x = (x*10) + ((double)NextCh-'0');
    }
    if (PeekCh == '.') {
        double y = 10;
        SkipCh;
        while (BTW(PeekCh, '0', '9')) {
            x += ((double)NextCh-'0')/y;
            y *= 10;
        }
    }
    return newVal(VAL_NUM, x*sign, 0);
}

int toStr() {
    static char chs[1024];
    size_t len = 0;
    char delim = NextCh;
    while (PeekCh && (PeekCh != delim)) { chs[len++] = NextCh; }
    if (PeekCh) { NextCh; }
    chs[len] = 0;
    if (newVal(VAL_STR, 0, malloc(len+1))) {
        strcpy(tokens[numTokens].string, chs);
        return numTokens;
    }
    return 0;
}

int isNum() {
    if (BTW(PeekCh, '0', '9')) { return toNum(); }
    if ((PeekCh == '-') && BTW(PeekCh1, '0', '9')) { return toNum(); }
    return 0;
}

int isId() {
    if (BTW(PeekCh, 'A', 'Z')) { return toId(); }
    if (BTW(PeekCh, 'a', 'z')) { return toId(); }
    if (PeekCh == '_') { return toId(); }
    return 0;
}

int lineComment() {
    while (PeekCh) { NextCh; }
    return 1;
}

int isOp() {
    if ((PeekCh == '=') && (PeekCh1 == '=')) { Skip(2); return newOp(OP_EQUALS, "=="); }
    if ((PeekCh == '~') && (PeekCh1 == '=')) { Skip(2); return newOp(OP_NEQ, "~="); }
    if ((PeekCh == '<') && (PeekCh1 == '=')) { Skip(2); return newOp(OP_LE, "<="); }
    if ((PeekCh == '>') && (PeekCh1 == '=')) { Skip(2); return newOp(OP_GE, ">="); }
    if ((PeekCh == '+') && (PeekCh1 == '+')) { Skip(2); return newOp(OP_INC, "++"); }
    if ((PeekCh == '-') && (PeekCh1 == '-')) { Skip(2); return newOp(OP_DEC, "--"); }
    if ((PeekCh == '/') && (PeekCh1 == '/')) { return lineComment(); }

    if (PeekCh == '=') { SkipCh; return newOp(OP_ASSIGN, "="); }
    if (PeekCh == '<') { SkipCh; return newOp(OP_LT, "<"); }
    if (PeekCh == '>') { SkipCh; return newOp(OP_GT, ">"); }
    
    if (PeekCh == '+') { SkipCh; return newOp(OP_ADD,  "+"); }
    if (PeekCh == '-') { SkipCh; return newOp(OP_SUB,  "-"); }
    if (PeekCh == '*') { SkipCh; return newOp(OP_MULT, "*"); }
    if (PeekCh == '/') { SkipCh; return newOp(OP_DIV,  "/"); }
    if (PeekCh == '%') { SkipCh; return newOp(OP_MOD,  "%"); }
    if (PeekCh == '^') { SkipCh; return newOp(OP_POW,  "^"); }

    if (PeekCh == '.') { SkipCh; return newOp(OP_DOT, "."); }
    if (PeekCh == ',') { SkipCh; return newOp(OP_COMMA, ","); }

    if (PeekCh == '(') { SkipCh; return newOp(LPAR, "("); }
    if (PeekCh == ')') { SkipCh; return newOp(RPAR, ")"); }
    if (PeekCh == '[') { SkipCh; return newOp(LBRACKET, "["); }
    if (PeekCh == ']') { SkipCh; return newOp(RBRACKET, "]"); }
    if (PeekCh == '{') { SkipCh; return newOp(LBRACE, "{"); }
    if (PeekCh == '}') { SkipCh; return newOp(RBRACE, "}"); }

    if (PeekCh == '&') { SkipCh; return newOp(OP_AND, "&"); }
    if (PeekCh == '|') { SkipCh; return newOp(OP_OR,  "|"); }
    if (PeekCh == '#') { SkipCh; return newOp(OP_XOR, "#"); }
    if (PeekCh == '~') { SkipCh; return newOp(OP_COM, "~"); }

    if (PeekCh == ':') { SkipCh; return newOp(OP_COLON, ":"); }
    if (PeekCh == ';') { SkipCh; return newOp(OP_SEMI, ";"); }
    return 0;
}

int isStr() {
    if ((PeekCh=='"') || (PeekCh == '\'')) { return toStr(); }
    return 0;
}

int tok_parse(const char *line) {
    printf("%s", line);
    tok_input = line;
    char ws[] = { 32, 9, 10, 13, 0 };
    while (PeekCh && (!tok_err)) {
        while (oneOf(PeekCh, ws)) { SkipCh; }
        if (!PeekCh) { continue; }
        if (isId()) { continue; }
        if (isNum()) { continue; }
        if (isOp()) { continue; }
        if (isStr()) { continue; }
        tokErr("ERROR: unknown token.");
        return 1;
    }
    return 0;
}

char *tokToName(int tok) {
    if (tok == TOK_ID) { return "ID"; }
    if (tok == TOK_OP) { return "OP"; }
    if (tok == TOK_KW) { return "KW"; }
    if (tok == TOK_VAL) { return "VAL"; }
    return "KW";
}

void tokDump(int n) {
    TOK_T *p = &tokens[n];
    printf("%4d: type %-3d %-3s  cval: %-16s  num: %-8g  id: %-4d  str: %s\n",
        n, p->type, tokToName(p->type), p->cval, p->num, p->id, p->string ? p->string : "");

}

void tokDumpAll() {
    printf("%d tokens\n", numTokens);
    for (int i=1; i <= numTokens; i++) { tokDump(i); }
}
