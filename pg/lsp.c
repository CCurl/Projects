// A minimal LISP experiment (c) 2025 Chris Curl
// https://leinonen.ninja/posts/building-lisp-from-the-ground-up

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
    #define _CRT_SECURE_NO_WARNINGS
    #define strDup(a) _strdup((a))
#else
    #define strDup(a) strdup((a))
#endif

#define MAX_LIST 999
#define MAX_ATOM 999
#define MAX_SYMBOLS 100
#define btwi(a ,b, c) ((a) >= (b) && (a) <= (c))
#define strEq(a, b) (strcmp((a), (b)) == 0)
#define BCASE break; case
#define isList(a) (((a) != NULL) && ((a)->type == ATOM_LIST))
#define isInteger(a) (((a) != NULL) && ((a)->type == ATOM_INT))
#define isDouble(a) (((a) != NULL) && ((a)->type == ATOM_DOUBLE))
#define isString(a) (((a) != NULL) && ((a)->type == ATOM_STRING))
#define isSymbol(a) (((a) != NULL) && ((a)->type == ATOM_SYMBOL))
#define isNil(a) (((a) == NULL) || ((a)->type == ATOM_NIL))

enum { ATOM_INT, ATOM_DOUBLE, ATOM_STRING, ATOM_SYMBOL, ATOM_LIST, ATOM_NIL } ATOM_TYPES;
enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } OPS;

typedef struct LIST_S {
    int loc;            // For GC: index in lists array
    struct ATOM_S *car;
    struct LIST_S *cdr;
} LIST;

typedef struct ATOM_S {
    char type;          // ATOM_TYPES
    int loc;            // For GC: index in atoms array
    union {
        long intVal;    // ATOM_INT
        double dblVal;  // ATOM_DOUBLE
        char *sym;      // ATOM_SYMBOL
        char *str;      // ATOM_STRING
        LIST *lst;      // ATOM_LIST
    } val;
} ATOM;

typedef struct SYMBOL_TABLE_ENTRY {
    char *name;
    ATOM value;  // Store value directly instead of pointer
} SYMBOL_TABLE_ENTRY;

ATOM *eval(ATOM *atom, LIST *cdr);

ATOM atoms[MAX_ATOM + 1];
LIST lists[MAX_LIST + 1];
SYMBOL_TABLE_ENTRY symbolTable[MAX_SYMBOLS];
int num_atoms = 0, num_lists = 0, num_symbols = 0;
char *toIn, token[256], tib[256], newTib[256];
int tokenLen, isInt, isDbl, err = 0;
double dblVal;
long intVal;

void checkNumType(const char* w) {
    char *lastChar = NULL;
    dblVal = strtod(w, &lastChar);
    isDbl = (*lastChar == 0);
    intVal = strtol(w, &lastChar, 10);
    isInt = (*lastChar == 0);
}

// Symbol table functions
ATOM *lookupSymbol(const char *name) {
    for (int i = 0; i < num_symbols; i++) {
        if (strEq(symbolTable[i].name, name)) {
            return &symbolTable[i].value;  // Return pointer to stored value
        }
    }
    return NULL;
}

void defineSymbol(const char *name, ATOM *value) {
    // Check if symbol already exists
    for (int i = 0; i < num_symbols; i++) {
        if (strEq(symbolTable[i].name, name)) {
            symbolTable[i].value = *value;  // Copy value directly
            return;
        }
    }
    
    // Add new symbol
    if (num_symbols < MAX_SYMBOLS) {
        symbolTable[num_symbols].name = strDup(name);
        symbolTable[num_symbols].value = *value;  // Copy value directly
        num_symbols++;
    } else {
        printf("Symbol table full!");
        err = 1;
    }
}

int setSymbol(const char *name, ATOM *value) {
    for (int i = 0; i < num_symbols; i++) {
        if (strEq(symbolTable[i].name, name)) {
            symbolTable[i].value = *value;  // Copy value directly
            return 1;
        }
    }
    return 0; // Symbol not found
}

char *prep(char *s) {
    char *d = newTib, c, bl = 32;
    int inQuote = 0;
    while (*s) {
        c = *(s++);
        if (c == '"') {
            inQuote = !inQuote;
            *d++ = c;
        } else if (!inQuote && ((c == '(') || (c == ')'))) {
            *d++ = bl; *d++ = c; *d++ = bl;
        } else { 
            *d++ = c > bl ? c : bl; 
        }
    }
    *d = 0;
    return newTib;
}

int nextToken() {
    tokenLen = 0;
    while (*toIn && (*toIn < 33)) { ++toIn; }
    
    if (*toIn == '"') { // Handle quoted strings
        token[tokenLen++] = *(toIn++); // Include opening quote
        while (*toIn && *toIn != '"') {
            token[tokenLen++] = *(toIn++);
        }
        if (*toIn == '"') {
            token[tokenLen++] = *(toIn++); // Include closing quote
        }
    } else {
        while (*toIn > 32) {
            token[tokenLen++] = *(toIn++);
        }
    }
    
    token[tokenLen] = 0;
    checkNumType(token);
    return tokenLen;
}

