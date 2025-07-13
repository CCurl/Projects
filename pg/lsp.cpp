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

#define HEAP_SZ 1000
#define NUM_OBJECTS 100
#define BTWI(n,l,h) (((l)<=(n)) && ((n)<=(h)))
#define BCASE break; case
#define SNODE struct NODE_S

typedef unsigned long ulong;
typedef char *PCHAR;
typedef SNODE { SNODE *next; int type; char *val; } NODE_T, *PNODE;

int ch = ' ', sym, state = 0;
char sym_name[32];
char tib[1024], *toIn, tok[64];
NODE_T object[NUM_OBJECTS], *freeList;
int len, tokType;
long theNum;

enum {
	SYM_FREE, SYM_LPAR, SYM_RPAR, SYM_IF, SYM_TRUE, SYM_FALSE, SYM_NIL,
	SYM_NUMBER, SYM_STRING, SYM_ENUM, SYM_EOI,
	SYM_DEFUN, SYM_ADD, SYM_UNK
};

enum {
	ATOM_FREE, ATOM_LIST, ATOM_NUMBER, ATOM_STRING, ATOM_ENUM,
	ATOM_NIL, ATOM_TRUE, ATOM_FALSE, ATOM_SEQ,
	ATOM_FUNC, ATOM_ADD
};

/*---------------------------------------------------------------------------*/
/* Subs. */

void zType(const char *str) { fputs(str, stdout); }
void message(const char *msg, int addLF) { fprintf(stderr, "%s%s", msg, addLF ? "\n" : ""); }
void error(const char *err) { message("ERROR: ", 0); message(err, 1); exit(1); }
void syntax_error() { error("syntax error"); }

int isAlpha(int ch) { return BTWI(ch,'A','Z') || BTWI(ch,'a','z'); }
int isNum(int ch) { return BTWI(ch,'0','9'); }
int isAlphaNum(int ch) { return isAlpha(ch) || isNum(ch); }

PNODE getNext(PNODE obj) { return obj->next; }

/*---------------------------------------------------------------------------*/
/* A simple heap allocator */

typedef struct HEAP_S {
	int prev, next, sz, inUse;
	char data[8];
} HEAP_T, *PHEAP;

const int hASG = sizeof(long); // alloc size granularity
int hHere = 0, hLast = -1;
char heap[HEAP_SZ];
#define OH (sizeof(int)*4)

PHEAP hFindFree(int sz) {
	int c = 0;
	PHEAP best = NULL;
	while (c <= hLast) {
		PHEAP cur = (PHEAP)&heap[c];
		if ((cur->inUse == 0) && (sz <= cur->sz)) {
			if (cur->sz == sz) { return cur; }
			if ((best == NULL) || (best->sz < cur->sz)) { best = cur; }
		}
		c = cur->next;
		if (c == 0) { break; }
	}
	return best;
}

PCHAR hAlloc(int sz) {
	if (sz == 0) { sz = 1; }
	if ((sz % hASG) != 0) { sz += hASG - (sz % hASG); }
	
	PHEAP h = hFindFree(sz);
	if (h) {
		h->inUse = 1;
		return &h->data[0];
	}
	
	int newLast = hHere;
	int newHere = newLast + sz + OH;
	if (HEAP_SZ <= newHere) { error("out of heap!"); return NULL; }
	
	if (0 <= hLast) {
		h = (PHEAP)&heap[hLast];
		h->next = hHere;
	}
	h = (PHEAP)&heap[hHere];
	h->prev = hLast;
	h->next = 0;
	h->sz = sz;
	h->inUse = 1;
	PCHAR ret = &h->data[0];
	hHere = newHere;
	hLast = newLast;
	return ret;
}

void hFree(PCHAR obj) {
	int i = (obj - OH) - &heap[0];
	if (!BTWI(i, 0, hHere-1)) { return; }
	PHEAP h = (PHEAP)&heap[i];
	if (h->inUse == 1) { h->inUse = 0; }
	while (0 <= hLast) {
		h = (PHEAP)&heap[hLast];
		if (h->inUse) { break; }
		hHere = hLast;
		hLast = h->prev;
	}
}

void hDump() {
	int c = 0;
	printf("\nl-%d/h-%d/oh-%d/asg-%d",hLast,hHere,OH,hASG);
	while (c <= hLast) {
		PHEAP h = (PHEAP)&heap[c];
		printf("\nc-%d/p-%d/n-%d/u-%d/sz-%d/%s",c,h->prev, h->next, h->inUse, h->sz, h->data);
		c = h->next;
		if (c == 0) { break; }
	}
	printf("\n");
}

/*---------------------------------------------------------------------------*/
/* NODE allocate/free */

PNODE ndAlloc(int type, PNODE next) {
	PNODE obj = NULL;
	if (freeList == NULL) { error("Empty!"); }
	else {
		obj = freeList;
		freeList = getNext(obj);
		obj->type = type;
		obj->next = next;
		obj->val = 0;
	}
	return obj;
}

void ndFree(PNODE obj) {
	while (obj) {
		switch (obj->type) {
			case ATOM_ENUM:
			case ATOM_STRING:
				hFree(obj->val);
				break;
			case ATOM_LIST:
				ndFree((PNODE)obj->val);
				break;
		}
		PNODE nxt = obj->next;
		obj->next = freeList;
		obj->type = ATOM_FREE;
		obj->val = 0;
		freeList = obj;
		obj = nxt;
	}
}

/*---------------------------------------------------------------------------*/
/* Symbol stream. */

