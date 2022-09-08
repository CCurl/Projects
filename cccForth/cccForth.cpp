#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define PC           1
#define TEENSY4      2
#define XIAO         3
#define ESP32_DEV    4
#define ESP8266      5
#define APPLE_MAC    6
#define PICO         7
#define LINUX        8
#define LEO          9

#define __BOARD__    LINUX

#ifdef _WIN32
  #undef __BOARD__
  #define __BOARD__ PC
  #define __TARGET__   WINDOWS
  #include <Windows.h>
  #include <conio.h>
  #define CODE_SZ      ( 64*1024)
  #define VARS_SZ      (256*1024)
  #define STK_SZ        64
  #define LSTK_SZ       32
  #define LOCALS_SZ    160
  #define DICT_SZ     2000
  #define FLT_SZ        10
  #define __FILES__
#elif __BOARD__ == LINUX
  #undef __BOARD__
  #define __BOARD__ PC
  #define __TARGET__   LINUX
  #include <unistd.h>
  #include <termios.h>
  #define CODE_SZ      ( 64*1024)
  #define VARS_SZ      (256*1024)
  #define STK_SZ        64
  #define LSTK_SZ       32
  #define LOCALS_SZ    160
  #define DICT_SZ     2000
  #define FLT_SZ        10
  #define __FILES__
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#if __BOARD__ == TEENSY4
  #define CODE_SZ      (48*1024)
  #define VARS_SZ      (96*1024)
  #define STK_SZ        64
  #define LSTK_SZ       32
  #define LOCALS_SZ    160
  #define DICT_SZ     1000
  #define FLT_SZ        10
  #define __PIN__
  #define __FILES__
  // #define __EDITOR__
  #define NEEDS_ALIGN
#elif __BOARD__ == PICO
  #define CODE_SZ      (48*1024)
  #define VARS_SZ      (96*1024)
  #define STK_SZ        64
  #define LSTK_SZ       32
  #define LOCALS_SZ    160
  #define DICT_SZ     1000
  #define FLT_SZ        10
  #define __PIN__
  #define __FILES__
  // #define __EDITOR__
  #define NEEDS_ALIGN
#elif __BOARD__ == XIAO
  #define CODE_SZ      (12*1024)
  #define VARS_SZ      (12*1024)
  #define STK_SZ        32
  #define LSTK_SZ       12
  #define LOCALS_SZ     80
  #define DICT_SZ      200
  #define FLT_SZ         8
  #define __PIN__
  #define NEEDS_ALIGN
// #define __GAMEPAD__
#elif __BOARD__ == ESP8266
  #define CODE_SZ      (14*1024)
  #define VARS_SZ      (12*1024)
  #define STK_SZ        32
  #define LSTK_SZ       12
  #define LOCALS_SZ     80
  #define DICT_SZ      100
  #define FLT_SZ         8
  #define __PIN__
  #define NEEDS_ALIGN
#endif

#define TOS           stks[sp]
#define NOS           stks[sp-1]
#define AOS           (byte*)TOS
#define CTOS          (char*)TOS
#define CNOS          (char*)NOS
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define FTOS          fstk[fsp]
#define FNOS          fstk[fsp-1]
#define FDROP         fsp-=(0<fsp)?1:0
#define DROP1         sp--
#define DROP2         sp-=2
#define CA(l)         (st.code+l)
#define DP_AT(l)      ((DICT_T *)(&st.code[l]))
#define BTW(x, a, b)  ((a<=x)&&(x<=b))
#define BA(a)         ((byte *)a)

typedef unsigned char byte;
typedef unsigned short WORD;
typedef long CELL;
typedef unsigned long UCELL;
typedef unsigned short USHORT;

#define CELL_SZ   sizeof(CELL)
#define CSZ       CELL_SZ

#define NAME_LEN 16

typedef struct {
    USHORT xt;
    byte flags;
    byte len;
    char name[NAME_LEN];
} DICT_E;

typedef struct {
    int HERE, VHERE, LAST;
    byte code[CODE_SZ + 4];
    byte vars[VARS_SZ + 4];
    DICT_E dict[DICT_SZ];
} ST_T;

#define BIT_IMMEDIATE 0x80

extern ST_T st;
extern byte sp, isError, isBye;
extern CELL BASE, STATE, tHERE, tVHERE, tempWords[10];
extern CELL stks[];

extern void vmReset();
extern void systemWords();
extern void push(CELL);
extern CELL pop();
extern void SET_WORD(byte *l, WORD v);
extern void SET_LONG(byte *l, long v);
extern void printString(const char*);
extern void printStringF(const char*, ...);
extern void printChar(char);
extern void printBase(CELL, CELL);
extern int strLen(const char *);
extern char *strCpy(char *,const char *);
extern char *strCat(char *,const char *);
extern char *strCatC(char *,const char);
extern int strEq(const char *,const char *);
extern int strEqI(const char *,const char *);
extern char *sprintF(char* dst, const char* fmt, ...);
extern char *rTrim(char *);
extern void run(WORD);
extern int doFind(const char *);
extern void doParse(const char *);
extern void doDotS();
extern void doWords();
extern void doOK();
extern byte *doExt(CELL, byte *);
extern void doEditor();
extern int charAvailable();
extern int getChar();
extern CELL doTimer();
extern void doSleep();