void print(ATOM *atom) {
    if (atom == NULL) { printf("nil\n"); return; }
    switch (atom->type) {
        case  ATOM_INT:    printf("%ld", (long)atom->val.intVal);
        BCASE ATOM_DOUBLE: printf("%f", atom->val.dblVal);
        BCASE ATOM_STRING: printf("\"%s\"", atom->val.str);
        BCASE ATOM_SYMBOL: printf("%s", atom->val.sym);
        BCASE ATOM_LIST:   printf("( ");
            LIST *entry = atom->val.lst;
            while (entry != NULL) {
                print(entry->car);
                printf(" ");
                entry = entry->cdr;
            }
            printf(")");
        break; default:
            printf("Print: unknown ATOM type: %d\n", atom->type);
    }
}

// Parser
LIST *newListEntry() {
    if (num_lists > MAX_LIST) { printf("Out of lists!"); err=999; return NULL; }
    LIST *e = &lists[num_lists++];
    e->loc = num_lists - 1;
	e->car = NULL;
	e->cdr = NULL;
    return e;
}

ATOM *newAtom(char type, double num, char *str) {
    if (num_atoms > MAX_ATOM) { printf("Out of atoms!"); err=999; return NULL; }
    ATOM *a = &atoms[num_atoms++];
	a->type = type;
    a->loc = num_atoms - 1;
    if (type == ATOM_INT) a->val.intVal = (long)num;
    else if (type == ATOM_DOUBLE) a->val.dblVal = num;
    else if (type == ATOM_STRING) a->val.str = strDup(str);
    else if (type == ATOM_SYMBOL) a->val.sym = strDup(str);
    else if (type == ATOM_LIST) a->val.lst = newListEntry();
    return a;
}

