#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ATOM_MAX 32
#define CONS_MAX 32
#define MAX_TOKEN 64
#define btwi(a ,b, c) ((a) >= (b) && (a) <= (c))
#define strEq(a, b) (strcmp((a), (b)) == 0)

typedef struct CONS_S {
    struct ATOM_S *car;
    struct CONS_S *cdr;
} CONS_T;

typedef struct ATOM_S {
    short type; // ATOM_TYPES
    short ndx;  // location of the atom in the atom array
    union { double d; int64_t i; char *s; CONS_T *l; } val;
} ATOM_T;

// Types of atoms and tokens
enum { EOI=-1, INT=1, DOUBLE, SYMBOL, STRING, CONS, LPAR='(', RPAR=')' };

ATOM_T atom[ATOM_MAX+1];
CONS_T cons[CONS_MAX+1];
short num_atoms = 0, num_cons = 0;
char token[MAX_TOKEN], *toIn, tib[256];
int tokenLen, isInt, isDouble;

#define car(L) ((L)->car)
#define cdr(L) ((L)->cdr)
#define dblVal(a) ((a)->val.d)
#define intVal(a) ((a)->val.i)
#define strVal(a) ((a)->val.s)
#define consVal(a) ((a)->val.l)

void err(const char *msg) { fprintf(stderr, "Error: %s\n", msg); exit(1); }

CONS_T *newCons() {
    if (num_cons >= CONS_MAX) { err("Out of lists!"); }
    cons[num_cons] = (CONS_T){ NULL, NULL };
    return &cons[num_cons++];
}

ATOM_T *newAtom(int type, double num, char *str) {
    printf("-newAtom: type %d-", type);
    if (num_atoms >= ATOM_MAX) { err("Out of atoms!"); }
    ATOM_T * ret = &atom[num_atoms++];
    ret->ndx = num_atoms - 1;
    ret->type = type;
    switch (type) {
        case INT:    intVal(ret)  = (int64_t)num; break;
        case DOUBLE: dblVal(ret)  = num; break;
        case STRING: strVal(ret)  = str ? strdup(str) : NULL; break;
        case CONS:   consVal(ret) = NULL; break;
    }
    return ret;
}

double dblValue;
long intValue;

int doNum(char w) {
    tokenLen = 0;
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
    intValue = strtol(token, NULL, 10);
    return INT;
}

int nextToken() {
again:
    switch (*toIn) {
        case 0: return EOI;
        case ' ': case '\t': case '\n': ++toIn; goto again;
        case '(': case ')': return *toIn++;
    }
    if (btwi(*toIn, '0', '9') || *toIn == '.') { return doNum(*toIn++); }
    tokenLen = 0;
    while (*toIn > 32 && *toIn != '(' && *toIn != ')') { token[tokenLen++] = *toIn++; }
    token[tokenLen] = 0;
    return STRING;
}

ATOM_T *parseToken(int tok);
ATOM_T *parseList() {
    nextToken(); // consume '('
    ATOM_T *listAtom = newAtom(CONS, 0, NULL);
    CONS_T *current = NULL;
    int tok = nextToken();
    while ((tok != RPAR)) {
        if (tok == EOI) { err("Unexpected end of input!"); }
        ATOM_T *elem = parseToken(tok);
        if (current == NULL) {
            consVal(listAtom)->car = elem;
            current = consVal(listAtom);
        } else {
            current->cdr = newCons();
            current = current->cdr;
            current->car = elem;
        }
        tok = nextToken();
    }
    return listAtom;
}

ATOM_T *parseToken(int tok) {
    if (tok == 0) { tok = nextToken(); }
    switch (tok) {
        case EOI: return NULL;
        case LPAR: return parseList();
        case INT: return newAtom(INT, (double)intValue, NULL);
        case DOUBLE: return newAtom(DOUBLE, dblValue, NULL);
        case STRING: return newAtom(STRING, 0, token);
    }
    printf("Unknown token type: %d\n", tok);
    return NULL;
}

void test(char *src) {
    printf("Testing: %s\n", src);
    toIn = src;
    parseToken(0);
    // TODO: implement the logic to parse and evaluate the source code
    // For now, we just print the source
}

void tests() {
    ATOM_T *i = newAtom(INT, 42, NULL);
    printf("INT: %ld\n", intVal(i));
    ATOM_T *d = newAtom(DOUBLE, 3.14, NULL);
    printf("DOUBLE: %f\n", dblVal(d));
    ATOM_T *s = newAtom(STRING, 0, "hello");
    printf("STRING: %s\n", strVal(s));
    test("123456789012345.6789");
    // test("(+ 1 2)");
    // test("(+ 1 (+ 2 3))");
    // test("(+ \"hi ()\" (+ 2 3))");
}

int main(int argc, char **argv) {
    // printf("%d\n", (int)sizeof(ATOM_ENTRY)); 
    tests();
    ATOM_T *expr;
    while ((expr = parseToken(0)) != NULL) { 
        //ATOM_T *result = eval(expr); 
        //print_expr(result); 
        //printf("\n"); 
    }
    return 0;
}