// FILEs
extern byte *doFile(CELL, byte *);
extern void fileInit();
extern void fOpen();
extern void fRead();
extern void fWrite();
extern void fClose();
extern void fDelete();
extern void fList();
extern void fSave();
extern void fLoad();

#endif
byte sp, rsp, lsp, lb, isError, sb, rb, fsp, *y;
CELL BASE, stks[STK_SZ], locals[LOCALS_SZ], lstk[LSTK_SZ+1], seed;
float fstk[FLT_SZ];

ST_T st;

void doEditor() {}

void vmReset() {
    lsp = lb = 0, fsp = 0;
    sb = 2, rb = (STK_SZ-2);
    sp = sb - 1, rsp = rb + 1;
    st.LAST = 0;
    st.HERE = tHERE = 2;
    st.VHERE = tVHERE = 0;
    for (int i = 0; i < CODE_SZ; i++) { st.code[i] = 0; }
    for (int i = 0; i < VARS_SZ; i++) { st.vars[i] = 0; }
    for (int i = 0; i < 10; i++) { tempWords[i] = 0; }
    systemWords();
}

inline void push(CELL v) { stks[++sp] = v; }
inline CELL pop() { return stks[sp--]; }

inline void fpush(float v) { fstk[++fsp] = v; }
inline float fpop() { return fstk[fsp--]; }

inline void rpush(CELL v) { stks[--rsp] = v; }
inline CELL rpop() { return stks[rsp++]; }

#ifdef NEEDS_ALIGN
WORD GET_WORD(byte* l) { return *l | (*(l + 1) << 8); }
long GET_LONG(byte* l) { return GET_WORD(l) | GET_WORD(l + 2) << 16; }
void SET_WORD(byte* l, WORD v) { *l = (v & 0xff); *(l + 1) = (byte)(v >> 8); }
void SET_LONG(byte* l, long v) { SET_WORD(l, v & 0xFFFF); SET_WORD(l + 2, (WORD)(v >> 16)); }
#else
WORD GET_WORD(byte* l) { return *(WORD *)l; }
long GET_LONG(byte* l) { return *(long *)l; }
void SET_WORD(byte* l, WORD v) { *(WORD *)l = v; }
void SET_LONG(byte* l, long v) { *(long *)l = v; }
#endif // NEEDS_ALIGN

void printBase(CELL num, CELL base) {
    UCELL n = (UCELL) num, isNeg = 0;
    if ((base == 10) && (num < 0)) { isNeg = 1; n = -num; }
    char* cp = (char *)&st.vars[VARS_SZ];
    *(cp--) = 0;
    do {
        int x = (n % base) + '0';
        n = n / base;
        *(cp--) = ('9' < x) ? (x+7) : x;
    } while (n);
    if (isNeg) { printChar('-'); }
    printString(cp+1);
}

void doDotS() {
    if (sp<sb) { sp=sb-1; }
    printString("(");
    for (int d = sb; d <= sp; d++) {
        if (sb < d) { printChar(' '); }
        printBase(stks[d], BASE);
    }
    printString(")");
}

byte *doType(byte *a, int l, int delim) {
    if (l < 0) { l = 0; while (a[l]!=delim) { ++l; } }
    byte* e = a+l;
    while (a < e) {
        char c = (char)*(a++);
        if (c == '%') {
            c = *(a++);
            switch (c) {
            case 'b': printBase(pop(), 2);          break;
            case 'c': printChar((char)pop());       break;
            case 'd': printBase(pop(), 10);         break;
            case 'e': printChar(27);                break;
            case 'f': printStringF("%f", fpop());   break;
            case 'g': printStringF("%g", fpop());   break;
            case 'i': printBase(pop(), BASE);       break;
            case 'n': printString("\r\n");          break;
            case 'q': printChar('"');               break;
            case 's': printString((char*)pop());    break;
            case 't': printChar(9);                 break;
            case 'x': printBase(pop(), 16);         break;
            default: printChar(c);                  break;
            }
        } else { printChar(c); }
    }
    if (delim) { ++e; }
    return e;
}

byte* doFile(CELL ir, byte* pc) {
    ir = *(pc++);
    if (ir == 'O') { fOpen(); }
    else if (ir == 'D') { fDelete(); }
    else if (ir == 'L') { fList(); }
    else if (ir == 's') { fSave(); }
    else if (ir == 'l') { fLoad(); pc = 0; }
    else if (TOS == 0) { printString("-nofp-"); return pc; }
    else if (ir == 'R') { fRead(); }
    else if (ir == 'W') { fWrite(); }
    else if (ir == 'C') { fClose(); }
    return pc;
}

CELL doRand() {
    if (seed == 0) { seed = doTimer(); }
    seed ^= (seed << 13);
    seed ^= (seed >> 17);
    seed ^= (seed << 5);
    return seed & 0x7FFFFFFF;
}

