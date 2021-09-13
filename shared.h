#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define ADDR_SZ   4
#define CELL_SZ   4
#define CODE_SZ  (1*256)
#define DICT_SZ  (1*1024)
#define VARS_SZ  (1*256)
#define STK_SZ    15

typedef unsigned char byte;
typedef short WORD;

#if CELL_SZ == 2
typedef short CELL;
typedef unsigned short UCELL;
#else
typedef long CELL;
typedef unsigned long UCELL;
#endif

typedef byte *ADDR;

#define NAME_LEN 15

typedef struct {
    CELL xt;
    byte flags;
    char name[NAME_LEN+1];
} dict_t;

#define T  DSTK[DSP]
#define A (ADDR)DSTK[DSP]
#define N  DSTK[DSP-1]
#define R  RSTK[RSP]
#define DP_AT(l) (dict_t *)&DICT[l]
#define STR_AT(l) (char *)&VARS[l]

extern UCELL PC;
extern byte IR, DSP, RSP;
extern CELL BASE, STATE;
extern UCELL HERE, VHERE, LAST;
extern byte CODE[];
extern byte VARS[];
extern byte DICT[];
extern CELL DSTK[];
extern CELL RSTK[];

extern void push(CELL);
extern CELL pop();
extern void SET_WORD(ADDR l, WORD v);
extern void SET_LONG(ADDR l, long v);
extern int  SET_CELL(ADDR l, CELL v);
extern void CComma(CELL v);
extern void Comma(CELL v);


extern int strLen(const char *);
extern CELL getNext(CELL);
extern void run(CELL);

#endif
