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
				"T" |
				"F" |
				<nil>
	<list> ::=	<atom> |
				<list> <atom>
	<enum> ::=	<alpha> |
				<enum> <alphanum>
*/

#define NUM_OBJECTS 100
#define BTW(n,l,h) ((l<=n) && (n<=h))
#define BCASE break; case

#define TOKEN_LPAR    1
#define TOKEN_RPAR    2
#define TOKEN_EOI     6
#define TOKEN_UNK     7
#define TOKEN_SEXPR  12

#define ATOM_LIST    8
#define ATOM_NUMBER  5
#define ATOM_STRING  3
#define ATOM_SYMBOL  4
#define ATOM_NIL     9
#define ATOM_TRUE   10
#define ATOM_FALSE  11

typedef unsigned long ulong;
typedef struct NODE_S { int type; struct NODE_S* next; long val; } NODE_T;

char tib[1024];
long state = 0;
NODE_T object[NUM_OBJECTS], *freeList;

void zType(const char *str) { fputs(str, stdout); }

long getCar(NODE_T *obj) { return obj->val; }
NODE_T *getCdr(NODE_T *obj) { return obj->next; }
int  getType(NODE_T *obj) { return obj->type; }

void setCar(NODE_T *obj, long val) { obj->val = val; }
void setCdr(NODE_T *obj, NODE_T* next) { obj->next = next; }
void setType(NODE_T *obj, int type) { obj->type = type; }

long stk[32], sp = 0;
void push(long x) { if (sp < 31) { stk[++sp] = x; } }
long pop() { return (0 < sp) ? stk[sp--] : 0; }

NODE_T *myAlloc(long val, int type, NODE_T *next) {
	NODE_T *obj = NULL;
	if (freeList == NULL) { zType("-Empty-"); }
	else {
		obj = freeList;
		freeList = getCdr(obj);
		setType(obj, type);
		setCar(obj, val);
		setCdr(obj, next);
	}	
	return obj;
}	

void myFree(NODE_T *obj) {
	setCdr(obj, freeList);
	freeList = obj;
}

long isNum(char *wd) {
	long theNum = 0, isNeg = 0;
	if (*wd == '-') { ++wd; isNeg = 1; }
	while (*wd) {
		if (!BTW(*wd,'0','9')) { return 0; }
		theNum = (theNum * 10) + (*(wd++)-'0');
	}
	push(isNeg ? -theNum : theNum);
	return 1;
}

char *toIn, tok[64];
int len, tokType;


int tokenSub() {
	while (*toIn && (*toIn < 33)) { ++toIn; }
	if (*toIn == 0)   { return TOKEN_EOI; }
	
	len = 0;
	if (*toIn == '"') {
		++toIn;
		while (*toIn && (*toIn != '"')) { tok[len++] = *(toIn++); }
		if (*toIn) { ++toIn; }
		tok[len] = 0;
		return ATOM_STRING;
	}

	while (*toIn && (*toIn > 32)) { tok[len++] = *(toIn++); }
	tok[len] = 0;

	if (isNum(tok)) { return ATOM_NUMBER; }
	if (strcmp(tok, "(") == 0) { return TOKEN_LPAR; }
	if (strcmp(tok, ")") == 0) { return TOKEN_RPAR; }
	if (strcmp(tok, "T") == 0) { tokType = ATOM_TRUE; }
	if (strcmp(tok, "F") == 0) { tokType = ATOM_FALSE; }
	if (strcmp(tok, "<nil>") == 0) { tokType = ATOM_NIL; }
	if (BTW(tok[0], 'A', 'Z')) { return ATOM_SYMBOL; }
	if (BTW(tok[0], 'a', 'z')) { return ATOM_SYMBOL; }
	return TOKEN_UNK;
}

int nextToken() {
	tokType = tokenSub();
	return tokType;
}

NODE_T *buildAtom() {

	tokType = nextToken();
	if (tokType == TOKEN_EOI) { return NULL; }
	if (tokType == TOKEN_RPAR) { return NULL; }
	if (tokType == TOKEN_UNK) { return NULL; }

	NODE_T *atom = NULL, *o = NULL, *n = NULL;
	switch (tokType) {
		case TOKEN_LPAR:
			atom = myAlloc(0, ATOM_LIST, NULL);
			o = buildAtom();
			if (tokType != TOKEN_RPAR) { return NULL; }
			setCar(atom, (long)o);
			n = buildAtom();
			while (n) {
				if (tokType == TOKEN_UNK) { return NULL; }
				if (tokType == TOKEN_RPAR) { return atom; }
				setCdr(o, n);
				o = n;
				n = buildAtom();
			}
		BCASE ATOM_STRING: printf("-str/%s-", tok);
		BCASE ATOM_NUMBER: return myAlloc(pop(), ATOM_NUMBER, NULL);
		BCASE ATOM_SYMBOL: printf("-sym/%s-", tok);
	}
	return NULL;
}

void dumpAtom(NODE_T *obj) {
	NODE_T *o = obj;
	while (o) {
		switch (getType(o)) {
			case ATOM_LIST:    zType("( "); dumpAtom((NODE_T*)getCar(o)); zType(") ");
			BCASE TOKEN_SEXPR:  dumpAtom((NODE_T*)getCar(o));
			BCASE ATOM_NUMBER: printf("%ld ", getCar(o));
			BCASE ATOM_STRING: zType("<string> ");
			BCASE ATOM_SYMBOL: zType("<symbol> ");
			BCASE ATOM_NIL:    zType("<nil> ");
			BCASE ATOM_TRUE:   zType("<true> ");
			BCASE ATOM_FALSE:  zType("<false> ");
			break; default: break;
		}
		if (o) { o = getCdr(o); }
	}
}

void outer(const char *src) {
	printf("[%d/%s]\n", strlen(src), src);
	toIn = (char *)src;

	NODE_T *atom = buildAtom();
	while (atom) {
		if (atom) {
			dumpAtom(atom);
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
	
	NODE_T *o2 = myAlloc(22222, ATOM_NUMBER, NULL);
	NODE_T *o1 = myAlloc(0, ATOM_STRING, o2);
	NODE_T* o3 = myAlloc((long)o2, TOKEN_SEXPR, NULL);
	NODE_T *o4 = myAlloc(4444, ATOM_NUMBER, NULL);
	NODE_T *o5 = myAlloc((long)o4, ATOM_LIST, o3);
	NODE_T* o6 = myAlloc((long)o1, ATOM_LIST, NULL);
	// dumpNode(o1); zType("\n");
	// dumpNode(o2); zType("\n");
	dumpAtom(o3); zType("\n");
	dumpAtom(o4); zType("\n");
	dumpAtom(o5); zType("\n");
	dumpAtom(o6); zType("\n");
	myFree(o1);
	myFree(o2);
	myFree(o3);
	myFree(o4);

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
