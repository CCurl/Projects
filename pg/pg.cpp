#include <stdio.h>

#define BTW(x,l,h) ((l<=x)&&(x<=h))

typedef struct token_t {
	int token;
	char id[32];
	char *str;
} TOK_T;

TOK_T tokStk[32];
FILE *fp, *fpStk[32];
char tib[132], *in;
int fSP, tokSP, err;

char lCase(char c) { return BTW(c,'A','Z') ? c-32: c; }

int strLen(const char *x) {
	int l=0;
	while (*(x++)) ++l;
	return l;
}

int strEq(const char *x, const char *y) {
	while (*x && *y && (*x == *y)) { x++; y++; }
	return (*x|*y) ? 0 : 1;
}

int strEqI(const char *x, const char *y) {
	while (*x && *y && (lCase(*x) == lCase(*y))) { x++; y++; }
	return (*x|*y) ? 0 : 1;
}

char *rTrim(char *x) {
	int l=strLen(x);
	while (l && (x[l-1]<33)) { --l; x[l] = 0; }
	return x;
}

char *lTrim(char *x) {
	while (*x && (*x < 33)) { ++x; }
	return x;
}

char PK() { return *in; }

char NC() {
	while ( PK() && (PK() < 33) ) { ++in; }
	if (PK() == 0) { return 0; }
	return *(in++);
}

char *nextLine() {
	tib[0] = 0;
	while (fp && feof(fp)) {
		fclose(fp);
		fp = NULL;
		if (0 < fSP) {
			fp = fpStk[fSP--];
			return nextLine();
		}
	}

	if ((fp) && (fgets(tib, sizeof(tib), fp) == tib)) {
		return rTrim(tib);
	}
	return 0;
}

enum { EOS = 1, EOB, ID,  };
char idBuf[32];

int getID(char c) {
	int l = 0;
	if (c) { idBuf[l++] = c; }
	else { in = lTrim(in); }
	while (PK()) {
		c = lCase(PK());
		if (BTW(c,'a','z') || BTW(c,'0','9') || (c=='_')) {
			idBuf[l++] = NC();
		} else {
			break;
		}
	}
	idBuf[l] = 0;
	printf("-%d:%s-", l, idBuf);
	return l;
}

int tokIdent() {
	while (*in) {
		++in;
	}
	return 0;
}

int doErr(const char *msg) {
	printf("ERR: %s", msg);
	++err;
	return 0;
}

int doPragma() {
	if (getID(0) == 0) { return doErr("invalid #pragma"); }
	if (strEq(idBuf, "include")) {
		if (getID(0) == 0) { return doErr("invalid #include"); }
		printf("-include [%s]-", idBuf);
	} else {
		doErr("invalid #pragma!");
	}
	return 0;
}

int stkKW(int KW) {
	printf("-%s,kw:%d-", idBuf, KW);
	return KW;
}

int checkKW() {
	if (strEq(idBuf,"begin")) return stkKW(1);
	if (strEq(idBuf,"while")) return stkKW(2);
	if (strEq(idBuf,"repeat")) return stkKW(3);
	if (strEq(idBuf,"again")) return stkKW(4);
	if (strEq(idBuf,"if")) return stkKW(5);
	if (strEq(idBuf,"else")) return stkKW(6);
	if (strEq(idBuf,"then")) return stkKW(7);
	return 0;
}

int doID(char c) {
	int l = getID(c);
	if (checkKW()) { return 1; }
	return 1;
}

int nextToken() {
	in = lTrim(in);
	char c = NC();
	if (c == '#') { return doPragma(); }
	if (BTW(c,'A','Z')) { return doID(c); }
	if (BTW(c,'a','z')) { return doID(c); }
	if (c == '_') { return doID(c); }
	return 0;
}

void doGenerate() {
	for (int i=1; 0<=tokSP; i++) {
		TOK_T *t = &tokStk[i];
		printf("%d: %d, [%s], %p", i, t->token, t->id, t->str );
		if (t->str) { printf(", [%s]", t->str); }
		printf("\n");
	}
	tokSP = 0;
}

void parseLine() {
	in = nextLine();
	printf("[%s]\n", in);
	while (in && PK()) { nextToken(); }
}

int main(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		printf("%d: [%s]", i, argv[i]);
	}
	fSP = tokSP = 0;
	fp = NULL;
	if (argc>1) {
		fp = fopen(argv[1],"rt");
	}
	while (fp && (err<10)) {
		parseLine();
	}
	return 0;
}
