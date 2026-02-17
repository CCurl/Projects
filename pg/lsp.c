// A minimal LISP experiment (c) 2025 Chris Curl
// https://leinonen.ninja/posts/building-lisp-from-the-ground-up

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LIST 999
#define MAX_ATOM 999
#define btwi(a ,b, c) ((a) >= (b) && (a) <= (c))
#define strEq(a, b) (strcmp((a), (b)) == 0)

enum { ATOM_INT, ATOM_DOUBLE, ATOM_STRING, ATOM_LIST, ATOM_NIL } ATOM_TYPES;

typedef struct LIST_S {
    struct ATOM_S *car;
    struct LIST_S *cdr;
} LIST;

typedef struct ATOM_S {
    char type;          // ATOM_TYPES
    union {
        long intVal;    // ATOM_INT
        double dblVal;  // ATOM_DOUBLE
        char *sym;      // ATOM_SYMBOL
        char *str;      // ATOM_STRING
        LIST *lst;      // ATOM_LIST
    } val;
} ATOM;

char *toIn, token[256], tib[256];
int tokenLen, isInt, isDouble, err = 0;
ATOM atoms[MAX_ATOM + 1];
LIST lists[MAX_LIST + 1];
int num_atoms = 0, num_lists = 0;

double dblVal;
long intVal;

void checkNumType(const char* w) {
    char *lastChar = NULL;
    dblVal = strtod(w, &lastChar);
    isDouble = (*lastChar == 0);
    intVal = strtol(w, &lastChar, 10);
    isInt = (*lastChar == 0);
}

void prep(char *s) {
    char *d = tib, inQuote = 0;
    while (*s) {
        if (*s == '"') { inQuote = !inQuote; }
        if (inQuote) { *d++ = *s++ }
        if (!inQuote && ((*s == '(') || (*s == ')'))) {
            *d++ = ' '; *d++ = *s++; *d++ = ' ';
        } else if (*s == '"') {
            *d++ = *s++;
            while (*s && *s != '"') { *d++ = *s++; }
            if (*s) { *d++ = *s++; }
        } else {
            *d++ = *s++;
        }
    }
    *d = 0;
    toIn = tib;
}

int nextToken() {
    again:
    tokenLen = 0;
    isInt = isDouble = 0;
    while (*toIn && (*toIn < 33)) { ++toIn; }
    while (*toIn > 32) {
        if (*toIn == ')') { break; }
        token[tokenLen++] = *(toIn++);
    }
    if ((*toIn == ')') && (tokenLen == 0)) { goto again; }
    token[tokenLen] = 0;
    checkNumType(token);
    return tokenLen;
}

void print(ATOM *atom) {
    if (atom == NULL) { printf("nil\n"); return; }
    switch (atom->type) {
        case ATOM_INT:
            printf("%ld", (long)atom->val.intVal);
            break;
        case ATOM_DOUBLE:
            printf("%f", atom->val.dblVal);
            break;
        case ATOM_STRING:
            printf("%s", atom->val.str);
            break;
        case ATOM_LIST:
            printf("( ");
            LIST *entry = atom->val.lst;
            while (entry != NULL) {
                print(entry->car);
                printf(" ");
                entry = entry->cdr;
            }
            printf(")");
            break;
        default:
            printf("Print: unknown ATOM type: %d\n", atom->type);
    }
}

// Parser
LIST *newListEntry() {
    if (num_lists > MAX_LIST) { printf("Out of lists!"); err=999; return NULL; }
    LIST *e = &lists[num_lists++];
	e->car = NULL;
	e->cdr = NULL;
    return e;
}

ATOM *newAtom(char type, double num, char *str) {
    if (num_atoms > MAX_ATOM) { printf("Out of atoms!"); err=999; return NULL; }
    ATOM *a = &atoms[num_atoms++];
	a->type = type;
    if (type == ATOM_INT) a->val.intVal = (long)num;
    else if (type == ATOM_DOUBLE) a->val.dblVal = num;
    else if (type == ATOM_STRING) a->val.str = strdup(str);
    else if (type == ATOM_LIST) a->val.lst = newListEntry();
    return a;
}

ATOM *parse();
ATOM *parselist() {
    ATOM *atom = newAtom(ATOM_LIST, 0, NULL);
    LIST *entry = atom->val.lst;
    nextToken();
    while (!strEq(token, ")") && !err) {
        if (tokenLen == 0) { printf("-eoi-"); err = 1; return NULL; }
        ATOM *a = parse();
        if (entry->car == NULL) { entry->car = a; }
        else {
            LIST *newEntry = newListEntry();
            newEntry->car = a;
            entry->cdr = newEntry;
            entry = newEntry;
        }
        nextToken();
    }
    return atom;
}

ATOM *parse() {
    if  (err) { return NULL; }
    if (tokenLen == 0) { return NULL; }
    if (isInt) { return newAtom(ATOM_INT, (double)intVal, NULL); }
    if (isDouble) { return newAtom(ATOM_DOUBLE, dblVal, NULL); }
    if (strEq(token, "(")) { return parselist(); }
    return newAtom(ATOM_STRING, 0, token);
}

ATOM *eval(ATOM *atom) {
    // For simplicity, we just return the atom itself for now
    if (atom == NULL) { return NULL; }
    switch (atom->type) {
        case ATOM_INT: return atom;
        case ATOM_DOUBLE: return atom;
        case ATOM_STRING: return atom;
        case ATOM_LIST: return atom;
        default:
            printf("Eval: unknown ATOM type: %d\n", atom->type);
            return NULL;
    }
}

void read() {
    printf("\nlisp: ");
    fgets(tib, sizeof(tib), stdin);
    toIn = tib;
    err = 0;
}

void test(char *s) {
    printf("\ninit: %s\n", s);
    toIn = s;
    nextToken();
    ATOM *result = eval(parse());
    print(result);
}

void tests() {
    test("123456789012345.6789");
    test("'(1 2 3 4 5)");
    test("(+ 1 2 3 4 5)");
    test("(+ 1 (+ 2 3) 4 5)");
}

int main() {
    while (err != 999) {
        read();
        nextToken();
        ATOM *result = eval(parse());
        print(result);
    }
    return 0;
}