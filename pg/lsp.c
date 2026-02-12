// A Tachyon inspired system, MIT license, (c) 2025 Chris Curl

#include <stdio.h>
#include <string.h>

#define MEM_SZ 16*1024*1024
#define btwi(a ,b, c) ((a) >= (b) && (a) <= (c))

char mem[MEM_SZ], *toIn, wd[256];
int sp = 0;
double stk[256];

void push(double v) { if (sp < 255) stk[sp++] = v; }
double pop() { return (sp > 0) ? stk[--sp] : 0; }

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

int isNum(const char *w) {
	double n = 0;
    int isNeg = 0;
	if (w[0] == '-') { isNeg = 1; ++w; }
	if (w[0] == 0) { return 0; }
	while (*w && (*w != '.')) {
		char c = *(w++);
		if (btwi(c,'0','9') && btwi(c,'0','9')) { n = (n*10)+(c-'0'); }
		else return 0;
	}
    if (*w == '.') {
        ++w;
        double frac = 1;
        while (*w) {
            char c = *(w++);
            if (btwi(c,'0','9') && btwi(c,'0','9')) { frac /= 10; n += (c-'0')*frac; }
            else return 0;
        }
    }
	push(isNeg ? -n : n);
	return 1;
}

void parse(const char *src) {
    toIn = (char*)src;
    while (nextWord()) {
        if (isNum(wd)) { double x = pop(); printf("num: %f (%ld)\n", x, (long)x); }
        else if (wd[0] == '"') { printf("string: %s\n", &wd[1]); }
        else if (wd[0] == '(') { printf("-lpar-\n"); }
        else if (wd[0] == ')') { printf("-rpar-\n"); }
        else if (strcmp(wd, "car") == 0) { printf("-car-\n"); }
        else if (strcmp(wd, "cdr") == 0) { printf("-cdr-\n"); }
        else if (strcmp(wd, "lambda") == 0) { printf("-lambda-\n"); }
        else if (strcmp(wd, "def") == 0) { printf("-def-\n"); }
        else { printf("word: %s\n", wd); }
    }
}

int main() {
    toIn = mem;
    parse("123451234512345.6789 ( \"hello world\" (2 3 (foo bar)) )");
    parse("lambda (L) (car L))");
    parse("def (func x) (cdr x))");
    return 0;
}
