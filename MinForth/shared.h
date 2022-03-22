#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define ADDR_SZ   4
// #define ADDR_SZ 2
#define U(l) user[l]
#define UA(l) &U(l)

#define CELL_SZ   4
#define USER_SZ  (2*1024)
#define VARS_SZ  (1*256)
#define STK_SZ    8

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
#define DP_AT(l) (DICT_T *)&user[l]

#define TOS  DSTK[DSP]
#define AOS (byte*)DSTK[DSP]
#define NOS  DSTK[DSP-1]
#define R  RSTK[RSP]
#define STR_AT(l) (char *)&VARS[l]

extern WORD PC;
extern char IR, DSP, RSP;
extern CELL BASE, STATE, VHERE;
extern WORD LAST, HERE;
extern byte user[];
extern byte VARS[];
extern CELL DSTK[];
extern CELL RSTK[];

extern void push(CELL);
extern CELL pop();
extern void SET_WORD(byte *l, WORD v);
extern void SET_LONG(byte *l, long v);
//extern void CComma(CELL v);
//extern void Comma(CELL v);


extern int strLen(const char *);
extern void run(WORD);

#endif
