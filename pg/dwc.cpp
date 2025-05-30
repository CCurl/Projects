#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define CODE_SZ       0x10000
#define VARS_SZ      0x400000
#define STK_SZ           63
#define NAME_LEN         25
#define IMMED          0x80
#define LAST_OP          31
#define LIT_MASK      0x70000000
#define LIT_BITS      0x0FFFFFFF
#define btwi(n,l,h)   ((l<=n) && (n<=h))
#define TOS           dstk[dsp]
#define NOS           dstk[dsp-1]
#define byte          uint8_t
#define wc_t          uint32_t
#define cell          int32_t
#define WC_SZ             4
#define CELL_SZ           4

enum { COMPILE=1, INTERPRET, COMMENT };
typedef struct { wc_t xt; byte sz; byte fl; byte ln; char nm[NAME_LEN+1]; } DE_T;

wc_t code[CODE_SZ], dsp, rsp, lsp;
byte vars[VARS_SZ];
cell dstk[STK_SZ+1], rstk[STK_SZ+1], lstk[STK_SZ+1];
cell here, last, vhere, base, state;
char *toIn, wd[32];

#define PRIMS \
	X(EXIT,  "exit",     0, pc=(wc_t)rpop(); if (pc <= LAST_OP) { return 0; } ) \
	X(LIT,   "",         0, push(cellAt((cell)&code[pc])); pc += (CELL_SZ/WC_SZ); ) \
	X(JMP0,  "",         0, if (pop()==0) { pc = code[pc]; } else { pc++; } ) \
	X(JMP,   "",         0, pc = code[pc]; ) \
	X(COMMA, ",",        0, comma((wc_t)pop()); ) \
	X(DUP,   "dup",      0, push(TOS); ) \
	X(DROP,  "drop",     0, pop(); ) \
	X(SWAP,  "swap",     0, t = TOS; TOS = NOS; NOS = t; ) \
	X(STO,   "!",        0, t = pop(); n = pop(); cellTo(t,n); ) \
	X(FET,   "@",        0, TOS = cellAt(TOS); ) \
	X(CSTO,  "c!",       0, t = pop(); n= pop(); *(byte*)t = (byte)n; ) \
	X(CFET,  "c@",       0, TOS = *(byte*)TOS; ) \
	X(RTO,   ">r",       0, rpush(pop()); ) \
	X(RAT,   "r@",       0, push(rstk[rsp]); ) \
	X(RFROM, "r>",       0, push(rpop()); ) \
	X(TIMER, "timer",    0, push(clock()); ) \
	X(MULT,  "*",        0, t = pop(); TOS *= t; ) \
	X(ADD,   "+",        0, t = pop(); TOS += t; ) \
	X(SUB,   "-",        0, t = pop(); TOS -= t; ) \
	X(SMOD,  "/mod",     0, t = TOS; n = NOS; TOS = n/t; NOS = n%t; ) \
	X(LT,    "<",        0, t = pop(); TOS = (TOS  < t) ? 1 : 0; ) \
	X(EQ,    "=",        0, t = pop(); TOS = (TOS == t) ? 1 : 0; ) \
	X(GT,    ">",        0, t = pop(); TOS = (TOS  > t) ? 1 : 0; ) \
	X(EMIT,  "emit",     0, emit(pop()); ) \
	X(ZTYPE, "ztype",    0, zType((const char*)pop()); ) \
	X(ADDW,  "add-word", 0, addToDict(0); ) \
	X(OP26,  "[",    IMMED, state = 0; ) \
	X(OP27,  "for",      0, lsp += 2; lstk[lsp] = pop(); lstk[lsp-1] = pc; ) \
	X(OP28,  "next",     0, if (0 < --lstk[lsp]) { pc=(wc_t)lstk[lsp-1]; } else { lsp=(1<lsp) ? lsp-2: 0; } ) \
	X(OP29,  "and",      0, t = pop(); TOS &= t; ) \
	X(OP30,  "or",       0, t = pop(); TOS |= t; ) \
	X(OP31,  "xor",      0, t = pop(); TOS ^= t; )

#define X(op, nm, fl, cd) op,
enum { PRIMS };

DE_T *addToDict(const char *w);
void outer(const char *src);
void compileNum(cell n);
void push(cell v) { if (dsp < STK_SZ) { dstk[++dsp] = v; } }
cell pop() { return (0 < dsp) ? dstk[dsp--] : 0; }
void rpush(cell v) { if (rsp < STK_SZ) { rstk[++rsp] = v; } }
cell rpop() { return (0 < rsp) ? rstk[rsp--] : 0; }
cell cellAt(cell loc) { return *(cell*)loc; }
void cellTo(cell loc, cell val) { *(cell*)loc = val; }
void comma(wc_t val) { code[here++] = val; }
int changeState(int st) { state = st; return st; }
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

