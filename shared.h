#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define ADDR_SZ   4
#define CELL_SZ   4
#define STK_SZ    7

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
    ADDR next;
    ADDR xt;
    byte flags;
    char name[NAME_LEN+1];
} dict_t;

#define A (ADDR)DSTK[DSP]

extern ADDR PC;
extern byte IR, DSP, RSP;
extern CELL BASE, STATE;
extern ADDR HERE, VHERE;
extern dict_t *LAST;
extern CELL DSTK[];
extern ADDR RSTK[];

extern void push(CELL);
extern CELL pop();
extern void SET_WORD(ADDR l, WORD v);
extern void SET_LONG(ADDR l, long v);
extern int  SET_CELL(ADDR l, CELL v);
extern void CComma(CELL v);
extern void Comma(CELL v);

extern int strLen(const char *);
extern dict_t *getNext(dict_t *);
extern void run(ADDR);
extern void vm_init(ADDR, UCELL);
#endif
