#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MEM_SZ      1024*1024
#define DICTSZ      2048
#define STK_MASK    0x1f

#define NCASE       goto next; case
#define PUSH(x)     push((long)x)
#define POP         pop()
#define RPUSH(x)    rpush((long)x)
#define RPOP        rpop()
#define S0          stk[sp]
#define S1          stk[(sp-1)&STK_MASK]
#define D1          sp=((sp-1)&STK_MASK)
#define D2          sp=((sp-2)&STK_MASK)
#define R0          rstk[rsp]
#define L0          lstk[lsp]
#define L1          lstk[lsp-1]
#define L2          lstk[lsp-2]
#define MEMB(x)     mem.b[(x)]
#define MEML(x)     mem.l[(x)]

#define CELL_SZ      sizeof(long)

#define LIT1        1
#define LIT4        4

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;

long stk[STK_MASK+1], rstk[STK_MASK+1], lstk[STK_MASK+1];
long sp, rsp, lsp, t, a;
union { byte b[MEM_SZ*sizeof(long)]; long l[MEM_SZ]; } mem;

void push(long x) { sp = ((sp+1) & STK_MASK); stk[sp] = x; }
long pop() { long x = stk[sp]; D1; return x; }

void rpush(long x) { rsp = ((rsp+1) & STK_MASK); rstk[rsp] = x; }
long rpop() { long x = rstk[rsp]; (rsp = (rsp-1) & STK_MASK); return x; }

void run() {
    byte *pc = &MEMB(0);
    next:

    // printf("-pc/ir:%c/%d-",*(pc),*(pc));
    switch(*(pc++)) {
        NCASE ' ': // NOP
        NCASE 1: PUSH(*(pc++));
        NCASE 4: PUSH(*(long*)pc); pc += sizeof(long);
        NCASE '#': t=S0; PUSH(t);
        NCASE '$': t=S0; S0=S1; S1=t;
        NCASE '%': t=S1; PUSH(t);
        NCASE '\'': D1;
        NCASE '*': S1*=S0; D1;
        NCASE '+': S1+=S0; D1;
        NCASE '-': S1-=S0; D1;
        NCASE '.': printf(" %ld",POP);
        NCASE '/': S1 /= ((S0)?S0:1); D1;
        NCASE ':': if (*(pc+sizeof(long)) != ';') { RPUSH(pc+sizeof(long)); }
        case  'J': pc = &MEMB(*(long*)pc); 
        NCASE 'Z': if (S0 == 0) { pc = &MEMB(*(long*)pc); } else { pc+=sizeof(long); }
        NCASE 'z': if (S0 != 0) { pc = &MEMB(*(long*)pc); } else { pc+=sizeof(long); }
        NCASE ';': if (0 < rsp) { pc = (byte*)RPOP; } else { return; }
        NCASE '<': S1=(S1<S0)?-1:0;  D1;
        NCASE '=': S1=(S1==S0)?-1:0; D1;
        NCASE '>': S1=(S1>S0)?-1:0;  D1;
        NCASE 'a': t=*(pc++);
            if (t=='a') { PUSH(a); }                        // a
        else if (t=='>') { a = POP; }                       // >a
            else if (t=='@') { PUSH(MEMB(a)); }             // a@
            else if (t=='!') { MEMB(a) = (byte)POP; }       // a!
            else if (t=='1') { PUSH(MEMB(a++)); }           // a@+
            else if (t=='2') { MEMB(a++) = (byte)POP; }     // a!+
        NCASE 'I': PUSH(L0);
        NCASE 'N': printf("\n");
        NCASE 'l': t=*(pc++); if (t=='@') { S0 = *(long*)&MEMB(S0); }
            else if (t=='!') { *(long*)&MEMB(S0) = S1; D2; }
        NCASE 'c': t=*(pc++); if (t=='@') { S0 = MEMB(S0); }
            else if (t=='!') { MEMB(S0) = (byte)S1; D2; }
        NCASE 'Q': exit(0);
        NCASE 'T': PUSH(clock());
        NCASE '[': lsp+=3; L0 = POP; L1 = POP; L2 = (long)pc;
        NCASE ']': if (++L0 < L1) { pc = (byte*)L2; } else { lsp -= 3; }
        NCASE '\\': if (0 < sp) sp--;
        NCASE 'd': --S0;
        NCASE 'e': printf("%c",(char)POP);
        NCASE 'i': ++S0;
        NCASE 0: pc = &MEMB(0);
        default: printf("-ir:%d-",*(pc-1)); return;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: mf binfile\n");
    } else {
        sp = rsp = lsp = 0;
        FILE *fp = fopen(argv[1],"rb");
        if (fp) {
            int n = fread(&MEMB(0), 1, MEM_SZ*CELL_SZ, fp);
            // printf("-%d bytes-\n",n);
            fclose(fp);
        run();
        } else {
            printf("Cannot open binfile: %s\n", argv[1]);
        }
    }
    return 0;
}