ATOM *parse() {
    if  (err) { return NULL; }
    if (tokenLen == 0) { return NULL; }
    if (isInt) { return newAtom(ATOM_INT, (double)intVal, NULL); }
    if (isDbl) { return newAtom(ATOM_DOUBLE, dblVal, NULL); }
    if (strEq(token, "(")) {
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
    // Handle quoted strings
    if (token[0] == '"' && tokenLen > 1 && token[tokenLen-1] == '"') {
        token[tokenLen-1] = '\0'; // Remove closing quote
        return newAtom(ATOM_STRING, 0, token+1); // Skip opening quote
    }
    // Everything else is a symbol
    return newAtom(ATOM_SYMBOL, 0, token);
}

double getNum(ATOM *a, int *isInt) {
    if (isInteger(a)) { *isInt = 1; return (double)a->val.intVal; }
    else if (isDouble(a)) { *isInt = 0; return a->val.dblVal; }
    else { printf("getNum: not a number"); err = 1; return 0; }
}

ATOM *doOp(LIST *l, int op) {
    double result;
	int allInts = 1, isInt, first = 1;
    while (l != NULL) {
        ATOM *a = eval(l->car, NULL);
        double num = getNum(a, &isInt);
        if (!isInt) { allInts = 0; }
        if (first) { result = num; first = 0; }
        else if (op == OP_ADD) { result += num; }
        else if (op == OP_SUB) { result -= num; }
        else if (op == OP_MUL) { result *= num; }
        else if (op == OP_DIV) { result /= num; }
		l = l->cdr;
    }
    if (allInts) { return newAtom(ATOM_INT, result, NULL); }
	return newAtom(ATOM_DOUBLE, result, NULL);
}

ATOM *eval(ATOM *atom, LIST *cdr) {
    if (atom == NULL) { return NULL; }
    switch (atom->type) {
	    case ATOM_INT: return atom;
        case ATOM_DOUBLE: return atom;
        case ATOM_STRING: return atom; // Strings evaluate to themselves
        case ATOM_SYMBOL:
            if (strEq(atom->val.sym, "+")) { return doOp(cdr, OP_ADD); }
            else if (strEq(atom->val.sym, "-")) { return doOp(cdr, OP_SUB); }
            else if (strEq(atom->val.sym, "*")) { return doOp(cdr, OP_MUL); }
            else if (strEq(atom->val.sym, "/")) { return doOp(cdr, OP_DIV); }
            else if (strEq(atom->val.sym, "nil")) { return NULL; }
            else if (strEq(atom->val.sym, "define")) {
                if (cdr && cdr->car && cdr->cdr && cdr->cdr->car) {
                    if (isSymbol(cdr->car)) {
                        ATOM *value = eval(cdr->cdr->car, NULL);
                        defineSymbol(cdr->car->val.sym, value);
                        return value;
                    }
                }
                printf("Invalid define syntax");
                err = 1;
                return NULL;
            }
            else if (strEq(atom->val.sym, "set")) {
                if (cdr && cdr->car && cdr->cdr && cdr->cdr->car) {
                    if (isSymbol(cdr->car)) {
                        ATOM *value = eval(cdr->cdr->car, NULL);
                        if (setSymbol(cdr->car->val.sym, value)) {
                            return value;
                        } else {
                            printf("Cannot set undefined variable: %s\n", cdr->car->val.sym);
                            err = 1;
                            return NULL;
                        }
                    }
                }
                printf("Invalid set syntax");
                err = 1;
                return NULL;
            }
            else if (strEq(atom->val.sym, "car")) {
                if (cdr && cdr->car) {
                    ATOM *arg = eval(cdr->car, NULL);
                    if (arg && isList(arg) && arg->val.lst) {
                        return arg->val.lst->car;
                    }
                }
                return NULL;
            }
            else if (strEq(atom->val.sym, "cdr")) {
                if (cdr && cdr->car) {
                    ATOM *arg = eval(cdr->car, NULL);
                    if (arg && isList(arg) && arg->val.lst && arg->val.lst->cdr) {
                        ATOM *result = newAtom(ATOM_LIST, 0, NULL);
                        result->val.lst = arg->val.lst->cdr;
                        return result;
                    }
                }
                return NULL;
            }
            else if (strEq(atom->val.sym, "quote")) {
                if (cdr && cdr->car) {
                    return cdr->car; // Return argument without evaluating
                }
                return NULL;
            }
            else if (strEq(atom->val.sym, "list")) {
                // Create a new list from the evaluated arguments  
                if (!cdr) {
                    ATOM *empty = newAtom(ATOM_LIST, 0, NULL);
                    return empty;
                }
                ATOM *result = newAtom(ATOM_LIST, 0, NULL);
                LIST *entry = result->val.lst;
                LIST *arg = cdr;
                int first = 1;
                while (arg != NULL) {
                    ATOM *evalArg = eval(arg->car, NULL);
                    if (first) { 
                        entry->car = evalArg;
                        first = 0;
                    } else {
                        LIST *newEntry = newListEntry();
                        newEntry->car = evalArg;
                        entry->cdr = newEntry;
                        entry = newEntry;
                    }
                    arg = arg->cdr;
                }
                return result;
            }
            else {
                // Try to lookup as variable
                ATOM *value = lookupSymbol(atom->val.sym);
                if (value != NULL) {
                    return value;
                }
                printf("Undefined symbol: %s\n", atom->val.sym);
                err = 1;
                return NULL;
            }
        case ATOM_LIST:
            if (atom->val.lst && atom->val.lst->car) {
                return eval(atom->val.lst->car, atom->val.lst->cdr);
            }
            return atom; // Empty list evaluates to itself
        default:
            printf("Eval: unknown ATOM type: %d\n", atom->type);
            return NULL;
    }
}

int readInput() {
    printf("\nlisp: ");
    if (fgets(tib, sizeof(tib), stdin) == NULL) {
        printf("\nGoodbye!\n");
        return 0; // EOF (Ctrl+D)
    }
    
    // Remove newline if present
    int len = strlen(tib);
    if (len > 0 && tib[len-1] == '\n') {
        tib[len-1] = '\0';
    }
    
    // Check for quit commands
    if (strEq(tib, "quit") || strEq(tib, "exit")) {
        printf("Goodbye!\n");
        return 0;
    }
    
    // Skip empty lines
    if (strlen(tib) == 0) {
        return 1;
    }
    
    toIn = prep(tib);
    err = 0;
    return 1;
}

void test(char *s) {
    printf("\ntest: %s\n", s);
    toIn = prep(s);
    nextToken();
    ATOM *result = eval(parse(), NULL);
    print(result);
    printf("\n");
    // Reset memory allocators after each test
    num_atoms = 0;
    num_lists = 0;
    err = 0;
}

void resetMemory() {
    num_atoms = 0;
    num_lists = 0;
    num_symbols = 0;
    err = 0;
}

void tests() {
    test("123456789012345.6789");
    test("(+ 1 2 3)");
    test("(* 2 3 4)");
    test("\"hello world\"");
    test("hello");
    test("(quote (1 2 3))");  
    test("(car (quote (1 2 3)))");
    test("(cdr (quote (1 2 3)))");
    test("(list 1 2 3 4)");
    
    // Variable binding tests
    printf("\nVariable binding tests:\n");
    resetMemory();
    test("(define x 42)");
    test("x");
    test("(define name \"Alice\")");
    test("name");
    test("(define sum (+ 10 20))");
    test("sum");
    test("(set x 100)");
    test("x");
    test("undefined-var");
    test("(set missing 5)");
}

int main() {
    printf("=== Minimal LISP Interpreter ===\n");
    printf("Built-in tests:\n");
    tests();
    
    printf("\n=== Interactive REPL ===\n");
    printf("Type expressions to evaluate, 'quit' or 'exit' to quit, or Ctrl+D\n");
    
    while (1) {
        if (!readInput()) {
            break; // User quit or EOF
        }
        
        if (nextToken() > 0) {
            ATOM *result = eval(parse(), NULL);
            if (!err && result != NULL) {
                print(result);
                printf("\n");
            } else if (!err) {
                printf("nil\n");
            }
            // Errors are already printed by the eval function
        }
        
        // Reset for next iteration
        num_atoms = 0;
        num_lists = 0;
        // Symbol table values are now stored directly, so resetting atoms/lists is safe
        err = 0;
    }
    
    return 0;
}
