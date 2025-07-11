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

enum { SYM_UNK, SYM_LPAR, SYM_RPAR, SYM_IF, SYM_T, SYM_F, SYM_NIL,
	 SYM_NUM, SYM_STRING, SYM_SYMBOL, SYM_EOI };
	 
typedef unsigned long ulong;
typedef union {
	long nVal;
	char cVal[64];
	char *pVal;
} ATOM_T;
typedef SNODE { SNODE *next; SNODE *child; int type; ATOM_T val; } NODE_T;

// These line up with the enums of SYMBOLS
const char *sym_words[] = { "", "(", ")", "if", "true", "false", "nil", NULL };

int ch = ' ', sym, state = 0;
char sym_name[32];
char tib[1024], *toIn, tok[64];
NODE_T object[NUM_OBJECTS], *freeList;
int len, tokType;
long theNum;

void zType(const char *str) { fputs(str, stdout); }
void message(const char *msg, int addLF) { fprintf(stderr, "%s%s", msg, addLF ? "\n" : ""); }
void error(const char *err) { message("ERROR: ", 0); message(err, 1); exit(1); }
void syntax_error() { error("syntax error"); }

int isAlpha(int ch) { return BTWI(ch,'A','Z') || BTWI(ch,'a','z'); }
int isNum(int ch) { return BTWI(ch,'0','9'); }
int isAlphaNum(int ch) { return isAlpha(ch) || isNum(ch); }
void next_ch() { ch = (*toIn) ? *(toIn++) : 0; }

NODE_T *getNext(NODE_T *obj) { return obj->next; }

NODE_T *myAlloc(int type, NODE_T *next) {
	NODE_T *obj = NULL;
	if (freeList == NULL) { error("Empty!"); }
	else {
		obj = freeList;
		freeList = getNext(obj);
		obj->type = type;
		obj->next = next;
	}
	return obj;
}	

void myFree(NODE_T *obj) {
	obj->next = freeList;
	freeList = obj;
}

long isNum(char *wd) {
	int isNeg = 0;
	long x = 0;
	if (*wd == '-') { ++wd; isNeg = 1; }
	while (*wd) {
		char c = *(wd++);
		if (!BTWI(c,'0','9')) { return 0; }
		x = (x*10) + (c-'0');
	}
	theNum = (isNeg) ? -x : x;
	return 1;
}

int symSub() {
	while (*toIn && (*toIn < 33)) { ++toIn; }
	if (*toIn == 0)   { return SYM_EOI; }
	
	len = 0;
	if (*toIn == '"') {
		++toIn;
		while (*toIn && (*toIn != '"')) { tok[len++] = *(toIn++); }
		if (*toIn) { ++toIn; }
		tok[len] = 0;
		return SYM_STRING;
	}
	
	while (*toIn && (*toIn > 32)) { tok[len++] = *(toIn++); }
	tok[len] = 0;
	
	if (isNum(tok)) { return SYM_NUM; }
	for (int i = 0; sym_words[i]; i++ ) {
		if (strcmp(sym_words[i], tok) == 0) {
			return i;
		}
	}
	if (BTWI(tok[0], 'A', 'Z')) { return SYM_SYMBOL; }
	if (BTWI(tok[0], 'a', 'z')) { return SYM_SYMBOL; }
	return sym;
}

int nextSym() {
	sym = symSub();
	// printf("-sym=%d/%s/%ld-", sym, tok, theNum);
	return sym;
}

/*---------------------------------------------------------------------------*/
/* Parser. */
enum { ATOM_LIST, ATOM_NUMBER, ATOM_STRING, ATOM_SYMBOL,
	ATOM_NIL, ATOM_TRUE, ATOM_FALSE, ATOM_SEQ
};

NODE_T *buildAtom() {
	NODE_T *x;
	// printf("(ba)");
	if (sym == SYM_EOI) { return NULL; }
	
	if (sym == SYM_NUM) {
		// printf("-num:%ld-", theNum);
		x = myAlloc(ATOM_NUMBER, NULL);
		x->val.nVal = theNum;
		return x;
	}

	if (sym == SYM_SYMBOL) {
		x = myAlloc(ATOM_SYMBOL, NULL);
		strcpy(x->val.cVal, tok);
		return x;
	}

	if (sym == SYM_T) {
		x = myAlloc(ATOM_TRUE, NULL);
		return x;
	}

	if (sym == SYM_F) {
		x = myAlloc(ATOM_FALSE, NULL);
		return x;
	}

	if (sym == SYM_NIL) {
		x = myAlloc(ATOM_NIL, NULL);
		return x;
	}

	if (sym == SYM_LPAR) {
		// printf("(list)");
		x = myAlloc(ATOM_LIST, NULL);
		x->child = NULL;
		NODE_T *n, *a;
		nextSym();
		while (sym != SYM_RPAR) {
			// printf("(na)");
			if (sym == SYM_EOI) { syntax_error(); }
			a = buildAtom();
			if (x->child == NULL) { x->child = a; }
			else { n->next = a; }
			n = a;
			nextSym();
		}
		return x;
	}
	return NULL;
}

void dumpAtom(NODE_T *obj) {
	NODE_T *o = obj;
	while (o) {
		switch (o->type) {
			case ATOM_LIST:    zType("( "); dumpAtom(o->child); zType(") ");
			BCASE ATOM_NUMBER: printf("%ld ", o->val.nVal);
			BCASE ATOM_STRING: zType("<string> ");
			BCASE ATOM_SYMBOL: printf("%s ", o->val.cVal);
			BCASE ATOM_NIL:    zType("<nil> ");
			BCASE ATOM_TRUE:   zType("<true> ");
			BCASE ATOM_FALSE:  zType("<false> ");
			break; default: break;
		}
		if (o) { o = o->next; }
	}
}

void outer(const char *src) {
	printf("[%d/%s]\n", strlen(src), src);
	toIn = (char *)src;
	nextSym();

	NODE_T *atom = buildAtom();
	while (atom) {
		if (atom) {
			dumpAtom(atom);
			nextSym();
			atom = buildAtom();
		}
	}
}

int main(int argc, char *argv[]) {
	const char *boot_fn = (1 < argc) ? argv[1]  : "boot.lsp";
	freeList = (NODE_T*)NULL;
	for (long i = 0; i < NUM_OBJECTS; i++) {
		myFree(&object[i]);
	}
	
	FILE *fp = fopen(boot_fn, "rb");
	while (state != 999) {
		if (fp == 0) { zType("\nlsp>"); }
		if (tib == fgets(tib, sizeof(tib), fp ? fp : stdin)) {
			tib[strlen(tib)-1] = 0;
			outer(tib);
			if (fp) { zType("\n"); }
		} else {
			if (fp) { fclose(fp); }
			fp = NULL;
		}
	}
	return 0;
}
