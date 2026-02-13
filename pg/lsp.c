// A minimal LISP experiment (c) 2025 Chris Curl

https://leinonen.ninja/posts/building-lisp-from-the-ground-up

#include <stdio.h>
#include <string.h>

#define btwi(a ,b, c) ((a) >= (b) && (a) <= (c))
#define strEq(a, b) (strcmp((a), (b)) == 0)

enum { TYPE_NUMBER, TYPE_STRING, TYPE_LIST, TYPE_NIL } ATOM_TYPES;
enum {
    TOK_LPAR, TOK_RPAR, TOK_NUMBER, TOK_STRING
    , TOK_LAMBDA, TOK_DEFINE, TOK_CAR, TOK_CDR
    , TOK_EOI, TOK_UNKNOWN
};

typedef struct {
    int token;
    char *name;
} tokens;

typedef unsigned char BYTE;

typedef struct ATOM_S {
    BYTE type;       // ATOM_TYPES
    union val {
        double num;  // TYPE_NUMBER
        char *sym;   // TYPE_SYMBOL
        char *str;   // TYPE_STRING
    };
	struct ATOM_S *car; // TYPE_LIST
    struct ATOM_S *cdr;
} ATOM;

char *toIn, wd[256];

int doQuote() {
    int ln = 0;
    wd[ln++] = '"';
    while (*toIn) {
        char ch = (*toIn++);
        if (ch == '"') { break; }
        wd[ln++] = ch;
    }
    wd[ln] = 0;
    return ln;
}

double dblVal;
int isNum(const char* w) {
    dblVal = 0;
    int isNeg = 0;
    if (w[0] == '-') { isNeg = 1; ++w; }
    if (w[0] == 0) { return 0; }
    while (*w && (*w != '.')) {
        char c = *(w++);
        if (btwi(c, '0', '9')) { dblVal = (dblVal * 10) + (c - '0'); }
        else return 0;
    }
    if (*w == '.') {
        ++w;
        double frac = 1;
        while (*w) {
            char c = *(w++);
            if (btwi(c, '0', '9')) { frac /= 10; dblVal += (c - '0') * frac; }
            else return 0;
        }
    }
    if (isNeg) { dblVal = -dblVal; }
    return 1;
}

int nextWord() {
    int ln = 0;
    while (*toIn && (*toIn < 33)) { ++toIn; }

    if (*toIn == '(') { wd[0] = *(toIn++); wd[1] = 0; return 1; }
    if (*toIn == ')') { wd[0] = *(toIn++); wd[1] = 0; return 1; }
    if (*toIn == '"') { ++toIn; return doQuote(); }
    while (*toIn > 32) {
        char ch = (*toIn++);
        if (ch == '(') { --toIn; break; }
        if (ch == ')') { --toIn; break; }
        wd[ln++] = ch;
    }
    wd[ln] = 0;
    return ln;
}

int nextToken() {
    if (nextWord() == 0) { return TOK_EOI; }
    if (wd[0] == '"') { return TOK_STRING; }
    if (strEq(wd, "(")) { return TOK_LPAR; }
    if (strEq(wd, ")")) { return TOK_RPAR; }
    if (strEq(wd, "lambda")) { return TOK_LAMBDA; }
    if (strEq(wd, "define")) { return TOK_DEFINE; }
    if (strEq(wd, "car")) { return TOK_CAR; }
    if (strEq(wd, "cdr")) { return TOK_CDR; }
    if (isNum(wd)) { return TOK_NUMBER; }
    return TOK_UNKNOWN;
}

// SYMBOLS
#define MAX_ATOM 999
ATOM atoms[MAX_ATOM + 1];
int num_atoms = 0;
ATOM *newAtom(BYTE type, double num, char *str) {
    ATOM *a = &atoms[num_atoms];
	a->type = type;
	a->car = a->cdr = NULL;
    if (type == TYPE_NUMBER) a->num = num;
    if (type == TYPE_STRING) a->str = _strdup(str);
    return a;
}

// REPL

char tib[256];
void read() {
    printf("\nlisp: ");
    fgets(tib, sizeof(tib), stdin);
    toIn = tib;
}

int lvl = 0, err = 0;
ATOM *eval(int tok) {
    ATOM *atom = NULL;
    if (tok == TOK_EOI) { return NULL; }
    // printf("--token: %d--\n", tok);
    switch (tok) {
        case TOK_LPAR: { // List
            atom = newAtom(TYPE_LIST, 0, NULL);
            int tok2 = nextToken();
            ATOM *entry = NULL;
            while (tok2 != ')') {
                if (tok2 == TOK_EOI) { err = 1; return NULL; }
                if (atom->car == NULL) {
                    atom->car = eval(tok2);
                    entry = atom->car;
                } else {
                    entry->cdr = eval(tok2);
                    entry = entry->cdr;
                }
                tok2 = nextToken();
            }
            return atom;
        }
        break;
        case TOK_STRING:
			atom = newAtom(TYPE_STRING, 0, wd);
            break;
        case TOK_LAMBDA: printf("-lambda-\n"); break;
        case TOK_DEFINE: printf("-def-\n"); break;
        case TOK_CAR: printf("-car-\n"); break;
        case TOK_CDR: printf("-cdr-\n"); break;
        case TOK_NUMBER: 
			atom = newAtom(TYPE_NUMBER, dblVal, NULL);
            break;
        default:
            printf("Unknown token: %s\n", wd);
            err = 1;
    }
    return atom;
}

void print() {
    if (err) {
        printf("Evaluation completed with errors.\n");
        err = 0;
    }
    else {
        printf("Evaluation completed successfully.\n");
    }
}

void init(char *s) {
    printf("init: %s\n", s);
    toIn = s;
    eval(nextToken());
}

int main() {
    init("123451234512345.6789 ( \"hello world\" (2 3 (foo bar)))");
    init("(lambda (L) (car L))");
    init("(define (func x) (cdr x))");
    while (1) {
        read();
        eval(nextToken());
        print();
    }
    return 0;
}

