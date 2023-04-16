#ifndef __CF_H__
#define __CF_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdarg.h>

typedef int CELL;
typedef unsigned short ushort;
typedef unsigned char byte;

#define RED      1 // Define Word
#define GREEN    2 // Compile
#define YELLOW   3 // Immediate
#define BLUE     4 // Machine code
#define PURPLE   5 // 
#define CYAN     6 // 
#define WHITE    7 // Comment

#define COMMENT  WHITE
#define COMPILE  GREEN
#define DEFINE   RED
#define INTERP   YELLOW
#define ASM      CYAN

#define STK_SZ     15
#define USER_SZ    64*1024
#define CELL_SZ    sizeof(CELL)

#define betw(x, y, z) ((y <= x) && (x <= z))

typedef struct {
    ushort xt;
    byte sz;
    char name[32];
} DICT_T;

extern char theBlock[];

extern void GotoXY(int, int);
extern void Color(int, int);
extern void push(CELL);
extern CELL pop();
extern int charAvailable();
extern int getChar();
extern void printString(const char* s);
extern void printStringF(const char* fmt, ...);
extern void printChar(char c);
extern void doEditor();
extern void doCompiler(char *);

#endif 
