#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define CODE_SZ       0x10000
#define VARS_SZ      0x400000
#define DICT_SZ        1000
#define STK_SZ           63
#define NAME_LEN         25
#define INLINE            1
#define IMMED             2
#define LIT_MASK      0xF0000000
#define LIT_BITS      0x0FFFFFFF
#define btwi(n,l,h)   ((l<=n) && (n<=h))
#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define byte          uint8_t
#define wc_t          uint32_t
#define cell          int64_t
#define ucell         uint64_t

typedef struct { wc_t xt; byte fl; byte ln; char nm[NAME_LEN+1]; } DE_T;

wc_t code[CODE_SZ], dsp, rsp, lsp;
byte vars[VARS_SZ];
DE_T dict[1000];
cell dstk[STK_SZ+1], rstk[STK_SZ+1], lstk[STK_SZ+1], A, T;
cell here, last, vhere, base, state;
char *toIn, wd[32];

#define PRIMS \
	X(EXIT,    "exit",   0, pc=(wc_t)rpop(); if (pc <= BYE) { /* zType("\n"); */ return 0; } ) \
	X(LIT,     "",       0, push(cellAt((cell)&code[pc])); pc += (sizeof(cell)/sizeof(wc_t)); ) \
	X(JMP0,    "",       0, if (pop()) { pc = code[pc]; } else { pc++; } ) \
	X(COMMA,   ",",      0, comma((wc_t)pop()); ) \
	X(DUP,     "dup",    0, push(TOS); ) \
	X(DROP,    "drop",   0, pop(); ) \
	X(SWAP,    "swap",   0, t = TOS; TOS = NOS; NOS = t; ) \
	X(STO,     "!",      0, t = pop(); n = pop(); storeAt(t,n); ) \
	X(FET,     "@",      0, push(cellAt(pop())); ) \
	X(CSTO,    "c!",     0, t = pop(); n= pop(); *(byte*)t = (byte)n; ) \
	X(CFET,    "c@",     0, TOS = *(byte*)TOS; ) \
	X(RTO,     ">r",     0, rpush(pop()); ) \
	X(RAT,     "r@",     0, push(rstk[rsp]); ) \
	X(RFROM,   "r>",     0, push(rpop()); ) \
	X(ATO,     "a!",     0, A = pop(); ) \
	X(AAT,     "a@",     0, push(A); ) \
	X(TTO,     "t!",     0, T = pop(); ) \
	X(TAT,     "t@",     0, push(T); ) \
	X(OUTER,   "outer",  0, outer((char*)pop()); ) \
	X(MULT,    "*",      0, t = pop(); TOS *= t; ) \
	X(ADD,     "+",      0, t = pop(); TOS += t; ) \
	X(COM,     "com",    0, TOS=~TOS; ) \
	X(SMOD,    "/mod",   0, t = TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(EQ,      "=",      0, t = pop(); TOS = (TOS == t) ? 1 : 0; ) \
	X(EQ0,     "0=",     0, TOS = (TOS) ? 1 : 0; ) \
	X(EMIT,    "emit",   0, emit(pop()); ) \
	X(ZTYPE,   "ztype",  0, zType((const char*)pop()); ) \
	X(COLON,   ":",      0, addToDict(0); state = 1; ) \
	X(SEMI,    ";",  IMMED, comma(EXIT); state = 0; ) \
	X(FOR,     "for",    0, lsp += 2; lstk[lsp] = pop(); lstk[lsp-1] = pc; ) \
	X(NXT,     "next",   0, if (0 < lstk[lsp]--) { pc=(wc_t)lstk[lsp-1]; } else { lsp=(1<lsp) ? lsp-2: 0; } ) \
	X(BYE,     "bye",    0, zType("Bye"); exit(0); )

#define X(op, nm, fl, cd) op,
enum { PRIMS };

DE_T *addToDict(const char *w);
int outer(const char *src);
void compileNum(cell n);
void push(cell v) { if (dsp < STK_SZ) { dstk[++dsp] = v; } }
cell pop() { return (0 < dsp) ? dstk[dsp--] : 0; }
void rpush(cell v) { if (rsp < STK_SZ) { rstk[++rsp] = v; } }
cell rpop() { return (0 < rsp) ? rstk[rsp--] : 0; }
cell cellAt(cell loc) { return *(cell*)loc; }
void storeAt(cell loc, cell val) { *(cell*)loc = val; }
void comma(wc_t val) { code[here++] = val; }
void emit(cell ch) { fputc((char)ch, stdout); }
void zType(const char *str) { fputs(str, stdout); }
void addLit(const char* name, cell val) { addToDict(name); compileNum(val); comma(EXIT); }
int lower(int c) { return btwi(c, 'A', 'Z') ? c+32 : c; }

int strLen(const char *str) {
	int ln = 0;
	while (*(str++)) { ln++; }
	return ln;
}

void strCpy(char* dst, const char *src) {
	while (*src) { *(dst++) = *(src++); }
	*dst = 0;
}

int strEqI(const char *src, char *dst) {
	while (*src == *dst) {
		if (*src == 0) { return 1; }
		src++; dst++;
	}
	return 0;
}

void compileNum(cell n) {
	if (btwi(n, 0, LIT_BITS)) { comma((wc_t)(n | LIT_MASK)); }
	else { comma(LIT); storeAt((cell)&code[here], n); here += 2; }
}

int nextWord() {
	int ln = 0;
	while (*toIn && (*toIn < 33)) { ++toIn; }
	while (*toIn > 32) { wd[ln++] = *(toIn++); }
	wd[ln] = 0;
	return ln;
}

int isNum(const char *w, cell b) {
	cell n = 0, isNeg = 0;
	if ((w[0] == 39) && (w[2] == 39) && (w[3] == 0)) { push(w[1]); return 1; }
	if (w[0] == '%') { b = 2; ++w; }
	if (w[0] == '#') { b = 10; ++w; }
	if (w[0] == '$') { b = 16; ++w; }
	if ((b == 10) && (w[0] == '-')) { isNeg = 1; ++w; }
	if (w[0] == 0) { return 0; }
	char c = lower(*(w++));
	while (c) {
		n = (n * b);
		if (btwi(c, '0', '9') && btwi(c, '0', '0' + b - 1)) { n += (c - '0'); }
		else if (btwi(c, 'a', 'a' + b - 11)) { n += (c - 'a' + 10); }
		else return 0;
		c = lower(*(w++));
	}
	push(isNeg ? -n : n);
	return 1;
}

void iToA(cell n, cell b) {
	if (n < 0) { emit('-'); n = -n; }
	if (b <= n) { iToA(n / b, b); }
	n %= b; if (9 < n) { n += 7; }
	emit('0' + (char)n);
}

DE_T *findInDict(char *w) {
	cell cw = last;
	int ln = strLen(w);
	while (cw < DICT_SZ) {
		DE_T *dp = &dict[cw];
		if ((dp->ln == ln) && (strEqI(dp->nm, w))) { return dp; }
		cw++;
	}
	return (DE_T *)0;
}

DE_T *addToDict(const char *w) {
	if (last == 0) { return (DE_T*)0; }
	if (!w) {
		if (!nextWord()) return (DE_T*)0;
		w = &wd[0];
	}
	int ln = strLen(w);
	if (NAME_LEN < ln) {
		zType("-length:"); zType(w); emit('-');
		ln=NAME_LEN; wd[ln] = 0;
	}
	--last;
	DE_T *dp = &dict[last];
	dp->xt = (wc_t)here;
	dp->fl = 0;
	dp->ln = ln;
	strCpy(dp->nm, w);
	return dp;
}

#undef X
#define X(op, nm, fl, cd) case op: cd goto next;

int inner(wc_t pc) {
	wc_t ir;
	cell n, t;
next:
	ir = code[pc++];
	// zType("-pc:"); iToA(pc-1,10); zType(",ir:"); iToA(ir,10); emit('-');
	switch (ir)	{
		PRIMS
	default:
		if (LIT_BITS <= ir) { push(ir & LIT_BITS); goto next; }
		if (code[pc] != EXIT) { rpush(pc); }
		pc = ir;
		goto next;
	}
}

int outer(const char *src) {
	toIn = (char *)src;
	while (nextWord()) {
		if (isNum(wd, base)) {
			if (state == 1) { compileNum(pop()); }
			continue;
		}
		DE_T *dp = findInDict(wd);
		if (dp) {
			if ((state == 0) || (dp->fl == IMMED)) {
				code[10] = dp->xt;
				code[11] = EXIT;
				inner(10);
			}
			else { comma(dp->xt); }
			continue;
		}
		zType("-word?:"); zType(wd); emit('-');
		state = 0;
		return 1;
	}
	return 0;
}

void addPrim(const char *nm, wc_t op, byte fl) {
	DE_T *dp = addToDict(nm);
	dp->xt = op;
	dp->fl = fl;
}

#undef X
#define X(op, nm, fl, cd) addPrim(nm, op, fl);

int main() {
	last = DICT_SZ;
	here = BYE+1;
	base = 10;
	PRIMS
	addLit("cell", sizeof(cell));
	addLit("(vh)", (cell)&vhere);
	addLit("(h)", (cell)&here);
	addLit("(l)", (cell)&last);
	addLit("vars", (cell)&vars[0]);
	addLit("code", (cell)&code[0]);
	addLit("wc-sz", (cell)sizeof(wc_t));
	addLit("dict", (cell)&dict[0]);
	addLit("de-sz", (cell)sizeof(DE_T));
	outer(": here (h) @ wc-sz * code + ; \
		: last (l) @ de-sz * dict + ; \
		: cr #13 emit #10 emit ; \
		: bb 'S' emit 1000 dup dup * * for next 'E' emit ; \
		cr bb cr \
		");
	for (wc_t i = 1; i <= dsp; i++) { iToA(dstk[i], 10); emit(32); }
	outer("bye");
	return 0;
}
