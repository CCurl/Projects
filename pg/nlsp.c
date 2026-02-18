#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif

enum { ATOM_MAX = 1024, CONS_MAX = 1024, MAX_TOKEN = 256};
enum { EOI=-1, INT=1, DOUBLE, SYMBOL, STRING, CONS, LPAR='(', RPAR=')' };

typedef struct CONS_S { struct ATOM_S *car; struct CONS_S *cdr; } CONS_T;
typedef struct ATOM_S {
    short type; // ATOM_TYPES
    short ndx;  // location of the atom in the atom array
    union { double d; int64_t i; char *s; CONS_T *l; } val;
} ATOM_T;

ATOM_T atom[ATOM_MAX+1];
CONS_T cons[CONS_MAX+1];
char token[MAX_TOKEN], *toIn, tib[256];
short num_atoms = 0, num_cons = 0, tokenLen, isInt, isDouble;
double dblValue;
int64_t intValue;

#define btwi(a ,b, c) ((b) <= (a) && (a) <= (c))

ATOM_T *parseToken(int tok);
void print(ATOM_T *a);
void pList(CONS_T *c) { while (c) { print(c->car); printf(" "); c=c->cdr; } }
void err(const char *msg) { fprintf(stderr, "Error: %s\n", msg); exit(1); }

CONS_T *newCons(ATOM_T *atom, CONS_T *cdr) {
    if (num_cons >= CONS_MAX) { err("Out of lists!"); }
    cons[num_cons] = (CONS_T){ atom, cdr };
    return &cons[num_cons++];
}

ATOM_T *newAtom(int type, double num, char *str) {
    if (num_atoms >= ATOM_MAX) { err("Out of atoms!"); }
    ATOM_T *ret = &atom[num_atoms];
    ret->ndx = num_atoms++;
    ret->type = type;
    switch (type) {
        case INT:    ret->val.i  = (int64_t)num; break;
        case DOUBLE: ret->val.d  = num; break;
        case STRING: ret->val.s  = str ? strdup(str) : NULL; break;
        case SYMBOL: ret->val.s  = str ? strdup(str) : NULL; break;
        case CONS:   ret->val.l = NULL; break; //  newCons(NULL, NULL); break;
    }
    return ret;
}

int doNum(char w) {
    isInt = isDouble = tokenLen = 0;
    token[tokenLen++] = w;
    while (btwi(*toIn, '0', '9')) { token[tokenLen++] = *toIn++; }
    if (*toIn == '.') {
        token[tokenLen++] = *toIn++;
        while (btwi(*toIn, '0', '9')) { token[tokenLen++] = *toIn++; }
        token[tokenLen] = 0;
        isDouble = 1; dblValue = strtod(token, NULL);
        return DOUBLE;
    }
    token[tokenLen] = 0;
    isInt = 1;
    intValue = strtoll(token, NULL, 10);
    return INT;
}

int nextToken() {
    tokenLen = 0;
again: switch (*toIn) {
        case 0: return EOI;
        case ' ': case '\t': case '\n': ++toIn; goto again;
        case '(': case ')': return *toIn++;
        case '"': ++toIn; // Skip the opening quote
        while (*toIn && (*toIn != '"')) { token[tokenLen++] = *toIn++; }
        if (*toIn == '"') { ++toIn; }
        token[tokenLen] = 0;
        return STRING;
    }
    if (btwi(*toIn, '0', '9') || *toIn == '.') { return doNum(*toIn++); }
    while (*toIn > 32 && *toIn != '(' && *toIn != ')') { token[tokenLen++] = *toIn++; }
    token[tokenLen] = 0;
    return SYMBOL;
}

ATOM_T *parseList(ATOM_T *listAtom) {
    CONS_T *current = NULL;
    while (1) {
        int tok = nextToken();
        if (tok == RPAR) { return listAtom; }
        if (tok == EOI) { err("Unexpected end of input!"); }
        CONS_T *next = newCons(parseToken(tok), NULL);
		if (listAtom->val.l == NULL) { listAtom->val.l = next; }
        else { current->cdr = next; }
        current = next;
    }
    return listAtom;
}

ATOM_T *parseToken(int tok) {
    if (tok == 0) { tok = nextToken(); }
    switch (tok) {
        case EOI: return NULL;
        case LPAR: return parseList(newAtom(CONS, 0, NULL));
        case INT: return newAtom(INT, (double)intValue, NULL);
        case DOUBLE: return newAtom(DOUBLE, dblValue, NULL);
        case STRING: return newAtom(STRING, 0, token);
        case SYMBOL: return newAtom(SYMBOL, 0, token);
    }
    printf("Unknown token type: %d\n", tok);
    return NULL;
}

void print(ATOM_T *a) {
    CONS_T *cons;
    if (a == NULL) { printf("nil"); return; }
    switch (a->type) {
		case CONS: printf("( "); pList(a->val.l); printf(")"); break;
        case INT: printf("%" PRId64, a->val.i); break;
        case DOUBLE: printf("%f", a->val.d); break;
        case STRING: printf("\"%s\"", a->val.s); break;
		case SYMBOL: printf("%s", a->val.s); break;
    }
}

void test(char *src) {
    printf("Testing: %s\n", src);
    toIn = src;
	print(parseToken(0)); printf("\n");
}

void tests() {
    test("999888777666");
    test("123456789012345.6789");
    test("()");
    test("(+ 1 2)");
    test("(+ 1 (+ 2 3))");
    test("(+ \"hi ()\" (+ 2 (* 3 4)))");
}

int main(int argc, char **argv) {
    printf("%d\n", (int)sizeof(ATOM_T)); 
    tests();
    ATOM_T *expr;
    while ((expr = parseToken(0)) != NULL) { 
        //ATOM_T *result = eval(expr); 
        //print_expr(result); 
        //printf("\n"); 
    }
    printf("%d atoms used, %d cons used\n", num_atoms, num_cons); 
    return 0;
}