void run(WORD start) {
    byte* pc = CA(start);
    CELL t1, t2;
    lsp = isError = 0;
    if (sp < sb) { sp = sb - 1; }
    if (rsp > rb) { rsp = rb + 1; }
    if (fsp < 0) { fsp = 0; }
    if (9 < fsp) { fsp = 9; }
    while (pc) {
        byte ir = *(pc++);
        switch (ir) {
        case 0: return;
        case 1: push(*(pc++));                                               break; // BLIT
        case 2: push(GET_WORD(pc)); pc += 2;                                 break; // WLIT
        case 4: push(GET_LONG(pc)); pc += 4;                                 break; // LIT
        case ' ': /* NOP */                                                  break; // NOP
        case '!': SET_LONG(AOS, NOS); DROP2;                                 break; // STORE
        case '"': pc = doType(pc, -1, '"');                                  break; // ."
        case '#': push(TOS);                                                 break; // DUP
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                             break; // SWAP
        case '%': push(NOS);                                                 break; // OVER
        case '&': t1 = NOS; t2 = TOS; NOS = t1 / t2; TOS = t1 % t2;          break; // /MOD
        case '\'': push(*(pc++));                                            break; // CHAR
        case '(': if (pop() == 0) { while (*pc != ')') { ++pc; } }           break; // Simple IF (NO ELSE)
        case ')': /* Simple THEN */                                          break; // Simple THEN
        case '*': t1 = pop(); TOS *= t1;                                     break; // MULT
        case '+': t1 = pop(); TOS += t1;                                     break; // ADD
        case ',': printChar((char)pop());                                    break; // EMIT
        case '-': t1 = pop(); TOS -= t1;                                     break; // SUB
        case '.': printBase(pop(), BASE);                                    break; // DOT
        case '/': t1 = pop(); TOS /= t1;                                     break; // DIV
        case '0': case '1': case '2': case '3': case '4': case '5':                 // NUMBER
        case '6': case '7': case '8': case '9': push(ir-'0');
            while (BTW(*pc,'0','9')) { TOS = (TOS*10) + *(pc++) - '0'; }     break;
        case ':': if (*(pc+2) != ';') { rpush(pc-st.code+2); }                      // CALL (w/tail-call optimization)
            pc = CA(GET_WORD(pc));                                           break;
        case ';': if (rsp>rb) { pc=0; rsp=rb+1; } else { pc=CA(rpop()); }    break; // RETURN
        case '>': NOS = (NOS > TOS) ? 1 : 0; DROP1;                          break; // >
        case '=': NOS = (NOS == TOS) ? 1 : 0; DROP1;                         break; // =
        case '<': NOS = (NOS < TOS) ? 1 : 0; DROP1;                          break; // <
        case '?': if (pop()==0) { pc=CA(GET_WORD(pc)); } else { pc+=2; }     break; // 0BRANCH
        case '@': TOS = GET_LONG(AOS);                                       break; // FETCH
        case 'C': ir = *(pc++); if (ir=='@') { TOS = *AOS; }
                else if(ir=='!') { *AOS = (byte)NOS; DROP2; }                break; // C@, C!
        case 'D': --TOS;                                                     break; // 1-
        case 'E': rpush(pc-st.code); pc = CA(pop());                         break; // EXECUTE
        case 'F': ir = *(pc++); if (ir=='.') { printStringF("%g",fpop()); }         // FLOAT ops
                else if (ir=='#') { fpush(FTOS); }
                else if (ir=='$') { float x=FTOS; FTOS=FNOS; FNOS=x; }
                else if (ir=='%') { fpush(FNOS); }
                else if (ir=='\\') { FDROP; }
                else if (ir=='i') { fpush((float)pop()); }
                else if (ir=='o') { push((CELL)fpop()); }
                else if (ir=='+') { FNOS+=FTOS; FDROP; }
                else if (ir=='-') { FNOS-=FTOS; FDROP; }
                else if (ir=='*') { FNOS*=FTOS; FDROP; }
                else if (ir=='/') { FNOS/=FTOS; FDROP; }
                else if (ir=='<') { push((FNOS<FTOS)?1:0); FDROP; FDROP; }
                else if (ir=='>') { push((FNOS>FTOS)?1:0); FDROP; FDROP; }   break;
        case 'G': pc = CA(GET_WORD(pc));                                     break; // BRANCH
        case 'I': push(L0);                                                  break; // I
        case 'J': t1 = (lsp>2) ? lsp-3 : 0; push(lstk[t1]);                  break; // J
        case 'K': ir = *(pc++); if (ir=='@') { push(getChar()); }                   // KEY?, KEY
                else if(ir=='?') { push(charAvailable()); }                  break;
        case 'M': L0 += pop();                                               break; // +I
        case 'P': ++TOS;                                                     break; // 1+
        case 'R': ir = *(pc++); if (ir == '<') { rpush(pop()); }                    // >R, R@, R>
                if (ir == '>') { push(rpop()); }
                if (ir == '@') { push(stks[rsp]); }                          break;
        case 'S': ir = *(pc++); if (ir == 'e') { TOS += strLen(CTOS); }             // STR-END
                else if (ir == 'a') { strCat(CTOS, CNOS); DROP2; }                  // STR-CAT
                else if (ir == 'c') { strCatC(CTOS, (char)NOS); DROP2; }            // STR-CATC
                else if (ir == '=') { NOS = strEq(CTOS, CNOS); DROP1; }             // STR-EQ
                else if (ir == 'i') { NOS = strEqI(CTOS, CNOS); DROP1; }            // STR-EQI
                else if (ir == 'l') { TOS = (CELL)strLen(CTOS); }                   // STR-LEN
                else if (ir == 'r') { TOS = (CELL)rTrim(CTOS); }                    // STR-RTRIM
                else if (ir == 't') { *CTOS = 0; }                                  // STR-TRUNC
                else if (ir == 'y') { strCpy(CTOS, CNOS); DROP2; }           break; // STR-CPY
        case 'T': t1=pop(); y=(byte*)pop(); while (t1--) printChar(*(y++));  break; // TYPE (a c--)
        case 'Y': vmReset();                                                return; // RESET
        case 'Z': doType((byte *)pop(),-1, 0);                               break; // ZTYPE
        case '[': lsp += 3; L2 = (CELL)pc;                                          // FOR (f t--)
            L1 = (TOS > NOS) ? TOS : NOS;
            L0 = (TOS < NOS) ? TOS : NOS; DROP2;                             break;
        case '\\': DROP1;                                                    break; // DROP
        case ']': ++L0; if (L0<L1) { pc=(byte*)L2; } else { lsp-=3; }        break; // NEXT
        case '^': ir=*(pc++); lsp-=(ir=='F')?3:1; if (lsp<0) { lsp=0; }      break; // UNLOOP, UNLOOP-W
        case '_': TOS = -TOS;                                                break; // NEGATE
        case '`': push((CELL)pc); while (*(pc++)) {}                         break; // ZQUOTE
        case 'b': ir = *(pc++); if (ir == '~') { TOS = ~TOS; }                      // BINARY ops
                else if (ir == '%') { NOS %= TOS; DROP1; }
                else if (ir == '&') { NOS &= TOS; DROP1; }
                else if (ir == '^') { NOS ^= TOS; DROP1; }
                else if (ir == '|') { NOS |= TOS; DROP1; }
                else if (ir == 'L') { NOS = (NOS << TOS); DROP1; }
                else if (ir == 'R') { NOS = (NOS >> TOS); DROP1; }
                else { --pc; printChar(32); }                                break;
        case 'c': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { locals[lb+t1]+=CSZ; }   break; // incLocal-CELL
        case 'd': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { --locals[lb+t1]; }      break; // decLocal
        case 'i': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { ++locals[lb+t1]; }      break; // incLocal
        case 'f': pc = doFile(ir, pc);                                       break; // FILE ops
        case 'l': if ((lb+10)<LOCALS_SZ) { lb+=10; }                         break; // +TMPS
        case 'm': if (lb>9) { lb-=10; }                                      break; // -TMPS
        case 'r': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { push(locals[lb+t1]); }  break; // readLocal
        case 's': t1=*(pc++)-'0'; if (BTW(t1,0,9)) { locals[lb+t1]=pop(); }  break; // setLocal
        case 't': printString((char *)pop());                                break; // QTYPE
        case 'v': t1=GET_LONG(pc); pc+=4; push((CELL)&st.vars[t1]);          break; // VAR-ADDR
        case 'w': ir = *(pc++); if (ir == '@') { TOS = GET_WORD(AOS); }
                else if (ir == '!') { SET_WORD(AOS, (WORD)NOS); DROP2; }     break; // w@, w!
        case 'x': ir = *(pc++); if (ir==']') { t1 = L0; L0 += pop();                // +LOOP
                    if ((t1 < L1) && (L0 < L1)) { pc = (byte*)(L2); }
                    else if ((t1 > L1) && (L0 > L1)) { pc = (byte*)(L2); }
                    else { lsp -= 3; } }
                else if (ir=='[') { lsp += 3; L0=pop(); L1=pop(); L2=(CELL)pc; }    // DO
                else if (ir=='S') { doDotS(); }                                     // .S
                else if (ir=='R') { push(doRand()); }                               // RAND
                else if (ir=='A') { st.VHERE+=pop(); tVHERE=st.VHERE; }             // ALLOT
                else if (ir=='T') { push(doTimer()); }                              // TIMER
                else if (ir=='Y') { y=(byte*)pop(); t2=system((char*)y); }          // SYSTEM
                else if (ir=='D') { doWords(); }                                    // WORDS
                else if (ir=='W') { doSleep(); }                                    // MS
                else if (ir=='Q') { isBye=1; return; }                       break; // BYE
        case 'z': pc = doExt(*pc, pc+1);                                     break; // EXT
        case '{': ++lsp; L0=(CELL)pc;                                        break; // BEGIN
        case '}': if (pop()) { pc=(byte*)L0; } else { lsp--; }               break; // WHILE
        case '~': TOS = (TOS) ? 0 : 1;                                       break; // NOT (0=)
        default: printStringF("-unk ir: %d (%c)-", ir, ir);                 return;
        }
    }
}

