#ifndef __FORTH_SHARED__
#define __FORTH_SHARED__

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

// #define __VERBOSE__ 1
#define MEM_SZ 8*1024

#ifdef __VERBOSE__
    #define TRACE(...) printf(__VA_ARGS__)
#else
    #define TRACE(...)
#endif

// ************************************************************************************************
// The VM's instruction set
// ************************************************************************************************
#define CLITERAL   '1'	// 0E
#define WLITERAL   '2'  // 01
#define LITERAL    '4'  // 01
#define FETCH      '@'  // 02
#define STORE      '!'  // 03
#define CSTORE     's'  // 10
#define WSTORE     'W'  // 10
#define SWAP       '$'  // 04
#define DROP       '\\' // 05
#define DUP        '#'  // 06
#define SLITERAL    7   // 07
#define JMP         8   // 08
#define JMPZ        9   // 09
#define JMPNZ      10   // 0A
#define CALL       'C'  // 0B
#define RET        ';'  // 0C
#define OR         '|'  // 0D
#define XOR        '^'  // 0D
#define CFETCH     'c'  // 0F
#define ADD        '+'  // 11
#define SUB        '-'  // 12
#define MUL        '*'  // 13
#define DIV        '/'  // 14
#define LT         '<'  // 15
#define EQ         '='  // 16
#define GT         '>'  // 17
#define DICTP      'P'  // 18
#define EMIT       ','  // 19
#define OVER       '%'  // 1A
#define COMPARE    'e'  // 1B  ( addr1 addr2 -- bool )
#define FOPEN      'O' // 1C  ( name mode -- fp status ) - mode: 0 = read, 1 = write
#define FREAD      'r'  // 1D  ( addr num fp -- count ) - fp == 0 means STDIN
#define FREADLINE  'l'  // 1E  ( addr fp -- count )
#define FWRITE     'w'  // 1F  ( addr num fp -- ) - fp == 0 means STDIN
#define FCLOSE     'o'  // 20  ( fp -- )
#define DTOR       15   // 21  >R (Data To Return)
#define RTOD       16   // 22  R> (Return To Data)
#define COM        '~'  // 23
#define AND        '&'  // 24
#define PICK       17   // 25
#define DEPTH      18   // 26
#define GETCH      19   // 27
#define COMPAREI   20   // 28 ( addr1 addr2 -- bool )
#define SLASHMOD   21   // 29
#define NOT        22   // 2A ( val1 -- val2 )
#define RFETCH     23   // 2B ( addr -- val )
#define INC        'i'  // 2C 
#define RDEPTH     24   // 2D
#define DEC        'd'  // 2E 
#define GETTICK    25  // 2F
#define SHIFTLEFT  26  // 30
#define SHIFTRIGHT 27  // 31
#define PLUSSTORE  'S'  // 32
#define OPENBLOCK  28  // 33
#define FOR        '['
#define NEXT       ']'
#define INDEX      'I'
// unused 53-99
#define DBGDOT    222
#define DBGDOTS   223
// unused 102-251
#define NOP       252	// FC
#define BREAK     253	// FD
#define RESET     254	// FE
#define BYE       255	// FF

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
