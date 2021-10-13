#include <stdio.h>
#include "shared.h"

CELL BASE, STATE;
ADDR MEM;
byte IR, * PC, * HERE;
dict_t *LAST;
UCELL MEM_SZ;

byte DSP;
CELL T, DSTK[STK_SZ+1];
#define N DSTK[DSP]

byte RSP;
ADDR RSTK[STK_SZ+1];
#define R RSTK[RSP]

void push(CELL v) {
    DSP = (DSP+1) & STK_SZ;
    N = T;
    T = v;
}

void drop() {
    T = N;
    DSP = (DSP-1)&STK_SZ;
}
#define drop2() drop(); drop()

CELL pop() {
    CELL x = T;
    drop();
    return x;
}

void rpush(ADDR v) {
    RSP = (RSP+1) & STK_SZ;
    R = v;
}

inline void rdrop() {
    RSP = (RSP-1)&STK_SZ;
}

ADDR rpop() {
    ADDR x = R;
    rdrop();
    return x;
}

void vm_init(ADDR mem, UCELL mem_sz) {
    MEM = mem;
    MEM_SZ = mem_sz;
    DSP = RSP = STK_SZ;
    HERE = mem;
    LAST = 0;
    BASE = 10;
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

void CComma(CELL v) { *(HERE++) = (byte)v; }
void Comma(CELL v) { HERE += SET_CELL(HERE, v); }

int strLen(const char* str) {
    int l = 0;
    while (*str) {
        l++;
        str++;
    }
    return l;
}

dict_t *getNext(dict_t *l) {
    dict_t* dp = l;
    int len = strLen(dp->name);
    l = (dict_t *)l->next;
    while ((UCELL)l % 4) { ++l; }
    return l;
}

void doWords() {
    dict_t *l = LAST;
    ADDR end = MEM+MEM_SZ-2;
    while (l) {
        dict_t *dp =(dict_t *)l;
        printf("%s ", dp->name);
        l = getNext(l);
    }
}

void doEmit(CELL x) {
    putc(char(x), stdout);
}

// NB: type: 1=>VAR, 2=>CODE, 3=>absolute
ADDR buildAddr(UCELL addr, int type) {
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

void run(ADDR start) {
    PC = start;
    int rdepth = 0;
    CELL t1, t2;
    while (1) {
        IR = *(PC++);
        switch (IR) {
        case 0: return;
        case ' ': break;
        case 1: push(*(PC++));                     break;
        case 2: push(GET_WORD(PC)); PC += 2;       break;
        case 4: push(GET_LONG(PC)); PC += 4;       break;
        case '#': push(T);                         break;
        case '\\': pop();                          break;
        case '+': N += T; drop();             break;
        case '-': N -= T; drop();             break;
        case '*': N *= T; drop();             break;
        case '/': N /= T; drop();             break;
        case '=': N = (N == T) ? 1 : 0; drop();     break;
        case '>': N = (N > T)  ? 1 : 0; drop();      break;
        case '<': N = (N < T)  ? 1 : 0; drop();      break;
        case '.': printf("%d", pop());             break;
        case ',': doEmit(pop());                   break;
        case 'b': printf(" ");                     break;
        case '@': T = doFetch(T, 1);                  break;
        case 'c': T = doCFetch(T, 1);                 break;
        case '!': doStore(T, N, 1);  drop2();  break;
        case 'C': doCStore(T, N, 1); drop2();  break;
        case 'j': if (pop() == 0) {
                PC = (ADDR)GET_CELL(PC); 
            } else { PC += CELL_SZ; }
            break;
        case 'J': PC = (ADDR)GET_CELL(PC);        break;
        case 'n': printf("\r\n");                  break;
        case '$': t1 = pop(); t2 = pop();    // SWAP
            push(t1); push(t2);
            break;
        case '%': t1 = pop(); t2 = T;        // OVER
            push(t1); push(t2);
            break;
        case ':': rpush(PC + ADDR_SZ);
            PC = (ADDR)GET_CELL(PC);
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
