#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/*	Grammar:
	-------------
	<atom>  ::=	<number> |
				'<string>' |
				"<string>" |
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
#define BTWI(n,l,h) (((l)<=(n)) && ((n)<=(h)))
#define BCASE break; case
#define SNODE struct NODE_S

typedef unsigned long ulong;
typedef char *PCHAR;
typedef SNODE { SNODE *next; int type; char *val; } NODE_T, *PNODE;

int ch = ' ', sym, state = 0;
char sym_name[32], tib[1024], *toIn, tok[64];
long theNum;
NODE_T object[NUM_OBJECTS], *freeList;

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

/*---------------------------------------------------------------------------*/
/* A simple heap allocator */
#define HEAPINDEX_SZ 1000
#define HEAP_SZ 100000

typedef struct {
	uint32_t sz, inUse, off;
} HEAP_T, *PHEAP;

uint32_t hHere = 0, hiCount = 0;
HEAP_T hIndex[HEAPINDEX_SZ];
char heap[HEAP_SZ];

void hDump() {
	printf("\nn-%u/h-%u",hiCount,hHere);
	for (int i = 0; i < hiCount; i++) {
		PHEAP x = (PHEAP)&hIndex[i];
		printf("\nhi-%d/iu-%u/sz-%u/off-%u/d:[%s]",
			i, x->inUse, x->sz, x->off, &heap[x->off]);
	}
	printf("\n");
}

int hFindFree(int sz) {
	int best = -1;
	for (int i = 0; i < hiCount; i++) {
		PHEAP x = &hIndex[i];
		if ((x->inUse == 0) && (sz <= x->sz)) {
			if (x->sz == sz) { return i; }
			if ((best == -1) || (hIndex[best].sz < x->sz)) { best = i; }
		}
	}
	return best;
}

PCHAR hAlloc(int sz) {
	const int hASG = 8; // alloc size granularity
	if (sz == 0) { sz = 1; }
	if ((sz % hASG) != 0) { sz += hASG - (sz % hASG); }
	
	int hi = hFindFree(sz);
	if (0 <= hi) {
		hIndex[hi].inUse = 0;
		return &heap[hIndex[hi].off];
	}
	
	int newHere = hHere + sz;
	if (HEAP_SZ <= newHere) { error("heap full!"); }
	if (HEAPINDEX_SZ <= hiCount) { error("heap index full!"); }
	
	PHEAP x = &hIndex[hiCount++];
	x->sz = sz;
	x->off = hHere;
	x->inUse = 1;
	hHere = newHere;
	return &heap[x->off];
}

int hFindData(char *data) {
	int32_t off = data-&heap[0];
	if (!BTWI(off,0,HEAP_SZ-1)) { return -1; }
	for (int i = 0; i < hiCount; i++) {
		if (hIndex[i].off == off) { return i; }
	}
	return -1;
}

void hFree(char *data) {
	int hi = hFindData(data);
	if (hi == -1) { return; }
	PHEAP x = &hIndex[hi];
	x->inUse = 0;
	while (0 < hiCount) {
		x = (PHEAP)&hIndex[hiCount-1];
		if (x->inUse) { break; }
		hHere = x->off;
		x->off = x->sz = 0;
		hiCount--;
	}
}

/*---------------------------------------------------------------------------*/
/* NODE allocate/free */

PNODE ndAlloc(int type) {
	if (freeList) {
		PNODE obj = freeList;
		freeList = freeList->next;
		obj->type = type;
		obj->next = NULL;
		obj->val = 0;
		return obj;
	}
	error("Out of nodes!");
	return NULL;
}

void ndFree(PNODE obj) {
	while (obj) { 
		// printf("-ndf/%d-", obj->type);
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
		// printf("--%s", tib);
		return tib;
	} else {
		if (inputFp) { fclose(inputFp); }
		return NULL;
		// inputFp = NULL;
		// return getInput();
	}
}

long isNum(const char *w) {
	long x = 0, isNeg = (*w == '-');
	if (isNeg) { ++w; if (*w == 0) { return 0; } }
	while (*w) {
		char c = *(w++);
		if (!BTWI(c,'0','9')) { return 0; }
		x = (x*10) + (c-'0');
	}
	theNum = (isNeg) ? -x : x;
	return 1;
}

int isEnum(const char *w) {
	if (!isAlpha(*(w++))) { return 0; }
	while (*w) { if (!isAlphaNum(*(w++))) { return 0; } }
	return 1;
}

int symSub() {
	restart:
	if (toIn == NULL) { return SYM_EOI; }
	while (*toIn && (*toIn < 33)) { ++toIn; }
	if (*toIn == 0) { toIn = getInput(); goto restart; }
	
	int len = 0;
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
		case SYM_TRUE:  return ndAlloc(ATOM_TRUE);
		case SYM_FALSE: return ndAlloc(ATOM_FALSE);
		case SYM_NIL:   return ndAlloc(ATOM_NIL);
		case SYM_ADD:   return ndAlloc(ATOM_ADD);
		case SYM_NUMBER:
			x = ndAlloc(ATOM_NUMBER);
			x->val = (PCHAR)theNum;
			return x;
		case SYM_ENUM:
			x = ndAlloc(ATOM_ENUM);
			x->val = hAlloc(strlen(tok)+1);
			strcpy(x->val, tok);
			return x;
		case SYM_STRING:
			x = ndAlloc(ATOM_STRING);
			x->val = hAlloc(strlen(tok)+1);
			strcpy(x->val, tok);
			return x;
		case SYM_DEFUN:
			x = ndAlloc(ATOM_FUNC);
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
			x = ndAlloc(ATOM_LIST);
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
	printf("\n");
}

void parse() {
	toIn = getInput();

	nextSym();
	while (sym != SYM_EOI) {
		PNODE a = buildAtom();
		dumpAtom(a);
		// hDump();
		ndFree(a);
		nextSym();
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
	parse();
	hDump();
	return 0;
}