// cccForth.cpp : A minimal and memory conscious Forth interpreter

// #include "shared.h"

typedef struct {
    const char *name;
    const char *op;
} PRIM_T;

// Words that directly map to VM operations
PRIM_T prims[] = {
    // Stack
    { "DROP", "\\" }
    , { "DUP", "#" }
    , { "OVER", "%" }
    , { "SWAP", "$" }
    , { "NIP", "$\\" }
    , { "1+", "P" }
    , { "1-", "D" }
    , { "2+", "PP" }
    , { "2*", "#+" }
    , { "2/", "2/" }
    , { "2DUP", "%%" }
    , { "2DROP", "\\\\" }
    , { "ROT", "Q<$Q>$" }
    , { "-ROT", "$Q<$Q>" }
    , { "TUCK", "$%" }
    // Memory
    , { "@", "@" }
    , { "C@", "C@" }
    , { "W@", "w@" }
    , { "!", "!" }
    , { "C!", "C!" }
    , { "W!", "w!" }
    , { "+!", "$%@+$!" }
    // Math
    , { "+", "+" }
    , { "-", "-" }
    , { "/", "/" }
    , { "*", "*" }
    , { "ABS", "#0<(_)" }
    , { "/MOD", "&" }
    , { "MOD", "b%" }
    , { "NEGATE", "_" }
    // Input/output
    , { "(.)", "." }
    , { ".", ".32," }
    , { "CR", "13,10," }
    , { "EMIT", "," }
    , { "KEY", "K@" }
    , { "KEY?", "K?" }
    , { "LOAD", "zL"}
    , { "QTYPE", "t" }
    , { "ZTYPE", "Z" }
    , { "COUNT", "#Sl" }
    , { "TYPE", "T" }
    , { "SPACE", "32," }
    , { "SPACES", "0[32,]" }
    // Logical / flow control
    , { ".IF", "(" }
    , { ".THEN", ")" }
    , { "FOR", "[" }
    , { "NEXT", "]" }
    , { "DO", "x[" }
    , { "LOOP", "]" }
    , { "+LOOP", "x]" }
    , { "I", "I" }
    , { "J", "J" }
    , { "+I", "M" }
    , { "UNLOOP", "^F" }
    , { "UNLOOP-F", "^F" }
    , { "BEGIN", "{" }
    , { "WHILE", "}" }
    , { "UNTIL", "~}" }
    , { "AGAIN", "1}" }
    , { "UNLOOP-W", "^W" }
    , { "TRUE", "1" }
    , { "FALSE", "0" }
    , { "=", "=" }
    , { "<", "<" }
    , { ">", ">" }
    , { "<=", ">~" }
    , { ">=", "<~" }
    , { "<>", "=~" }
    , { "!=", "=~" }
    , { "0=", "~" }
    , { "EXIT", ";" }
    // String
    , { "STR-LEN", "Sl" }
    , { "STR-END", "Se" }
    , { "STR-CAT", "Sa" }
    , { "STR-CATC", "Sc" }
    , { "STR-CPY", "Sy" }
    , { "STR-EQ", "S=" }
    , { "STR-EQI", "Si" }
    , { "STR-TRUNC", "St" }
    , { "STR-RTRIM", "Sr" }
    // Binary/bitwise
    , { "AND", "b&" }
    , { "OR", "b|" }
    , { "XOR", "b^" }
    , { "INVERT", "b~" }
    , { "LSHIFT", "bL" }
    , { "RSHIFT", "bR" }
    // Float
    , { "I>F", "Fi" }  // In
    , { "F>I", "Fo" }  // Out
    , { "F+", "F+" }  // Add
    , { "F-", "F-" }  // Sub
    , { "F*", "F*" }  // Mult
    , { "F/", "F/" }  // Div
    , { "F<", "F<" }  // LT
    , { "F>", "F>" }  // GT
    , { "F.", "F." }  // PRINT
    , { "FDUP", "F#" }
    , { "FOVER", "F%" }
    , { "FSWAP", "F$" }
    , { "FDROP", "F\\" }
    // System
    , { "ALLOT", "xA" }
    , { "BL", "32" }
    , { "BYE", "xQ" }
    , { "CELL", "4" }
    , { "CELLS", "4*" }
    , { "CELL+", "4+" }
    , { "EXECUTE", "E" }
    , { "MAX", "%%<($)\\" }
    , { "MIN", "%%>($)\\" }
    , { "MS", "xW" }
    , { "NOP", " " }
    , { "NOT", "~" }
    , { ">R", "R<" }
    , { "R>", "R>" }
    , { "R@", "R@" }
    , { "RAND", "xR" }
    , { "RESET", "Y" }
    , { ".S", "xS" }
    , { "SYSTEM", "xY" }
    , { "TIMER", "xT" }
    , { "+TMPS", "l" }
    , { "-TMPS", "m" }
    , { "WORDS", "xD" }
#ifdef __FILES__
    // Extension: FILE operations
    , { "FOPEN", "fO" }
    , { "FGETC", "fR" }
    , { "FPUTC", "fW" }
    , { "FCLOSE", "fC" }
    , { "FDELETE", "fD" }
    , { "FLIST", "fL" }
    , { "FSAVE", "fs" }
    , { "FLOAD", "fl" }
#endif
#ifdef __PIN__
    // Extension: PIN operations ... for dev boards
    , { "pin-in","zPI" }          // open input
    , { "pin-out","zPO" }         // open output
    , { "pin-up","zPU" }          // open input-pullup
    , { "pin!","zPWD" }           // Pin write: digital
    , { "pin@","zPRD" }           // Pin read: digital
    , { "pina!","zPWA" }          // Pin write: analog
    , { "pina@","zPRA" }          // Pin read: analog
#endif
#ifdef __EDITOR__
    // Extension: A simple block editor
    , { "EDIT","zE" }         // |EDIT|zE|(n--)|Edit block n|
#endif
#ifdef __GAMEPAD__
    // Extension: GAMEPAD operations
    , { "gp-button","xGB" }
#endif
    , {0,0}
};

