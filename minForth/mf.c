#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MEM_SZ      1024*1024
#define STK_MASK    0x1f

#define NCASE       goto next; case
#define RPUSH(x)    rpush((long)x)
#define RPOP        rpop()
#define S0          stk[sp]
#define S1          stk[sp-1]
#define MEMB(x)     mem.b[(x)]
#define MEML(x)     mem.l[(x)]
#define CELL_SZ     sizeof(long)

enum {
    JUMP = 0, RET, JMPT0, JMPC0, CALL, U5, U6, U7,
    U8, AATINC, LIT, AAT, UC, ASTOREINC, UE, ASTORE,
    COM, TIMES2, DIV2, ADDMULT, XOR, AND, U16, ADD,
    POPR, AVALUE, DUP, OVER, PUSHR, TOA, NOP, DROP
};

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;

long stk[STK_MASK+1], rstk[STK_MASK+1];
long sp, rsp, t, a, cf;
union { byte b[MEM_SZ*CELL_SZ]; long l[MEM_SZ]; } mem;

void push(long x) { stk[++sp] = x; }
long pop() { return stk[sp--]; }

void rpush(long x) { rsp = ((rsp+1) & STK_MASK); rstk[rsp] = x; }
long rpop() { long x = rstk[rsp]; (rsp = (rsp-1) & STK_MASK); return x; }

#ifndef NEEDS_ALIGN
long GetNumAt(byte *a) {
    return *(long*)a;
}

void SetNumAt(byte *a, long val) {
    *(long*)a = val;
}

#else
long GetNumAt(byte *a) {
    return *a | *(a+1)<<8 | *(a+2)<<16 | *(a+2)<<24;
}

void SetNumAt(byte *a, long val) {
    *a = (val & 0xFF);
    *(a+1) = (val>>8 & 0xFF);
    *(a+2) = (val>>16 & 0xFF);
    *(a+2) = (val>>24 & 0xFF);
}
#endif

void run(long st) {
    byte *pc = &MEMB(st);
    next:

    // printf("-pc/ir:%c/%d-",*(pc),*(pc));
    switch(*(pc++)) {
        case  JUMP: pc = &MEMB(GetNumAt(pc)); 
        NCASE RET: if (0 < rsp) { pc = (byte*)RPOP; } else { return; }
        NCASE JMPT0: if (S0 == 0) { pc = &MEMB(GetNumAt(pc)); } else { pc+=CELL_SZ; }
        NCASE JMPC0: if (cf != 0) { pc = &MEMB(GetNumAt(pc)); } else { pc+=CELL_SZ; }
        NCASE CALL: RPUSH(pc+sizeof(long)); pc = &MEMB(GetNumAt(pc));
        NCASE U5: // Unused
        NCASE U6: // Unused
        NCASE U7: // Unused
        // Memory access
        NCASE U8: // Unused
        NCASE AATINC: push(MEML(a++));
        NCASE LIT: push(GetNumAt(pc)); pc += CELL_SZ;
        NCASE AAT: push(MEML(a));
        NCASE UC: // Unused
        NCASE ASTOREINC: MEML(a++) = pop();
        NCASE UE: // Unused
        NCASE ASTORE: MEML(a) = pop();
        // ALU instructions
        NCASE COM: S0 = ~S0;
        NCASE TIMES2: S0 *= 2;
        NCASE DIV2:   S0 /= 2;
        NCASE ADDMULT: if (S0 & 0x01) { S0 += S1; }
        NCASE XOR: t=pop(); S0 ^= t;
        NCASE AND: t=pop(); S0 &= t;
        NCASE U16: // Unused
        NCASE ADD: t=pop(); S0 += t;
        // Register instructions
        NCASE POPR: push(rpop());
        NCASE AVALUE: push(a);
        NCASE DUP: t=S0; push(t);
        NCASE OVER: t=S1; push(t);
        NCASE PUSHR: rpush(pop());
        NCASE TOA: a = pop();
        NCASE NOP: // NOP
        NCASE DROP: sp = (0<sp) ? sp-1: 0;
        default: printf("-ir:%d-",*(pc-1)); return;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: mf binfile\n");
    } else {
        sp = rsp = 0;
        FILE *fp = fopen(argv[1],"rb");
        if (fp) {
            int n = fread(&MEMB(0), 1, MEM_SZ*CELL_SZ, fp);
            // printf("-%d bytes-\n",n);
            fclose(fp);
        run(0);
        } else {
            printf("Cannot open binfile: %s\n", argv[1]);
        }
    }
    return 0;
}
