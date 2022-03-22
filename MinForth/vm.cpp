#include <stdio.h>
#include "shared.h"

char IR, DSP, RSP;
CELL BASE;
CELL DSTK[STK_SZ + 1];
CELL RSTK[STK_SZ + 1];
byte user[USER_SZ];
byte VARS[USER_SZ];
WORD PC;

#define TOS  DSTK[DSP]
#define NOS  DSTK[DSP-1]
#define DROP2 pop(); pop()

void push(CELL v) { if (DSP < STK_SZ) { DSTK[++DSP] = v; } }
CELL pop() { return DSP ? DSTK[DSP--] : 0; }

void rpush(CELL v) { if (RSP < STK_SZ) { RSTK[++RSP] = v; } }
CELL rpop() { return RSP ? RSTK[RSP--] : 0; }

unsigned short GET_WORD(byte *l) { return *l | (*(l+1) << 8); }
long GET_LONG(byte *l) { return GET_WORD(l) | GET_WORD(l + 2) << 16; }

void SET_WORD(byte *l, WORD v) { *l = (v & 0xff); *(l+1) = (byte)(v >> 8); }
void SET_LONG(byte *l, long v) { SET_WORD(l, v & 0xFFFF); SET_WORD(l + 2, (WORD)(v >> 16)); }

void printString(const char* cp) { printf("%s", cp); }
void printChar(char c) { printf("%c", c); }

void doDot(CELL x) {
    if (BASE == 10) { printf(" %ld", x); return; }
    if (BASE == 16) { printf(" %lx", x); return; }
    printf(" (%d in base %d)", x, BASE);
}

void run(WORD start) {
    PC = start;
    int rdepth = 0;
    CELL t1, t2;
    while (1) {
        IR = U(PC++);
        switch (IR) {
        case 0: return;
        case ' ': break;
        case 1: push(U(PC++));                       break;
        case 2: push(GET_WORD(UA(PC))); PC += 2;     break;
        case 4: push(GET_LONG(UA(PC))); PC += 4;     break;
        case '#': push(TOS);                         break;
        case '%': push(NOS);                         break;
        case '$': t1 = TOS; TOS = NOS; NOS = t1;     break;
        case '\\': pop();                            break;
        case '+': t1 = pop(); TOS += t1;             break;
        case '-': t1 = pop(); TOS -= t1;             break;
        case '*': t1 = pop(); TOS *= t1;             break;
        case '/': t1 = pop(); TOS /= t1;             break;
        case '=': NOS = (NOS == TOS) ? 1 : 0; pop(); break;
        case '>': NOS = (NOS > TOS) ? 1 : 0; pop();  break;
        case '<': NOS = (NOS < TOS) ? 1 : 0; pop();  break;
        case '.': doDot(pop());                      break;
        case ',': printChar((char)pop());            break;
        case 'b': printChar(' ');                    break;
        case '@': TOS = GET_LONG((byte*)TOS);        break;
        case 'c': TOS = *AOS;                        break;
        case 'w': TOS = GET_WORD(AOS);               break;
        case '!': SET_LONG(AOS, NOS); DROP2;         break;
        case 'C': *AOS = (byte)NOS; DROP2;           break;
        case 'W': SET_WORD(AOS, (WORD)NOS); DROP2;   break;
        case 'A': t1 = pop(); TOS &= t1;             break;
        case 'O': t1 = pop(); TOS |= t1;             break;
        case 'X': t1 = pop(); TOS ^= t1;             break;
        case '&': t1 = NOS; t2 = TOS;
            NOS = t1 / t2; TOS = t1 % t2;            break;
        case 'j': if (pop() == 0) { PC = GET_WORD(UA(PC)); }
                else { PC += ADDR_SZ; }
            break;
        case 'J': PC = GET_WORD(UA(PC));             break;
        case 'n': printf("\r\n");                    break;
        case ':': rpush(PC + ADDR_SZ);
            PC = GET_WORD(user+PC);
            ++rdepth;
            break;
        case ';': if (rdepth < 1) { return; }
                PC = (WORD)rpop();
                --rdepth;
                break;
        case 'Z': RSP = 99; return;
        }
    }
}