char word[32], *in;
CELL STATE, tHERE, tVHERE, tempWords[10];
byte isBye=0;

void CComma(CELL v) { st.code[tHERE++] = (byte)v; }
void Comma(CELL v) { SET_LONG(&st.code[tHERE], v); tHERE += CELL_SZ; }
void WComma(WORD v) { SET_WORD(&st.code[tHERE], v); tHERE += 2; }

char lower(char c) { return BTW(c, 'A', 'Z') ? (c + 32) : c; }

int strLen(const char* str) {
    int l = 0;;
    while (*(str++)) { ++l; }
    return l;
}

int strEq(const char *x, const char *y) {
    while (*x && *y && (*x == *y)) { ++x; ++y; }
    return (*x || *y) ? 0 : 1;
}

int strEqI(const char *x, const char *y) {
    while (*x && *y) {
        if (lower(*x) != lower(*y)) { return 0; }
        ++x; ++y;
    }
    return (*x || *y) ? 0 : 1;
}

char *strCpy(char *d, const char *s) {
    char *x = d;
    while (*s) { *(x++) = *(s++); }
    *x = 0;
    return d;
}

char *strCat(char *d, const char *s) {
    char *x = d+strLen(d);
    strCpy(x, s);
    return d;
}

char *strCatC(char *d, char c) {
    char *x = d+strLen(d);
    *x = c;
    *(x+1) = 0;
    return d;
}

