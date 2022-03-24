#include "shared.h"

byte sp, rsp, lsp, isError;
CELL BASE;
CELL stk[STK_SZ+1];
CELL rstk[STK_SZ+1];
LOOP_T lstk[LSTK_SZ+1];
byte user[USER_SZ+1];
byte vars[VARS_SZ+1];

void vmReset() {
    sp = rsp = lsp = 0;
    BASE = 10;
    HERE = LAST = 0;
    VHERE = (CELL)&vars[0];
    for (int i = 0; i < USER_SZ; i++) { user[i] = 0; }
    for (int i = 0; i < VARS_SZ; i++) { vars[i] = 0; }
}

void push(CELL v) { if (sp < STK_SZ) { stk[++sp] = v; } }
CELL pop() { return sp ? stk[sp--] : 0; }

void rpush(CELL v) { if (rsp < STK_SZ) { rstk[++rsp] = v; } }
CELL rpop() { return rsp ? rstk[rsp--] : 0; }

LOOP_T *lpush() { return (lsp < LSTK_SZ) ? &lstk[++lsp] : 0; }
LOOP_T *lpop() { return (lsp) ? &lstk[--lsp] : 0; }

WORD GET_WORD(byte *l) { return *l | (*(l+1) << 8); }
long GET_LONG(byte *l) { return GET_WORD(l) | GET_WORD(l + 2) << 16; }

void SET_WORD(byte *l, WORD v) { *l = (v & 0xff); *(l+1) = (byte)(v >> 8); }
void SET_LONG(byte *l, long v) { SET_WORD(l, v & 0xFFFF); SET_WORD(l + 2, (WORD)(v >> 16)); }

void printBase(CELL num, CELL base) {
    UCELL n = (UCELL) num;
    char isNeg = ((base == 10) && (num < 0)) ? 1 : 0;
    if (isNeg) { n = -num; }
    char* cp = (char *)&user[USER_SZ];
    *(cp--) = 0;
    do {
        int x = (n % base) + '0';
        n = n / base;
        *(cp--) = ('9' < x) ? (x+7) : x;
    } while (n);
    if (isNeg) { printChar('-'); }
    printString(cp+1);
}

void run(WORD start) {
    WORD pc = start;
    CELL t1, t2;
    rsp = lsp = isError = 0;
    while (isError == 0) {
        byte IR = U(pc++);
        switch (IR) {
        case 0: return;
        case  1 : push(U(pc++));                                       break;
        case  2 : push(GET_WORD(UA(pc))); pc += 2;                     break;
        case  4 : push(GET_LONG(UA(pc))); pc += 4;                     break;
        case '#': push(TOS);                                           break;
        case '%': push(NOS);                                           break;
        case '$': t1 = TOS; TOS = NOS; NOS = t1;                       break;
        case '\\': pop();                                              break;
        case '+': t1 = pop(); TOS += t1;                               break;
        case '-': t1 = pop(); TOS -= t1;                               break;
        case '*': t1 = pop(); TOS *= t1;                               break;
        case '/': t1 = pop(); TOS /= t1;                               break;
        case 'A': t1 = pop(); TOS &= t1;                               break;
        case 'O': t1 = pop(); TOS |= t1;                               break;
        case 'X': t1 = pop(); TOS ^= t1;                               break;
        case '~': TOS = ~TOS;                                          break;
        case '=': NOS = (NOS == TOS) ? 1 : 0; pop();                   break;
        case '>': NOS = (NOS > TOS) ? 1 : 0; pop();                    break;
        case '<': NOS = (NOS < TOS) ? 1 : 0; pop();                    break;
        case 'N': TOS = (TOS) ? 0 : 1;                                 break;
        case ',': printChar((char)pop());                              break;
        case 'D': TOS--;                                               break;
        case 'I': TOS++;                                               break;
        case '@': TOS = GET_LONG((byte*)TOS);                          break;
        case 'c': TOS = *AOS;                                          break;
        case 'w': TOS = GET_WORD(AOS);                                 break;
        case '!': SET_LONG(AOS, NOS); DROP2;                           break;
        case 'C': *AOS = (byte)NOS; DROP2;                             break;
        case 'W': SET_WORD(AOS, (WORD)NOS); DROP2;                     break;
        case '.': printBase(pop(), BASE);                           // break;
        case 'b': printChar(' ');                                      break;
        case 't': push(timer());                                break;
        case '&': t1 = NOS; t2 = TOS;
            NOS = t1 / t2; TOS = t1 % t2;                              break;
        case 'j': if (pop() == 0) { pc = GET_WORD(UA(pc)); }
                else { pc += 2; }                                      break;
        case 'J': pc = GET_WORD(UA(pc));                               break;
        case 'n': printString("\r\n");                                 break;
        case ':': rpush(pc+2); pc = GET_WORD(UA(pc));                  break;
        case ';': pc = (WORD)rpop();                                   break;
        case '[': lpush()->e = GET_WORD(UA(pc)); pc += 2;
            LOS.s = pc;
            LOS.f = TOS < NOS ? TOS : NOS;
            LOS.t = TOS > NOS ? TOS : NOS; DROP2;                      break;
        case 'i': push(LOS.f);                                         break;
        case ']': ++LOS.f; if (LOS.f <= LOS.t) { pc = LOS.s; }
                else { lpop(); }                                       break;
        case '{': lpush()->e = GET_WORD(UA(pc)); LOS.s = pc+2;      // break;
        case '}': if (TOS) { pc = LOS.s; } 
                else { pc = LOS.e;  pop(); lpop(); }                   break;
        case 255: vmReset();                                           return;
        default: pc = doExt(IR, pc);                                   break;
        }
    }
}
