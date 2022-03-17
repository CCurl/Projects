// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include "config.h"
#include <stdio.h>
#include <stdarg.h>

typedef long CELL;
typedef unsigned long UCELL;
typedef unsigned short ushort;
typedef unsigned char byte;
typedef byte *addr;

#define CELL_SZ     sizeof(CELL)
#define ADDR_SZ     sizeof(addr)
#define DENTRY_SZ   sizeof(DICT_T)

#define NUM_FUNCS (26*26*26)
#define USER       user
#define VAR        var
// #define HERE       REG[7]
#define T          dstack[dsp]
#define N          dstack[dsp-1]
#define R          rstack[rsp]
#define LSP        lsp
#define DROP1      pop()
#define DROP2      pop(); pop()
#define BetweenI(n, x, y) ((x <= n) && (n <= y))
#define ABS(n) ((n < 0) ? (-n) : (n))

typedef struct {
    addr start;
    CELL from;
    CELL to;
    addr end;
} LOOP_ENTRY_T;


typedef struct{
    ushort func;
    char name[14];
} DICT_T;

extern byte isBye;
extern byte isError;
extern addr HERE;
extern CELL BASE;
extern CELL dstack[];
extern CELL rstack[];
extern ushort dsp, rsp;
extern byte user[];
extern byte var[];
extern addr func[];

extern void vmInit();
extern void forthInit();
extern CELL pop();
extern void push(CELL);
extern addr create(addr);
extern void parse(addr);
extern addr run(addr);
extern addr doCustom(byte, addr);
extern void printChar(const char);
extern void printString(const char*);
extern void printStringF(const char*, ...);
extern void dumpStack();
extern void parse(const char *);
extern void setCell(addr to, CELL val);
extern CELL getCell(addr from);
extern void setWord(addr to, CELL val);
extern CELL getWord(addr from);
extern CELL getSeed();
extern void prim(const char *);