char *rTrim(char *d) {
    char *x = d+strLen(d);
    while ((d<=x) && (*x<=' ')) { *(x--) = 0; }
    return d;
}

void printStringF(const char *fmt, ...) {
    char *buf = (char*)&st.vars[VARS_SZ-100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 100, fmt, args);
    va_end(args);
    printString(buf);
}

void doExec() {
    if (STATE) {
        st.HERE = tHERE;
        st.VHERE = tVHERE;
    }
    else {
        CComma(0);
        run((WORD)st.HERE);
        tHERE = st.HERE;
        tVHERE = st.VHERE;
    }
}

int isTempWord(const char *nm) {
    return ((nm[0] == 'T') && BTW(nm[1], '0', '9') && (nm[2] == 0));
}

void doCreate(const char *name, byte f) {
    doExec();
    if (isTempWord(name)) {
        tempWords[name[1]-'0'] = tHERE;
        STATE = 1;
        return;
    }
    DICT_E *dp = &st.dict[st.LAST];
    dp->xt = st.HERE;
    dp->flags = f;
    strCpy(dp->name, name);
    dp->name[NAME_LEN-1] = 0;
    dp->len = strLen(dp->name);
    STATE = 1;
    ++st.LAST;
}

int doFindInternal(const char* name) {
    // Regular lookup
    int len = strLen(name);
    for (int i = st.LAST - 1; i >= 0; i--) {
        DICT_E* dp = &st.dict[i];
        if ((len == dp->len) && strEq(dp->name, name)) {
            return i;
        }
    }
    return -1;
}

int doFind(const char *name) {
    // Temporary word?
    if (isTempWord(name) && (tempWords[name[1]-'0'])) {
        push(tempWords[name[1]-'0']);
        push(0);
        return 1;
    }

    // Regular lookup
    int i = doFindInternal(name);
    if (0 <= i) {
        push(st.dict[i].xt);
        push(st.dict[i].flags);
        return 1;
    }
    return 0;
}

int doSee(const char* wd) {
    int i = doFindInternal(wd);
    if (i<0) { printString("-nf-"); return 1; }
    CELL start = st.dict[i].xt;
    CELL end = st.HERE;
    if ((i+1) < st.LAST) { end = st.dict[i + 1].xt; }

    printStringF("%s (%d): ", wd, start);
    for (int i = start; i < end; i++) {
        byte c = st.code[i];
        if (BTW(c, 32, 126)) { printChar(c); }
        else { printStringF("(%d)",c); }
    }
    return 1;
}

void doWords() {
    int n = 0;
    for (int i = st.LAST-1; i >= 0; i--) {
        DICT_E* dp = &st.dict[i];
        printString(dp->name);
        if ((++n) % 10 == 0) { printChar('\n'); }
        else { printChar(9); }
    }
}

int getWord(char *wd) {
    while (*in && (*in < 33)) { ++in; }
    int l = 0;
    while (*in && (32 < *in)) {
        *(wd++) = *(in++);
        ++l;
    }
    *wd = 0;
    return l;
}

int doNumber(int t) {
    CELL num = pop();
    if (t == 'v') {
        CComma('v');
        Comma(num);
    } else if (t == 4) {
        CComma(4);
        Comma(num);
    } else if ((num & 0xFF) == num) {
        CComma(1);
        CComma(num);
    } else if ((num & 0xFFFF) == num) {
        CComma(2);
        WComma((WORD)num);
    } else {
        CComma(4);
        Comma(num);
    }
    return 1;
}

char *sprintF(char *dst, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(dst, 256, fmt, args);
    va_end(args);
    return dst;
}

