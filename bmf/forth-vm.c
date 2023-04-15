#include <time.h>
#include "Shared.h"
#include "string.h"
#include "forth-vm.h"

extern int string_len(char* str);
extern bool string_equals(char *str1, char *str2);

// ------------------------------------------------------------------------------------------
// The VM
// ------------------------------------------------------------------------------------------
BYTE the_memory[MEM_SZ];
BYTE sp, rsp;
CELL stk[DSTACK_SZ+1], rstk[RSTACK_SZ+1];

#define TOS   stk[sp]
#define NOS   stk[sp-1]
#define POP   pop();
#define POP2  POP;POP

bool isEmbedded = false;
bool isBYE = false;

int _QUIT_HIT = 0;

static CELL arg1, arg2, arg3;
CELL hStdin = NULL, hStdout = NULL;

// ------------------------------------------------------------------------------------------
void init_vm()
{
    int sz = MEM_SZ;
    for (int i = 0; i < sz; i++) { the_memory[i] = 0; }
    CELL_AT(ADDR_MEM_SZ) = sz;
    sp = rsp = 0;
}

// ------------------------------------------------------------------------------------------
void push(CELL val) { if (sp < DSTACK_SZ) { stk[++sp] = val; } }
CELL pop() { return (0 < sp) ? stk[sp--] : 0; }

// ------------------------------------------------------------------------------------------
void rpush(CELL val) { if (rsp < RSTACK_SZ) { rstk[++rsp] = val; } }
CELL rpop() { return (0 < rsp) ? rstk[rsp--] : 0; }

typedef struct {
    CELL f, t, i;
    ADDR s, e;
} LOOP_T;

BYTE lsp;
LOOP_T lstk[8];

void doFor(ADDR pc) {
    LOOP_T* x = &lstk[++lsp];
    x->s = pc;
    x->e = 0;
    x->f = TOS < NOS ? TOS : NOS;
    x->t = TOS > NOS ? TOS : NOS;
    x->i = x->f;
}

ADDR doNext(ADDR pc) {
    if (lsp) {
        LOOP_T* x = &lstk[lsp];
        if (x->i++ < x->t) { pc = x->s; }
    }
    return pc;
}

