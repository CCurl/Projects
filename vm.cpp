#include <stdio.h>
#include "shared.h"

byte IR, DSP, RSP;
CELL BASE, STATE;
CELL DSTK[STK_SZ + 1];
CELL RSTK[STK_SZ + 1];
byte CODE[CODE_SZ];
byte VARS[VARS_SZ];
// byte DICT[DICT_SZ];
UCELL PC, HERE, VHERE, LAST;

#define T  DSTK[DSP]
#define A (ADDR)DSTK[DSP]
#define N  DSTK[DSP-1]
#define R  RSTK[RSP]

void push(CELL v) {
    if (DSP < STK_SZ) { DSTK[++DSP] = v; }
}

CELL pop() {
    CELL x = T;
    if (0 < DSP) { DSP--; }
    return x;
}

void rpush(CELL v) {
    if (RSP < STK_SZ) { RSTK[++RSP] = v; }
}

CELL rpop() {
    CELL x = R;
    if (0 < RSP) { RSP--; }
    return x;
}

unsigned short GET_WORD(ADDR l) { 
    return *l | (*(l+1) << 8); 
}

void SET_WORD(ADDR l, WORD v) { 
    *l = (v & 0xff); 
    *(l+1) = (byte)(v >> 8); 
}

long GET_LONG(ADDR l) { 
    long t1 = GET_WORD(l);
    long t2 = GET_WORD(l + 2) << 16;
    return t1 | t2;
}

void SET_LONG(ADDR l, long v) { 
    SET_WORD(l, v & 0xFFFF); 
    SET_WORD(l + 2, (WORD)(v >> 16)); 
}

CELL GET_CELL(ADDR l) { return GET_LONG(l); }
int SET_CELL(ADDR l, CELL v) { SET_LONG(l, v); return CELL_SZ;  }

void CComma(CELL v) { CODE[HERE++] = (byte)v; }
void Comma(CELL v) { HERE += SET_CELL(CODE + HERE, v); }

int strLen(const char* str) {
    int l = 0;
    while (*str) {
        l++;
        str++;
    }
    return l;
}

CELL getNext(CELL l) {
    dict_t* dp = DP_AT(l);
    int len = strLen(dp->name);
    l += (CELL_SZ + 1 + len + 1);
    while ((UCELL)l % 4) { ++l; }
    return l;
}

void doWords() {
    CELL l = LAST;
    CELL end = DICT_SZ - 2;
    while (l && (l < end)) {
        dict_t* dp = DP_AT(l);
        printf("%s ", dp->name);
        l = getNext(l);
    }
}

void doEmit(CELL x) {
    putc(char(x), stdout);
}

// NB: type: 1=>VAR, 2=>CODE, 3=>absolute
ADDR buildAddr(UCELL addr, int type) {
    if (type == 1) { addr += (UCELL)VARS; }
    else if (type == 2) { addr += (UCELL)CODE; }
    return (ADDR)addr;
}

CELL doFetch(UCELL from, int type) {
    ADDR addr = buildAddr(from, type);
    return GET_CELL(addr);
}

void doStore(UCELL to, CELL val, int type) {
    ADDR addr = buildAddr(to, type);
    SET_CELL(addr, val);
}

CELL doCFetch(UCELL from, int type) {
    ADDR addr = buildAddr(from, type);
    return *addr;
}

void doCStore(UCELL to, CELL val, int type) {
    ADDR addr = buildAddr(to, type);
    *addr = (byte)val;
}

void run(CELL start) {
    PC = start;
    int rdepth = 0;
    CELL t1, t2;
    while (1) {
        IR = CODE[PC++];
        switch (IR) {
        case 0: return;
        case ' ': break;
        case 1: push(CODE[PC++]);                  break;
        case 2: push(GET_WORD(CODE+PC)); PC += 2;  break;
        case 4: push(GET_LONG(CODE+PC)); PC += 4;  break;
        case '#': push(T);                         break;
        case '\\': pop();                          break;
        case '+': t1 = pop(); T += t1;             break;
        case '-': t1 = pop(); T -= t1;             break;
        case '*': t1 = pop(); T *= t1;             break;
        case '/': t1 = pop(); T /= t1;             break;
        case '=': N = (N == T) ? 1 : 0; pop();     break;
        case '>': N = (N > T) ? 1 : 0; pop();      break;
        case '<': N = (N < T) ? 1 : 0; pop();      break;
        case '.': printf("%d", pop());             break;
        case ',': doEmit(pop());                   break;
        case 'b': printf(" ");                     break;
        case '@': T = doFetch(T, 1);                  break;
        case 11:  T = doFetch(T, 2);                 break;
        case 12:  T = doFetch(T, 3);                 break;
        case 'c': T = doCFetch(T, 1);                 break;
        case 13:  T = doCFetch(T, 2);                 break;
        case 14:  T = doCFetch(T, 3);                 break;
        case '!': doStore(T, N, 1); pop(); pop();     break;
        case 15:  doStore(T, N, 2); pop(); pop();    break;
        case 16:  doStore(T, N, 3); pop(); pop();    break;
        case 'C': doCStore(T, N, 1); pop(); pop();    break;
        case 17:  doCStore(T, N, 2); pop(); pop();    break;
        case 18:  doCStore(T, N, 3); pop(); pop();    break;
        case 'j': if (pop() == 0) {
                PC = GET_CELL(CODE+PC); 
            } else { PC += CELL_SZ; }
            break;
        case 'J': PC = GET_CELL(CODE + PC);        break;
        case 'n': printf("\r\n");                  break;
        case '$': t1 = pop(); t2 = pop();    // SWAP
            push(t1); push(t2);
            break;
        case '%': t1 = pop(); t2 = T;        // OVER
            push(t1); push(t2);
            break;
        case ':': rpush(PC + ADDR_SZ);
            PC = GET_CELL(CODE+PC);
            ++rdepth;
            break;
        case ';': if (rdepth < 1) { return; }
                PC = rpop();
                --rdepth;
                break;
        case 'W': doWords(); break;
        case 'Z': RSP = 99; return;
        }
    }
}