int isNum(const char *wd) {
    CELL x = 0;
    int base = BASE, isNeg = 0, lastCh = '9';
    if ((wd[0]=='\'') && (wd[2]==wd[0]) && (wd[3]==0)) { push(wd[1]); return 1; }
    if (*wd == '#') { base = 10;  ++wd; }
    if (*wd == '$') { base = 16;  ++wd; }
    if (*wd == '%') { base = 2;  ++wd; lastCh = '1'; }
    if (base < 10) { lastCh = '0' + base - 1; }
    if ((*wd == '-') && (base == 10)) { isNeg = 1;  ++wd; }
    if (*wd == 0) { return 0; }
    while (*wd) {
        char c = *(wd++);
        int t = -1;
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((base == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((base == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x * base) + t;
    }
    if (isNeg) { x = -x; }
    push(x);
    return 1;
}

char *isRegOp(const char *wd) {
    if ((wd[0] == 'r') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    if ((wd[0] == 's') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    if ((wd[0] == 'i') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    if ((wd[0] == 'd') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    if ((wd[0] == 'c') && BTW(wd[1], '0', '9') && (!wd[2])) { return (char*)wd; }
    return 0;
}

int doPrim(const char *wd) {
    // Words cccForth can map directly into its VML (Virtual Machine Language)
    const char *vml = isRegOp(wd);

    for (int i = 0; prims[i].op && (!vml); i++) {
        if (strEqI(prims[i].name, wd)) { vml = prims[i].op; }
    }

    if (!vml) { return 0; } // Not found

    if (BTW(vml[0],'0','9') && BTW(st.code[tHERE-1],'0','9')) { CComma(' '); }
    for (int j = 0; vml[j]; j++) { CComma(vml[j]); }
    return 1;
}

int doQuote() {
    in++;
    CComma('`');
    while (*in && (*in != '"')) { CComma(*(in++)); }
    CComma(0);
    if (*in) { ++in; }
    return 1;
}

int doDotQuote() {
    ++in;
    CComma('"');
    while (*in && (*in != '"')) { CComma(*(in++)); }
    CComma('"');
    if (*in) { ++in; }
    return 1;
}

int doWord() {
    CELL flg = pop();
    CELL xt = pop();
    if (flg & BIT_IMMEDIATE) {
        doExec();
        run((WORD)xt);
    } else {
        CComma(':');
        WComma((WORD)xt);
    }
    return 1;
}

int doParseWord(char *wd) {
    if (strEq(word, "//")) { doExec(); return 0; }
    if (strEq(word, "\\")) { doExec(); return 0; }
    if (doPrim(wd))        { return 1; }
    if (doFind(wd))        { return doWord(); }
    if (isNum(wd))         { return doNumber(0); }
    if (strEq(wd, ".\""))  { return doDotQuote(); }
    if (strEq(wd, "\""))   { return doQuote(); }

    if (strEq(wd, ":")) {
        doExec();
        if (getWord(wd) == 0) { return 0; }
        doCreate(wd, 0);
        return 1;
    }

    if (strEq(wd, ";")) {
        CComma(';');
        doExec();
        STATE = 0;
        return 1;
    }

    if (strEq(wd, "(")) {
        while (*in && (*in != ')')) { ++in; }
        if (*in == ')') { ++in; }
        return 1;
    }

    if (strEqI(wd, "IF")) {
        CComma('?');
        push(tHERE);
        WComma(0);
        return 1;
    }

    if (strEqI(wd, "ELSE")) {
        CELL tgt = pop();
        CComma('G');
        push(tHERE);
        WComma(0);
        SET_WORD(CA(tgt), (WORD)tHERE);
        return 1;
    }

    if (strEqI(wd, "THEN")) {
        CELL tgt = pop();
        SET_WORD(CA(tgt), (WORD)tHERE);
        return 1;
    }

    if (strEqI(wd, "VARIABLE")) {
        if (getWord(wd)) {
            push((CELL)st.VHERE);
            st.VHERE += CELL_SZ;
            tVHERE = st.VHERE;
            doCreate(wd, 0);
            doNumber('v');
            CComma(';');
            doExec();
            STATE = 0;
            return 1;
        }
        else { return 0; }
    }

    if (strEqI(wd, "CONSTANT")) {
        if (getWord(wd)) {
            doCreate(wd, 0);
            doNumber(4);
            CComma(';');
            doExec();
            STATE = 0;
            return 1;
        }
        else { return 0; }
    }

    if (strEqI(wd, "'")) {
        doExec();
        if (getWord(wd) == 0) { return 0; }
        push(doFind(wd));
        return 1;
    }

    if (strEqI(wd, "FORGET")) {
        // Forget the last word
        st.HERE = tHERE = st.dict[st.LAST].xt;
        --st.LAST;
        return 1;
    }

    if (strEqI(wd, "SEE")) {
        doExec();
        if (getWord(wd) == 0) { return 0; }
        return doSee(wd);
    }

    printStringF("[%s]??", wd);
    if (STATE == 1) { STATE = 0; --st.LAST; }
    tHERE = st.HERE;
    tVHERE = st.VHERE;
    st.code[tHERE] = 0;
    return 0;
}

bool isASM(const char* ln) {
    if ((ln[0]=='s') && (ln[1]==':') && (ln[2]==' ')) {
        run((byte*)ln-st.code+3);
        return 1;
    }
    return 0;
}

void doParse(const char *line) {
    in = (char*)line;
    if (isASM(line)) { return; }
    while (getWord(word)) {
        if (tHERE < st.HERE) { tHERE = st.HERE; }
        if (tVHERE < st.VHERE) { tVHERE = st.VHERE; }
        if (doParseWord(word) == 0) { return; }
    }
    doExec();
}

void doOK() {
    if (STATE) { printString(" ... "); return; }
    printString("\r\nOK ");
    doDotS();
    printString("> ");
}

void systemWords() {
    BASE = 10;
    char *cp = (char*)(&st.vars[VARS_SZ-32]);
    sprintF(cp, ": cb %lu ;", (UCELL)st.code);     doParse(cp);
    sprintF(cp, ": vb %lu ;", (UCELL)st.vars);     doParse(cp);
    sprintF(cp, ": db %lu ;", (UCELL)st.dict);     doParse(cp);
    sprintF(cp, ": csz %d ;", CODE_SZ);            doParse(cp);
    sprintF(cp, ": vsz %d ;", VARS_SZ);            doParse(cp);
    sprintF(cp, ": dsz %d ;", DICT_SZ);            doParse(cp);
    sprintF(cp, ": ha %lu ;", (UCELL)&st.HERE);    doParse(cp);
    sprintF(cp, ": la %lu ;", (UCELL)&st.LAST);    doParse(cp);
    sprintF(cp, ": va %lu ;", (UCELL)&st.VHERE);   doParse(cp);
    sprintF(cp, ": base %lu ;", (UCELL)&BASE);     doParse(cp);
    sprintF(cp, ": >in %lu ;", (UCELL)&in);        doParse(cp);
}

#if __BOARD__ == PC
// Support for the PC
// NOTE: this is a .inc file because the Arduino IDE only knows about .CPP and .H files
// If it were a .CPP file, it would need a #if around the entire thing.
// Making it a .INC file avoids that need.

FILE *input_fp = NULL;
FILE *fpStk[10];
byte fpSP = 0;

CELL doTimer() { return clock(); }
#if __TARGET__ == LINUX

void changeMode(int mode)
{
    static struct termios oldt, newt;
    static int cmInit = 0;
    if (cmInit == 0) {
        cmInit = 1;
        tcgetattr( STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~( ICANON | ECHO );
    }

    if (mode) {
        tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    }
}

int charAvailable(void)
{
    struct timeval tv;
    fd_set rdfs;

    changeMode(1);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    int x = FD_ISSET(STDIN_FILENO, &rdfs);
    changeMode(0);
    return x;
}
int getChar(void) {
    return getchar();
}
void doSleep() { usleep(pop()); }

#else // WINDOWS
int charAvailable(void) { return _kbhit(); }
int getChar(void) { return _getch(); }
void doSleep() { Sleep(pop()); }
#endif

void fpPush(FILE *v) { if (fpSP < 9) { fpStk[++fpSP] = v; } }
FILE *fpPop() { return (fpSP) ? fpStk[fpSP--] : 0 ; }

FILE *myFopen(const char *fn, const char *md) {
    FILE *fp;
#ifdef _WIN32
    fopen_s(&fp, fn, md);
#else
    fp = fopen(fn, md);
#endif
    return fp;
}

void fOpen() {
    CELL mode = pop();
    char *fn = (char *)pop();
    FILE *fp = myFopen(fn, mode ? "wb" : "rb");
    push((CELL)fp);
}

void fClose() {
    CELL fp = pop();
    if (fp) { fclose((FILE *)fp); }
}

void fRead() {
    FILE *fp = (FILE *)pop();
    push(0); push(0);
    if (fp && (!feof(fp))) {
        NOS = fgetc(fp);
        TOS = 1;
    }
}

void fWrite() {
    FILE *fp = (FILE *)pop();
    CELL val = pop();
    if (fp) { fputc(val, fp); }
}

void fDelete() {
}

void fList() {
}

void fSave() {
    FILE *fp = myFopen("system.ccc", "wb");
    if (fp) {
        fwrite(&st, sizeof(st), 1, fp);
        fclose(fp);
        printString("-saved-");
    } else { printString("-error-"); }
}

void fLoad() {
    FILE *fp = myFopen("system.ccc", "rb");
    if (fp) {
        vmReset();
        int x=fread(&st, sizeof(st), 1, fp);
        fclose(fp);
        printString("-loaded-");
    } else { printString("-error-"); }
}

void doLoad(int blk) {
    char *fn = (char*)(&st.vars[VARS_SZ-32]);
    sprintF(fn, "./block-%03d.4th", blk);
    FILE *fp = myFopen(fn, "rt");
    if (fp) {
        if (input_fp) { fpPush(input_fp); }
        input_fp = fp;
    }
}

byte *doExt(CELL ir, byte *pc) {
    switch (ir) {
    case 'E': doEditor();                       break;
    case 'F': return doFile(ir, pc);
    case 'L': doLoad(pop());                    break;
    case 'T': push(clock());                    break;
    default: printString("-unk ext-");
    }
    return pc;
}

void printString(const char *cp) { fputs(cp, stdout); }
void printChar(char c) { putc(c, stdout); }

void doHistory(const char *txt) {
    FILE* fp = myFopen("history.txt", "at");
    if (fp) {
        fputs(txt, fp);
        fclose(fp);
    }
}

void loop() {
    char *tib = (char *)CA(tHERE+32);
    FILE* fp = (input_fp) ? input_fp : stdin;
    if (input_fp) {
        if (fgets(tib, 128, fp) == tib) {
            doParse(rTrim(tib));
        } else {
            fclose(input_fp);
            input_fp = fpPop();
        }
    } else {
        doOK();
        char *x=fgets(tib, 128, stdin);
        doHistory(tib);
        doParse(rTrim(tib));
    }
}

int main(int argc, char *argv[]) {
    if (sizeof(&st.HERE) > CELL_SZ) {
        printf("ERROR: CELL cannot support a pointer!");
        exit(1);
    }
    vmReset();
    if (argc > 1) { input_fp = myFopen(argv[1], "rt"); }
    else { doLoad(0); }
    while (!isBye) { loop(); }
    return 0;
}
#endif