// ------------------------------------------------------------------------------------------
// Where all the work is done
// ------------------------------------------------------------------------------------------
void cpu_loop(ADDR start)
{
    ADDR PC = start;
    BYTE IR;
    rsp = lsp = 0;

    isBYE = false;
    while (true)
    {
        IR = the_memory[PC++];
        switch (IR)
        {
        case CLITERAL: push(the_memory[PC++]);            break;
        case LITERAL: push(CELL_AT(PC));
            PC += CELL_SZ;                                break;
        case WLITERAL: push(WORD_AT(PC));
            PC += WORD_SZ;                                break;
        case FETCH: TOS = CELL_AT(TOS);                   break;
        case STORE: CELL_AT(TOS) = NOS; POP2;             break;
        case WSTORE: WORD_AT(TOS) = (USHORT)NOS; POP2;    break;
        case SWAP: arg1 = NOS;NOS = TOS; TOS = arg1;      break;
        case DROP: pop();                                 break;
        case DUP: push(TOS);                              break;
        case SLITERAL: push(PC); PC += BYTE_AT(PC) + 2;   break;
        case JMP: PC = ADDR_AT(PC);                       break;
        case JMPZ: if (pop()) { PC += ADDR_SZ; }
            else { PC = ADDR_AT(PC); }                    break;
        case JMPNZ: if (pop()) { PC = ADDR_AT(PC); }
                else { PC += ADDR_SZ; }                   break;
        case CALL: rpush(PC+ADDR_SZ); PC = ADDR_AT(PC);   break;
        case RET: if (rsp < 1) { return; }
                PC = (ADDR)rpop();                        break;
        case AND: NOS &= TOS; pop();                      break;
        case OR: NOS |= TOS; pop();                       break;
        case NOT: TOS = TOS == 0 ? 1 : 0;                 break;
        case XOR: NOS ^= TOS; pop();                      break;
        case COM: TOS = ~TOS;                             break;
        case CFETCH: TOS = the_memory[TOS];               break;
        case CSTORE: the_memory[TOS] = (BYTE)NOS; POP2;   break;
        case ADD: arg1 = pop(); TOS += arg1;              break;
        case SUB: arg1 = pop(); TOS -= arg1;              break;
        case MUL: arg1 = pop(); TOS *= arg1;              break;
        case DIV: arg1 = pop(); TOS /= arg1;              break;
        case LT: arg1=pop(); TOS = (TOS<arg1)? 1 : 0;     break;
        case EQ: arg1=pop(); TOS = (TOS==arg1)? 1 : 0;    break;
        case GT: arg1=pop(); TOS = (TOS>arg1)? 1 : 0;     break;
        case DICTP:PC += ADDR_SZ;                         break;
        case EMIT: arg1 = pop(); putchar(arg1);           break;
        case OVER: push(NOS);                             break;
        case FOR: doFor(PC);                              break;
        case NEXT: PC = doNext(PC);                       break;
        case INDEX: push(lsp ? lstk[lsp].i : 0);          break;
        case COMPARE: arg2 = pop(); arg1 = pop();
            {
                char *cp1 = (char *)&the_memory[arg1];
                char *cp2 = (char *)&the_memory[arg2];
                arg3 = string_equals(cp1, cp2);
                push(arg3);
            }
            break;
        case FOPEN: arg3 = pop();   // type: 0 => text, 1 => binary
            arg2 = pop();   // mode: 0 => read, 1 => write
            arg1 = pop();   // name
            {
                char *fileName = (char *)&the_memory[arg1 + 1];
                char mode[4];
                sprintf(mode, "%c%c", (arg2 == 0) ? 'r' : 'w', (arg3 == 0) ? 't' : 'b');
                TRACE("FOPEN %s, %s\n", fileName, mode);
                FILE *fp = fopen(fileName, mode);
                arg1 = (CELL) fp;
                push(arg1);
                push(fp != NULL ? 1 : 0);
            }
            break;
        case FREAD: arg3 = pop();
            arg2 = pop();
            arg1 = pop();
            {
                BYTE *pBuf = (BYTE *)&the_memory[arg1 + 1];
                int num = fread(pBuf, sizeof(BYTE), arg2, (arg3 == 0) ? stdin : (FILE *)arg3);
                push(num);
            }
            break;
        case FREADLINE: arg3 = pop();		// FP - 0 means STDIN
            arg2 = pop();		// max-sz
            arg1 = pop();		// to-addr - NB: this is a COUNTED and NULL-TERMINATED string!
            {
                char *tgt = (char *)&the_memory[arg1];
                FILE *fp = arg3 ? (FILE *)arg3 : stdin;
                char *pBuf = tgt;
                if (fgets((pBuf+1), arg2, fp) != (pBuf+1))
                {
                    // printf("<EOF>\n");
                    *(pBuf) = 0;
                    *(pBuf+1) = (char)0;
                    push(0);
                    break;
                }
                arg2 = (CELL)string_len(pBuf+1);
                // Strip off the trailing newline if there
                if ((arg2 > 0) && (pBuf[arg2] == '\n'))
                {
                    pBuf[arg2--] = (char)NULL;
                }
                *(pBuf) = (char)(arg2);
                push((arg2 > 0) ? arg2 : 1);
                // printf("%d: [%s]\n", (int)pBuf[0], pBuf+1);
            }
            break;
        case FWRITE: arg3 = pop();
            arg2 = pop();
            arg1 = pop();
            {
                BYTE *pBuf = (BYTE *)&the_memory[arg1];
                int num = fwrite(pBuf, sizeof(BYTE), arg2, arg3 == 0 ? stdin : (FILE *)arg3);
                push(num);
            }
            break;
        case FCLOSE: arg1 = pop();
            // printf("(fclose %lx)", arg1);
            if (arg1 != 0)
                fclose((FILE *)arg1);
            break;
        case DTOR: rpush(pop());             break;
        case RTOD: push(rpop());             break;
        case RFETCH: push(rstk[rsp]);        break;
        case PICK: TOS = stk[sp-TOS];        break;
        case DEPTH: push(sp);                break;
        case RDEPTH: push(rsp);              break;
        case GETCH: push(0);                 break; // push(getChar());
        case COMPAREI:
            arg2 = pop();
            arg1 = pop();
            {
                char *cp1 = (char *)&the_memory[arg1];
                char *cp2 = (char *)&the_memory[arg2];
                arg3 = string_equals_nocase(cp1, cp2);
                push(arg3);
            }
            break;
        case SLASHMOD: arg1 = NOS; arg2 = TOS;
            NOS = arg1 / arg2;   // quotient
            TOS = arg1 % arg2;   // remainder
            break;
        case INC: ++TOS;                         break;
        case DEC: --TOS;                         break;
        case GETTICK: arg1 = clock();
            push(arg1);
            break;
        case SHIFTLEFT: NOS = NOS << TOS;        break;
        case SHIFTRIGHT: NOS = NOS >> TOS;       break;
        case PLUSSTORE: CELL_AT(TOS) += NOS;
            POP2;                                break;
        case OPENBLOCK:
            { 
                char fn[64];
                arg1 = pop();
                sprintf(fn, "block-%04ld.fs", arg1);
                FILE *fp = fopen(fn, "rt");
                push((CELL)fp);
                push(TOS ? -1 : 0);
            }
            break;
        case DBGDOT: arg1 = pop();
            printf("[%ld] ", arg1);
            break;
        case DBGDOTS: printf("(%d)[", sp);
            for (int i = 1; i <= sp; i++) {
                printf("(%d)",i);
            }
            printf("]");
            break;
        case NOP:              break;
        case BREAK:            break;
        case RESET:            printf("-RESET ar %d-", PC-1);
            PC = 0;            break;
        case BYE:              return;
        default:               break;
        }
    }
    return;
}
