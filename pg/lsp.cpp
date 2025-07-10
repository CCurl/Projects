#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define NUM_OBJECTS 100
#define BTW(n,l,h) ((l<=n) && (n<=h))

#define TOKEN_LPAR    1
#define TOKEN_RPAR    2
#define TOKEN_STRING  3
#define TOKEN_SYMBOL  4
#define TOKEN_NUMBER  5
#define TOKEN_EOI     6
#define TOKEN_UNK     7

typedef struct OBJ { struct OBJ *next; long val; } OBJECT_T;

char tib[1024];
long state = 0;
OBJECT_T object[NUM_OBJECTS], *freeList;

void zType(const char *str) { fputs(str, stdout); }

long car(OBJECT_T *obj) { return obj->val; }
OBJECT_T *cdr(OBJECT_T *obj) { return obj->next; }
void setCar(OBJECT_T *obj, long val) { obj->val = val; }
void setCdr(OBJECT_T *obj, OBJECT_T *next) { obj->next = next; }

long stk[32], sp = 0;
void push(long x) { if (sp < 31) { stk[++sp] = x; } }
long pop() { return (0 < sp) ? stk[sp--] : 0; }

OBJECT_T *myAlloc(long val, OBJECT_T *next) {
	OBJECT_T *obj = NULL;
	if (freeList == NULL) { zType("-Empty-"); }
	else {
		obj = freeList;
		freeList = cdr(obj);
		setCar(obj, val);
		setCdr(obj, next);
	}	
	return obj;
}	

void myFree(OBJECT_T *obj) {
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
long len, tokType;
long nextToken() {
	while (*toIn && (*toIn < 33)) { ++toIn; }
	if (*toIn == 0)   { return TOKEN_EOI; }
	if (*toIn == '(') { ++toIn; return TOKEN_LPAR; }
	if (*toIn == ')') { ++toIn; return TOKEN_RPAR; }
	
	len = 0;
	if (*toIn == '"') {
		++toIn;
		while (*toIn && (*toIn != '"')) { tok[len++] = *(toIn++); }
		if (*toIn) { ++toIn; }
		tok[len] = 0;
		return TOKEN_STRING;
	}

	while (*toIn && (*toIn > 32)) {
		if (*toIn == ')') { break; }
		char ch = *(toIn++);
		tok[len++] = ch;
	}
	tok[len] = 0;
	if (isNum(tok)) { return TOKEN_NUMBER; }
	return TOKEN_SYMBOL;
}

void outer(const char *src) {
	printf("[%d/%s]\n", strlen(src), src);
	toIn = (char *)src;
	while (1) {
		tokType = nextToken();
		switch (tokType) {
			case TOKEN_EOI:    return;
			case TOKEN_LPAR:   zType("-lp-"); break;
			case TOKEN_RPAR:   zType("-rp-"); break;
			case TOKEN_STRING: printf("-str/%s-", tok); break;
			case TOKEN_NUMBER: printf("-num/%ld-", pop()); break;
			case TOKEN_SYMBOL: printf("-sym/%s-", tok); break;
			case TOKEN_UNK:    zType("-unk-"); break;
			default: break;
		}
	}
}

void printObj(OBJECT_T *obj) {
	OBJECT_T *o = obj;
	while (o) {
		if ((ulong)cdr(obj) == 1) {
			printf("\n( ");
			printObj((OBJECT_T *)car(o));
			printf(")");
			o = NULL;
		} else if ((ulong)cdr(obj) == 2) {
			printf("\n%ld", car(o));
			o = NULL;
		} else {
			printf("%ld ",car(o));
			o = cdr(o); 
		}
	}
}

int main(int argc, char *argv[]) {
	const char *boot_fn = (1 < argc) ? argv[1]  : "boot.lsp";
	freeList = (OBJECT_T*)NULL;
	for (long i = 0; i < NUM_OBJECTS; i++) {
		myFree(&object[i]);
	}
	
	OBJECT_T *o2 = myAlloc(22222, NULL);
	OBJECT_T *o1 = myAlloc(11111, o2);
	OBJECT_T *o3 = myAlloc((long)o1, (OBJECT_T *)1);
	OBJECT_T *o4 = myAlloc(4444, (OBJECT_T *)2);
	printObj(cdr(o1));
	printObj(o3);
	printObj(o4);
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
