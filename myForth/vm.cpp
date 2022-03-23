#include "shared.h"
#include <stdio.h>
#include <Windows.h>

char IR, sp, rsp, lsp;
CELL BASE;
CELL stk[STK_SZ+1];
CELL rstk[STK_SZ+1];
LOOP_T lstk[LSTK_SZ+1];
byte user[USER_SZ+1];
byte vars[VARS_SZ+1];
WORD PC;

void push(CELL v) { if (sp < STK_SZ) { stk[++sp] = v; } }
CELL pop() { return sp ? stk[sp--] : 0; }

void rpush(CELL v) { if (rsp < STK_SZ) { rstk[++rsp] = v; } }
CELL rpop() { return rsp ? rstk[rsp--] : 0; }

LOOP_T *lpush() { return (lsp < LSTK_SZ) ? &lstk[++lsp] : 0; }
LOOP_T *lpop() { return (lsp) ? &lstk[--lsp] : 0; }

unsigned short GET_WORD(byte *l) { return *l | (*(l+1) << 8); }
long GET_LONG(byte *l) { return GET_WORD(l) | GET_WORD(l + 2) << 16; }

void SET_WORD(byte *l, WORD v) { *l = (v & 0xff); *(l+1) = (byte)(v >> 8); }
void SET_LONG(byte *l, long v) { SET_WORD(l, v & 0xFFFF); SET_WORD(l + 2, (WORD)(v >> 16)); }

void printBase(UCELL num, CELL base) {
    char* cp = (char *)&user[USER_SZ];
    *(cp--) = 0;
    do {
        int x = num % base;
        num = num / base;
        if (9 < x) { x += 7; }
        *(cp--) = ('0' + x);
    } while (num);
    printString(cp+1);
}

void run(WORD start) {
    PC = start;
    CELL t1, t2;
    rsp = lsp = 0;
    while (1) {
        IR = U(PC++);
        switch (IR) {
        case 0: return;
        case   1: push(U(PC++));                       break;
        case   2: push(GET_WORD(UA(PC))); PC += 2;     break;
        case   4: push(GET_LONG(UA(PC))); PC += 4;     break;
        case '#': push(TOS);                           break;
        case '%': push(NOS);                           break;
        case '$': t1 = TOS; TOS = NOS; NOS = t1;       break;
        case '\\': pop();                              break;
        case '+': t1 = pop(); TOS += t1;               break;
        case '-': t1 = pop(); TOS -= t1;               break;
        case '*': t1 = pop(); TOS *= t1;               break;
        case '/': t1 = pop(); TOS /= t1;               break;
        case '=': NOS = (NOS == TOS) ? 1 : 0; pop();   break;
        case '>': NOS = (NOS > TOS) ? 1 : 0; pop();    break;
        case '<': NOS = (NOS < TOS) ? 1 : 0; pop();    break;
        case ',': printChar((char)pop());              break;
        case 'D': TOS--;                               break;
        case 'I': TOS++;                               break;
        case '@': TOS = GET_LONG((byte*)TOS);          break;
        case 'c': TOS = *AOS;                          break;
        case 'w': TOS = GET_WORD(AOS);                 break;
        case '!': SET_LONG(AOS, NOS); DROP2;           break;
        case 'C': *AOS = (byte)NOS; DROP2;             break;
        case 'W': SET_WORD(AOS, (WORD)NOS); DROP2;     break;
        case 'A': t1 = pop(); TOS &= t1;               break;
        case 'O': t1 = pop(); TOS |= t1;               break;
        case 'X': t1 = pop(); TOS ^= t1;               break;
        case '.': printBase(pop(), BASE);              // break;
        case 'b': printChar(' ');                      break;
        case 't': push(GetTickCount());                break;
        case '&': t1 = NOS; t2 = TOS;
            NOS = t1 / t2; TOS = t1 % t2;              break;
        case 'j': if (pop() == 0) { PC = GET_WORD(UA(PC)); }
                else { PC += 2; }                      break;
        case 'J': PC = GET_WORD(UA(PC));               break;
        case 'n': printf("\r\n");                      break;
        case ':': rpush(PC+2); PC = GET_WORD(UA(PC));  break;
        case ';': PC = (WORD)rpop();                   break;
        case '[': lpush()->e = GET_WORD(UA(PC)); PC += 2;
            LOS.s = PC;
            LOS.f = TOS < NOS ? TOS : NOS;
            LOS.t = TOS > NOS ? TOS : NOS;
            DROP2;                                     break;
        case 'i': push(LOS.f);                         break;
        case ']': ++LOS.f; if (LOS.f <= LOS.t) { PC = LOS.s; }
                else { lpop(); }                       break;
        case '{': lpush()->e = GET_WORD(UA(PC)); PC += 2;
            LOS.s = PC;                                break;
        case '}': if (TOS == 0) { pop(); lpop(); }
                else { PC = LOS.s; }                   break;
        case 'Z': rsp = 99;                            return;
        default:                                       break;
        }
    }
}
