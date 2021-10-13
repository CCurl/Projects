// MinForth.cpp : An extremely memory conscious Forth interpreter
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "Shared.h"

#define mem_sz 2048
char word[32];
FILE* input_fp = NULL;
byte lastWasCall = 0;
byte mem[mem_sz];

int main()
{
    vm_init(mem, mem_sz);

    printf("\r\nMinForth v0.0.1");
    printf("\r\nMEM: %p, SIZE: %lu, HERE: %p", mem, mem_sz, HERE);
    printf("\r\nHello.");
    push(1);
    push(2);
    push(3);
    ADDR x = (ADDR)"++.";
    run(x);
}