int strEqI(const char *src, const char *dst) {
	while (lower(*src) == lower(*dst)) {
		if (*src == 0) { return 1; }
		src++; dst++;
	}
	return 0;
}

void compileNum(cell n) {
	if (btwi(n, 0, LIT_BITS)) { comma((wc_t)(n | LIT_MASK)); }
	else { comma(LIT); cellTo((cell)&code[here], n); here += (CELL_SZ/WC_SZ); }
}

int nextWord() {
	int ln = 0;
	while (*toIn && (*toIn < 33)) { if (btwi(*toIn,COMPILE, COMMA)) { changeState(*toIn); } ++toIn; }
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

DE_T *addToDict(const char *w) {
	if (!w) {
		if (!nextWord()) return (DE_T*)0;
		w = &wd[0];
	}
	int ln = strLen(w);
	if (ln == 0) { return (DE_T*)0; }
	byte sz = WC_SZ + 3 + ln + 1;
	while (sz & 0x03) { ++sz; }
	last -= sz;
	if (last < vhere) { last += sz; return (DE_T*)0; }
	DE_T *dp = (DE_T*)last;
	dp->xt = (wc_t)here;
	dp->sz = sz;
	dp->fl = 0;
	dp->ln = ln;
	strCpy(dp->nm, w);
	return dp;
}

DE_T *findInDict(char *w) {
	cell cw = last, ln = strLen(w);
	while (cw < (cell)&vars[VARS_SZ]) {
		DE_T *dp = (DE_T *)cw;
		if ((dp->ln == ln) && (strEqI(dp->nm, w))) { return dp; }
		cw += dp->sz;
	}
	return (DE_T *)0;
}

#undef X
#define X(op, nm, fl, cd) case op: cd goto next;

int inner(wc_t pc) {
	wc_t ir;
	cell n, t;
next:
	ir = code[pc++];
	switch (ir)	{
		PRIMS
	default:
		if (LIT_BITS <= ir) { push(ir & LIT_BITS); goto next; }
		if (code[pc] != EXIT) { rpush(pc); }
		pc = ir;
		goto next;
	}
}

int isStateChange(const char *w) {
	if (state == COMMENT) {
		if (strEqI(w, "))")) { return changeState(COMPILE); }
		if (strEqI(w, ")"))  { return changeState(INTERPRET); }
		return state;
	}
	if (strEqI(w, "]"))  { return changeState(COMPILE); }
	if (strEqI(w, ":"))  { addToDict(0); return changeState(COMPILE); }
	if (strEqI(w, ";"))  { comma(EXIT);  return changeState(INTERPRET); }
	if (strEqI(w, "["))  { return changeState(INTERPRET); }
	if (strEqI(w, "("))  { return changeState(COMMENT); }
	if (strEqI(w, "((")) { return changeState(COMMENT); }
	return 0;
}

void outer(const char *src) {
	char *svIn = toIn;
	toIn = (char *)src;
	while (nextWord()) {
		if (isStateChange(wd)) { continue; }
		if (isNum(wd, base)) {
			if (state == COMPILE) { compileNum(pop()); }
			continue;
		}
		DE_T *dp = findInDict(wd);
		if (!dp) {
			zType("\n-word:["); zType(wd); zType("]?-\n");
			state = INTERPRET;
			break;
		}
		if ((state == INTERPRET) || (dp->fl & IMMED)) {
			code[10] = dp->xt;
			code[11] = EXIT;
			inner(10);
		}
		else { comma(dp->xt); }
	}
	toIn = svIn;
}

void addPrim(const char *nm, wc_t op, byte fl) {
	DE_T *dp = addToDict(nm);
	if (dp) {
		dp->xt = op;
		dp->fl = fl;
	}
}

#undef X
#define X(op, nm, fl, cd) addPrim(nm, op, fl);

int main(int argc, char *argv[]) {
	const char *boot_fn = (1 < argc) ? argv[1]  : "boot.fth";
	last = (cell)&vars[VARS_SZ];
	vhere = (cell)&vars[0];
	here = LAST_OP+1;
	base = 10;
	state = 3;
	PRIMS
	addLit("(vh)", (cell)&vhere);
	addLit("(h)", (cell)&here);
	addLit("(l)", (cell)&last);
	addLit("(sp)", (cell)&dsp);
	addLit("(stk)", (cell)&dstk[0]);
	addLit("state", (cell)&state);
	addLit("base", (cell)&base);
	addLit("vars", (cell)&vars[0]);
	addLit("code", (cell)&code[0]);
	addLit("vars-sz", VARS_SZ);
	addLit(">in", (cell)&toIn);
	FILE *fp = fopen(boot_fn, "rb");
	if (fp) {
		fread(&vars[1000], 1, 10000, fp);
		fclose(fp);
		outer((char *)&vars[1000]);
	}
	while (state != 999) {
		zType(" ok\n");
		char* tib = (char*)(vhere + 1024);
		fgets(tib, 256, stdin);
		outer(tib);
	}
	return 0;
}
