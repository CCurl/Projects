#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#include <stdio.h>

// #define __VERBOSE__ 1
#define MEM_SZ 64*1024

#ifdef __VERBOSE__
    #define TRACE(...) printf(__VA_ARGS__)
#else
    #define TRACE(...)
#endif

// ************************************************************************************************
// The VM's instruction set
// ************************************************************************************************
enum {
    CLITERAL,
    WLITERAL,
    LITERAL,
    FETCH,
    STORE,
    CSTORE,
    WSTORE,
    SWAP,
    DROP,
    DUP,
    SLITERAL,
    JMP,
    JMPZ,
    JMPNZ,
    CALL,
    RET,
    OR,
    XOR,
    CFETCH,
    ADD,
    SUB,
    MUL,
    DIV,
    LT,
    EQ,
    GT,
    DICTP,
    EMIT,
    OVER,
    COMPARE,
    FOPEN,
    FREAD,
    FREADLINE,
    FWRITE,
    FCLOSE,
    DTOR,
    RTOD,
    COM,
    AND,
    PICK,
    DEPTH,
    GETCH,
    COMPAREI,
    SLASHMOD,
    NOT,
    RFETCH,
    INC,
    RDEPTH,
    DEC,
    GETTICK,
    SHIFTLEFT,
    SHIFTRIGHT,
    PLUSSTORE,
    OPENBLOCK,
    FOR,
    NEXT,
    INDEX,
    DBGDOT,
    DBGDOTS,
    NOP,
    BREAK,
    RESET,
    BYE
};

// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************

typedef unsigned char BYTE;
typedef long CELL;				// Use long for a 32-bit implementation, short for a 16-bit
// #define CELL long;				// Use long for a 32-bit implementation, short for a 16-bit
typedef int bool;
typedef char *String;
typedef short SHORT;
typedef unsigned short ADDR;
typedef unsigned short USHORT;

typedef struct {
    char *asm_instr;
    BYTE opcode;
    char *forth_prim;
    CELL flags;
} OPCODE_T;

// flags is a bit field:
#define IS_IMMEDIATE 0x01
#define IS_INLINE    0x02
#define IS_HIDDEN    0x04

typedef struct {
    ADDR next, XT;
    BYTE flags;
    BYTE len;
    char name[30];
} DICT_T;

#undef NULL
#define NULL (0)
#define CELL_SZ (sizeof(CELL))
#define WORD_SZ (sizeof(USHORT))
#define ADDR_SZ (sizeof(USHORT))

#define DSTACK_SZ 64
#define RSTACK_SZ 64

extern CELL ADDR_CELL;     // 0x08
extern CELL ADDR_HERE;     // 0x10
extern CELL ADDR_LAST;     // 0x14
extern CELL ADDR_BASE;     // 0x18
extern CELL ADDR_STATE;    // 0x20
extern CELL ADDR_MEM_SZ;   // 0x24

#define CELL_AT(loc) *(CELL *)(&the_memory[loc])
#define BYTE_AT(loc) (the_memory[loc])
#define WORD_AT(loc) *(USHORT *)(&the_memory[loc])
#define ADDR_AT(loc) *(ADDR *)(&the_memory[loc])
#define betw(a,b,c) ((b<=a)&&(a<=c))

#define true 1
#define false 0

#define LPCTSTR char *

typedef struct {
    int addr;
    char tag[24];
    char code[128];
    char comment[128];
} LINE_T;

#endif