FILE *inputFp;
char *getInput() {
	if (inputFp == NULL) { zType("\nlsp>"); }
	if (tib == fgets(tib, sizeof(tib), inputFp ? inputFp : stdin)) {
		return tib;
	} else {
		if (inputFp) { fclose(inputFp); }
		inputFp = NULL;
		return getInput();
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
	if (!isAlpha(w[0])) { return 0; }
	int l = 1;
	while (w[l]) { if (!isAlphaNum(w[l++])) { return 0; } }
	return 1;
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
	
	while (*toIn > 32) { tok[len++] = *(toIn++); }
	tok[len] = 0;
	
	if (isNum(tok)) { return SYM_NUMBER; }
	if (strcmp(tok, "(") == 0) { return SYM_LPAR; }
	if (strcmp(tok, ")") == 0) { return SYM_RPAR; }
	if (strcmp(tok, "if") == 0) { return SYM_IF; }
	if (strcmp(tok, "true") == 0) { return SYM_TRUE; }
	if (strcmp(tok, "false") == 0) { return SYM_FALSE; }
	if (strcmp(tok, "nil") == 0) { return SYM_NIL; }
	if (strcmp(tok, "defun") == 0) { return SYM_DEFUN; }
	if (strcmp(tok, "+") == 0) { return SYM_ADD; }
	if (isEnum(tok)) { return SYM_ENUM; }

	message("Unknown symbol", 0);
	return SYM_UNK;
}

void nextSym() { sym = symSub(); }

/*---------------------------------------------------------------------------*/
/* Parser. */

int assert(int cond, const char *msg) {
	if (cond == 0) { message(msg, 1); }
	return cond;
}

PNODE buildAtom() {
	PNODE x = NULL, n = NULL, a = NULL;

	switch (sym) {
		case SYM_UNK:   return NULL;
		case SYM_TRUE:  return ndAlloc(ATOM_TRUE, NULL);
		case SYM_FALSE: return ndAlloc(ATOM_FALSE, NULL);
		case SYM_NIL:   return ndAlloc(ATOM_NIL, NULL);
		case SYM_ADD:   return ndAlloc(ATOM_ADD, NULL);
		case SYM_NUMBER:
			x = ndAlloc(ATOM_NUMBER, NULL);
			x->val = (PCHAR)theNum;
			return x;
		case SYM_ENUM:
			x = ndAlloc(ATOM_ENUM, NULL);
			x->val = hAlloc(strlen(tok)+1);
			strcpy(x->val, tok);
			return x;
		case SYM_STRING:
			x = ndAlloc(ATOM_STRING, NULL);
			x->val = hAlloc(strlen(tok)+1);
			strcpy(x->val, tok);
			return x;
		case SYM_DEFUN:
			x = ndAlloc(ATOM_FUNC, NULL);
			nextSym();
			if (assert(sym == SYM_LPAR, "defun ( ID ... )")) {
				n = buildAtom();
				if (assert(n->type == ATOM_LIST, "defun ( ID ... )")) {
					// TODO: strcpy(x->val, tok);
					x->val = n->val;
					return x;
				}
			}
			if (n) { ndFree(x); }
			ndFree(x);
			return NULL;
		case SYM_LPAR:
			x = ndAlloc(ATOM_LIST, NULL);
			x->val = NULL;
			nextSym();
			while (sym != SYM_RPAR) {
				if (sym == SYM_UNK) { return NULL; }
				a = buildAtom();
				if (x->val == NULL) { x->val = (PCHAR)a; }
				else if (n) { n->next = a; }
				n = a;
				nextSym();
			}
			return x;
		default: syntax_error();
	}
	
	return NULL;
}

PNODE evalAtom(PNODE obj) {
	return obj;
}

void dumpAtom(PNODE obj) {
	if (obj == NULL) { message("<null>",0); }
	PNODE o = obj;
	while (o) {
		switch (o->type) {
			case ATOM_LIST:    zType("( "); dumpAtom((PNODE)o->val); zType(")");
			BCASE ATOM_NUMBER: printf("%ld", (long)o->val);
			BCASE ATOM_STRING: printf("'%s'", o->val);
			BCASE ATOM_ENUM:   printf("%s", o->val);
			BCASE ATOM_NIL:    zType("<nil>");
			BCASE ATOM_TRUE:   zType("<true>");
			BCASE ATOM_FALSE:  zType("<false>");
			BCASE ATOM_FUNC:   zType("func <TODO>"); // zType(((PNODE)o->val)->val);
			BCASE ATOM_ADD:    zType("<+>");
			break; default: break;
		}
		zType(" ");
		if (o) { o = o->next; }
	}
}

void repl(const char *src) {
	toIn = (PCHAR)src;
	if (toIn == NULL) { toIn = getInput(); }

	while (1) {
		nextSym();
		PNODE atom = buildAtom();
		PNODE x = evalAtom(atom);
		dumpAtom(x); printf("\n");
		ndFree(x);
		if (x != atom) { ndFree(atom); }
		if (state == 999) { return; }
	}
}

int main(int argc, char *argv[]) {
	const char *boot_fn = (1 < argc) ? argv[1]  : "boot.lsp";
	freeList = (PNODE)NULL;
	for (long i = 0; i < NUM_OBJECTS; i++) {
		object[i].next = 0;
		object[i].type = ATOM_FREE;
		ndFree(&object[i]);
	}
	inputFp = fopen(boot_fn, "rb");
	repl(0);
	return 0;
}
