// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include <stdio.h>
#include <stdarg.h>

#define STK_SZ        8
#define LSTACK_SZ     4
#define USER_SZ     (32*1024)
#define NUM_REGS     52
#define NUM_FUNCS    26
#define VARS_OFFSET (16*1024)

#define CELL        long
#define UCELL       unsigned CELL
#define CELL_SZ     4
#define ushort      unsigned short
#define byte        unsigned char
#define addr        byte *

#define REG        sys.reg
#define USER       sys.user
#define FUNC       sys.func
#define HERE       REG[33]  // 26 + 'h'-'a'
#define VHERE      REG[47]  // 26 + 'v'-'a'
#define INDEX_I    REG[34]  // 26 + 'i'-'a'
#define INDEX_J    REG[35]  // 26 + 'j'-'a'
#define T          sys.dstack[sys.dsp]
#define N          sys.dstack[sys.dsp-1]
#define R          sys.rstack[sys.rsp]
#define LSP        sys.lsp
#define DROP1      pop()
#define DROP2      pop(); pop()
#define BetweenI(n, x, y) ((x <= n) && (n <= y))
#define NEXT       goto next
#define NCASE      NEXT; case
#define RCASE      return; case

typedef struct {
    addr start;
    CELL from;
    CELL to;
    addr end;
} LOOP_ENTRY_T;

typedef struct {
    ushort dsp, rsp, lsp;
    CELL   reg[NUM_REGS];
    byte   user[USER_SZ];
    addr   func[NUM_FUNCS];
    CELL   dstack[STK_SZ + 1];
    addr   rstack[STK_SZ + 1];
    LOOP_ENTRY_T lstack[LSTACK_SZ];
} SYS_T;

extern SYS_T sys;
extern byte isBye;
extern byte isError;

extern void vmInit();
extern CELL pop();
extern void push(CELL);
extern addr run(addr);
extern addr doCustom(byte, addr);
extern void printChar(const char);
extern void printString(const char*);
extern void printStringF(const char*, ...);
extern void dumpStack();
