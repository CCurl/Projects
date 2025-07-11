#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/*	Grammar:
	-------------
	<atom>  ::=	<number> |
				<string> |
				<enum> |
				"(" <list> ")" |
				true |
				false |
				nil
	<list> ::=	<atom> |
				<list> <atom>
	<enum> ::=	<alpha> |
				<enum> <alphanum>
*/

#define NUM_OBJECTS 100
#define BTWI(n,l,h) ((l<=n) && (n<=h))
#define BCASE break; case
#define SNODE struct NODE_S

typedef unsigned long ulong;
typedef union {
	long nVal;
	char cVal[64];
	char *pVal;
} ATOM_T;
typedef SNODE { SNODE *next; SNODE *child; int type; ATOM_T val; } NODE_T;

int ch = ' ', sym, state = 0;
char sym_name[32];
char tib[1024], *toIn, tok[64];
NODE_T object[NUM_OBJECTS], *freeList;
int len, tokType;
long theNum;

/*---------------------------------------------------------------------------*/
/* Subs. */

void zType(const char *str) { fputs(str, stdout); }
void message(const char *msg, int addLF) { fprintf(stderr, "%s%s", msg, addLF ? "\n" : ""); }
void error(const char *err) { message("ERROR: ", 0); message(err, 1); exit(1); }
void syntax_error() { error("syntax error"); }

int isAlpha(int ch) { return BTWI(ch,'A','Z') || BTWI(ch,'a','z'); }
int isNum(int ch) { return BTWI(ch,'0','9'); }
int isAlphaNum(int ch) { return isAlpha(ch) || isNum(ch); }

NODE_T *getNext(NODE_T *obj) { return obj->next; }

NODE_T* myAlloc(int type, NODE_T* next) {
	NODE_T* obj = NULL;
	if (freeList == NULL) { error("Empty!"); }
	else {
		obj = freeList;
		freeList = getNext(obj);
		obj->type = type;
		obj->next = next;
		obj->child = NULL;
	}
	return obj;
}

void myFree(NODE_T* obj) {
	obj->next = freeList;
	freeList = obj;
}

/*---------------------------------------------------------------------------*/
/* Symbol stream. */

enum {
	SYM_UNK, SYM_LPAR, SYM_RPAR, SYM_IF, SYM_TRUE, SYM_FALSE, SYM_NIL,
	SYM_NUMBER, SYM_STRING, SYM_ENUM, SYM_EOI
};

FILE *inputFp;
char *getInput() {
	restart:
	if (inputFp == 0) { zType("\nlsp>"); }
	if (tib == fgets(tib, sizeof(tib), inputFp ? inputFp : stdin)) {
		tib[strlen(tib) - 1] = 0;
		return tib;
	}
	else {
		if (inputFp) { fclose(inputFp); }
		inputFp = NULL;
		goto restart;
	}
}

long isNum(char *w) {
	int isNeg = 0;
	long x = 0;
	if (*w == '-') { ++w; isNeg = 1; }
	while (*w) {
		char c = *(w++);
		if (!BTWI(c,'0','9')) { return 0; }
		x = (x*10) + (c-'0');
	}
	theNum = (isNeg) ? -x : x;
	return 1;
}

int isEnum(char *w) {
	if (BTWI(w[0], 'A', 'Z')) { return 1; }
	if (BTWI(w[0], 'a', 'z')) { return 1; }
	return 0;
}

int symSub() {
	restart:
	while (*toIn && (*toIn < 33)) { ++toIn; }
	if (*toIn == 0) { toIn = getInput(); goto restart; }
	
	len = 0;
	if ((*toIn == '"') || (*toIn == '\'')) {
		char c = *(toIn++);
		while (*toIn && (*toIn != c)) { tok[len++] = *(toIn++); }
		if (*toIn) { ++toIn; }
		tok[len] = 0;
		return SYM_STRING;
	}
	
	while (*toIn && (*toIn > 32)) { tok[len++] = *(toIn++); }
	tok[len] = 0;
	
	if (isNum(tok)) { return SYM_NUMBER; }
	if (isEnum(tok)) { return SYM_ENUM; }

	if (strcmp(tok, "(") == 0) { return SYM_LPAR; }
	if (strcmp(tok, ")") == 0) { return SYM_RPAR; }
	if (strcmp(tok, "if") == 0) { return SYM_IF; }
	if (strcmp(tok, "true") == 0) { return SYM_TRUE; }
	if (strcmp(tok, "false") == 0) { return SYM_FALSE; }
	if (strcmp(tok, "nil") == 0) { return SYM_NIL; }

	message("Unknown symbol", 0);
	return SYM_UNK;
}

void nextSym() { sym = symSub(); }

/*---------------------------------------------------------------------------*/
/* Parser. */

enum {
	ATOM_LIST, ATOM_NUMBER, ATOM_STRING, ATOM_ENUM,
	ATOM_NIL, ATOM_TRUE, ATOM_FALSE, ATOM_SEQ
};

NODE_T *buildAtom() {
	NODE_T *x = NULL, *n = NULL, *a = NULL;

	switch (sym) {
		case SYM_UNK: return NULL;
		case SYM_TRUE: return myAlloc(ATOM_TRUE, NULL);
		case SYM_FALSE: return myAlloc(ATOM_FALSE, NULL);
		case SYM_NIL: return myAlloc(ATOM_NIL, NULL);
		case SYM_NUMBER:
			x = myAlloc(ATOM_NUMBER, NULL);
			x->val.nVal = theNum;
			return x;
		case SYM_ENUM:
			x = myAlloc(ATOM_ENUM, NULL);
			strcpy(x->val.cVal, tok);
			return x;
		case SYM_STRING:
			x = myAlloc(ATOM_STRING, NULL);
			strcpy(x->val.cVal, tok);
			return x;
		case SYM_LPAR:
			x = myAlloc(ATOM_LIST, NULL);
			x->child = NULL;
			nextSym();
			while (sym != SYM_RPAR) {
				if (sym == SYM_UNK) { return NULL; }
				a = buildAtom();
				if (x->child == NULL) { x->child = a; }
				else if (n) { n->next = a; }
				n = a;
				nextSym();
			}
			return x;
		default: syntax_error();
	}
	
	return NULL;
}

void dumpAtom(NODE_T *obj) {
	NODE_T *o = obj;
	while (o) {
		switch (o->type) {
			case ATOM_LIST:    zType("( "); dumpAtom(o->child); zType(")");
			BCASE ATOM_NUMBER: printf("%ld", o->val.nVal);
			BCASE ATOM_STRING: printf("'%s'", o->val.cVal);
			BCASE ATOM_ENUM: printf("%s", o->val.cVal);
			BCASE ATOM_NIL:    zType("<nil>");
			BCASE ATOM_TRUE:   zType("<true>");
			BCASE ATOM_FALSE:  zType("<false>");
			break; default: break;
		}
		zType(" ");
		if (o) { o = o->next; }
	}
}

void repl(const char *src) {
	toIn = (char *)src;
	if (toIn == NULL) { toIn = getInput(); }

	while (1) {
		nextSym();
		NODE_T *atom = buildAtom();
		if (state == 999) { return; }
		if (atom) { dumpAtom(atom); }
	}
}

int main(int argc, char *argv[]) {
	const char *boot_fn = (1 < argc) ? argv[1]  : "boot.lsp";
	freeList = (NODE_T*)NULL;
	for (long i = 0; i < NUM_OBJECTS; i++) {
		myFree(&object[i]);
	}
	
	inputFp = fopen(boot_fn, "rb");
	repl(0);
	return 0;
}
