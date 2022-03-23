#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define CELL_SZ   4
#define USER_SZ  (2*1024)
#define VARS_SZ  (1*256)
#define STK_SZ    8
#define LSTK_SZ   4

#define TOS  stk[sp]
#define NOS  stk[sp-1]
#define AOS (byte*)stk[sp]
#define LOS  lstk[lsp]
#define DROP2 pop(); pop()
#define U(l) user[l]
#define UA(l) &U(l)
#define DP_AT(l) (DICT_T *)&user[l]

typedef unsigned char byte;
typedef unsigned short WORD;

typedef long CELL;
typedef unsigned long UCELL;

typedef unsigned short USHORT;

typedef struct {
    byte prev;
    byte flags;
    char name[32];
} DICT_T;

typedef struct {
    WORD s, e;
    CELL f, t;
} LOOP_T;

extern WORD PC;
extern char IR, sp, rsp;
extern CELL BASE, STATE, VHERE;
extern WORD LAST, HERE;
extern byte user[];
extern byte vars[];
extern CELL stk[];
extern CELL rstk[];

extern void push(CELL);
extern CELL pop();
extern void SET_WORD(byte *l, WORD v);
extern void SET_LONG(byte *l, long v);
extern void printString(const char*);
extern void printChar(char);
extern void printBase(UCELL, CELL);
extern int strLen(const char *);
extern void run(WORD);

#endif
