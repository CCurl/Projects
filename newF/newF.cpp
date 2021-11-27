// S4 - A Minimal Interpreter

#include "newF.h"

SYS_T sys;
byte ir, isBye = 0, isError = 0;
static char buf[96];
addr pc;
CELL t1;
addr HERE;
addr LAST;
addr DICT_END;
CELL STATE;
CELL BASE;

void p(const char *x) { printString(x); }
void push(CELL v) { p("1"); if (sys.dsp < STK_SZ) { sys.dstack[++sys.dsp] = v; } }
CELL pop() { p("0"); return (sys.dsp) ? sys.dstack[sys.dsp--] : 0; }

inline void rpush(addr v) { if (sys.rsp < STK_SZ) { sys.rstack[++sys.rsp] = v; } }
inline addr rpop() { return (sys.rsp) ? sys.rstack[sys.rsp--] : 0; }

#define lAt() (&sys.lstack[LSP])
inline LOOP_ENTRY_T* lpush() { if (LSP < STK_SZ) { ++LSP; } return lAt(); }
inline LOOP_ENTRY_T *ldrop() { if (0 < LSP) { --LSP; } return lAt(); }

void vmInit() {
    sys.dsp = sys.rsp = sys.lsp = 0;
    for (int i = 0; i < NUM_REGS; i++) { REG[i] = 0; }
    for (int i = 0; i < USER_SZ; i++) { USER[i] = 0; }
    for (int i = 0; i < NUM_FUNCS; i++) { FUNC[i] = 0; }
    REG['f' - 'a'] = (CELL)&sys.func[0];
    REG['g' - 'a'] = NUM_REGS;
    REG['h' - 'a'] = (CELL)&sys.user[0];
    REG['n' - 'a'] = NUM_FUNCS;
    REG['r' - 'a'] = (CELL)&sys.reg[0];
    REG['s' - 'a'] = (CELL)&sys;
    REG['u' - 'a'] = (CELL)&sys.user[0];
    REG['z' - 'a'] = USER_SZ;
    HERE = &sys.user[0];
    LAST = HERE + USER_SZ;
    DICT_END = LAST-1;
    *(CELL *)LAST = 0;
    STATE = 0;
    BASE = 10;
    char src[64];
    sprintf(src, ": (here) %ld ;", (CELL)&HERE);        parse(src);
    sprintf(src, ": (last) %ld ;", (CELL)&LAST);        parse(src);
    sprintf(src, ": base %ld ;", (CELL)&BASE);          parse(src);
    sprintf(src, ": state %ld ;", (CELL)&STATE);        parse(src);
    sprintf(src, ": user %ld ;", (CELL)&sys.user[0]);   parse(src);
    sprintf(src, ": user_sz %ld ;", (CELL)USER_SZ);     parse(src);
}

void setCell(addr to, CELL val) {
#ifdef _NEEDS_ALIGN_
    *(to++) = (byte)val; 
    for (int i = 1; i < CELL_SZ; i++) {
        val = (val >> 8);
        *(to++) = (byte)val;
    }
#else
    * ((CELL *)to) = val;
#endif
}

CELL getCell(addr from) {
    CELL val = 0;
#ifdef _NEEDS_ALIGN_
    from += (CELL_SZ - 1);
    for (int i = 0; i < CELL_SZ; i++) {
        val = (val << 8) + *(from--);
    }
#else
    val = *((CELL*)from);
#endif
    return val;
}

void dumpStack() {
    for (UCELL i = 1; i <= sys.dsp; i++) {
        printStringF("%s%ld", (i > 1 ? " " : ""), (CELL)sys.dstack[i]);
    }
}

void printStringF(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printString(buf);
}

void skipTo(byte to) {
    while (*pc) {
        ir = *(pc++);
        if ((to == '"') && (ir != to)) { continue; }
        if (ir == to) { return; }
        if (ir == '\'') { ++pc; continue; }
        if (ir == '(') { skipTo(')'); continue; }
        if (ir == '[') { skipTo(']'); continue; }
        if (ir == '"') { skipTo('"'); continue; }
    }
    isError = 1;
}

int AddIt(CELL n, char c, char f, int m) {
    n = (n * 26) + (c - f);
    if (m <= n) { isError = 1; }
    return n;
}

void doIf() {
    CELL n = pop();
    if (n == 0) {
        skipTo(')');
        return;
    }
}

void doFor() {
    CELL t = (N < T) ? T : N;
    CELL f = (N < T) ? N : T;
    DROP2;
    LOOP_ENTRY_T *x = lpush();
    x->start = pc;
    INDEX = x->from = f;
    x->to = t;
    x->end = 0;
}

void doNext() {
    LOOP_ENTRY_T* x = lAt();
    x->from = ++INDEX;
    if (x->from <= x->to) {
        x->end = pc;
        pc = x->start;
    } else {
        INDEX = ldrop()->from;
    }
}

void doExt() {
    ir = *(pc++);
    switch (ir) {
    case '!': *(byte*)T = (byte)N; DROP2;          return;
    case '-': T = -T;                              return;
    case '/': if (T) { t1 = T; T = N % t1; N /= t1; }
        else { isError = 1; printString("-0div-"); }
        return;
    case '%': if (T) { N %= T; DROP1; }
        else { isError = 1; printString("-0div-"); }
        return;
    case '@': T = *(byte*)T;                       return;
    case 'C': rpush(pc);                        // fall thru to 'J'
    case 'J': pc = (addr)pop();                    return;
    case 'R': vmInit();                            return;
    default:
        pc = doCustom(ir, pc);
    }
}

addr run(addr start) {
    isError = 0;
    pc = start;
    LSP = 0;
    while (!isError && pc) {
        ir = *(pc++);
        switch (ir) {
        case 0: return pc;
        case 1: push(*(pc++));                          break;
        case 4: push(getCell(pc)); pc += CELL_SZ;       break;
        case 5: rpush(pc);                              // NO BREAK!
        case 6: pc = (addr)getCell(pc);                 break;
        case ' ': while (*(pc) == ' ') { pc++; }        break;  // 32
        case '!': setCell((byte*)T, N); DROP2;          break;  // 33
        case '"': while (*(pc) != ir) { printChar(*(pc++)); };  // 34
                ++pc; break;
        case '#': push(T);                              break;  // 35 (DUP)
        case '$': t1 = N; N = T; T = t1;                break;  // 36 (SWAP)
        case '%': push(N);                              break;  // 37 (OVER)
        case '&': t1 = pop(); T &= t1;                  break;  // 38
        case '\'': push(*(pc++));                       break;  // 39
        case '(': doIf();                               break;  // 40
        case ')': /* endIf() */                         break;  // 41
        case '*': t1 = pop(); T *= t1;                  break;  // 42
        case '+': t1 = pop(); T += t1;                  break;  // 43
        case ',': printChar((char)pop());               break;  // 44
        case '-': t1 = pop(); T -= t1;                  break;  // 45
        case '.': printStringF("%ld", (CELL)pop());     break;  // 46
        case '/': if (T) { N /= T; DROP1; }                     // 47
                else { isError = 1;  printString("-0div-"); }
                break;
        case '0': case '1': case '2': case '3': case '4':       // 48-57
        case '5': case '6': case '7': case '8': case '9':
            push(ir - '0'); ir = *(pc);
            while (BetweenI(ir, '0', '9')) {
                T = (T * 10) + (ir - '0');
                ir = *(++pc);
            } break;
        case ':': 
            break;
        case ';': pc = rpop();                          break;  // 59
        case '<': t1 = pop(); T = T < t1 ? 1 : 0;       break;  // 60
        case '=': t1 = pop(); T = T == t1 ? 1 : 0;      break;  // 61
        case '>': t1 = pop(); T = T > t1 ? 1 : 0;       break;  // 62
        case '?': /* FREE */                            break;  // 63
        case '@': T = getCell((byte*)T);                break;  // 64
        case 'A': case 'B': case 'C': case 'D': case 'E':       // 65-90
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O': case 'P':
        case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
            t1 = ir - 'A';
            if (BetweenI(*pc, 'A', 'Z') && (26 < NUM_FUNCS)) {
                t1 = AddIt(t1, *(pc++), 'A', NUM_FUNCS);
                if (BetweenI(*pc, 'A', 'Z') && ((26*26) < NUM_FUNCS)) {
                    t1 = AddIt(t1, *(pc++), 'A', NUM_FUNCS);
                }
            }
            isError = (NUM_FUNCS <= t1);
            if (isError) { printString("-funcNum-"); }
            else if (FUNC[t1]) {
                if (*pc != ';') { rpush(pc); }
                pc = FUNC[t1];
            } break;
        case '[': doFor();                              break;  // 91
        case '\\': DROP1;                               break;  // 92
        case ']': doNext();                             break;  // 93
        case '^': t1 = pop(); T ^= t1;                  break;  // 94
        case '_': T = (T) ? 0 : 1;                      break;  // 95
        case '`': doExt();                              break;  // 96
        case 'a': case 'b': case 'c': case 'd': case 'e':       // 97-122
        case 'f': case 'g': case 'h': case 'i': case 'j': 
        case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': 
        case 'q': case 'r': case 's': case 't': case 'u': 
        case 'v': case 'w': case 'x': case 'y': case 'z': 
            t1 = ir - 'a';
            if (BetweenI(*pc, 'a', 'z') && (26 < NUM_REGS)) {
                t1 = AddIt(t1, *(pc++), 'a', NUM_REGS);
                if (BetweenI(*pc, 'a', 'z') && ((26*26) < NUM_REGS)) {
                    t1 = AddIt(t1, *(pc++), 'a', NUM_REGS);
                }
            }
            isError = (t1 < NUM_REGS) ? 0 : 1;
            if (isError) { printString("-regNum-"); }
            else { push((CELL)&REG[t1]); }              break;
        case '{': if (T) { lpush()->start = pc; }               // 123
                else { DROP1;  skipTo('}'); }
            break;
        case '|': t1 = pop(); T |= t1;                  break;  // 124
        case '}': if (!T) { ldrop(); DROP1; }                   // 125
                else { lAt()->end = pc; pc = lAt()->start; }
            break;
        case '~': T = ~T;                               break;  // 126
        }
    }
    return pc;
}

bool strEquals(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) { return 0; }
        ++str1;
        ++str2;
    }
    return (*str1 == *str2) ? 1 : 0;
}

int strlen(const char *x) {
    int l = 0;
    while (*x) { ++x; ++l; }
    return l;
}

void strcpy(char *t, const char *f) {
    while (*f) { *(t++) = *(f++); }
    *t = 0;
}

void cComma(byte x) {
    p("c,");
    *(HERE++) = x;
}

void comma(CELL x) {
    p(",");
    setCell(HERE, x);
    HERE += CELL_SZ;
}

void create(const char *word) {
    DICT_T *dp = (DICT_T *)LAST;
    --dp;
    dp->XT = HERE;
    dp->flags = 0;
    dp->len = strlen(word);
    strcpy(dp->name, word);
    LAST = (addr)dp;
    printStringF("\n%s created at %p, XT=%p", word, dp, dp->XT);
}

DICT_T *find(char *word) {
    DICT_T *dp = (DICT_T *)LAST;
    while ((addr)dp < DICT_END) {
        if (strEquals(dp->name, word)) { return dp;  }
        ++dp;
    }
    return NULL;
}

bool isNum(char *word) {
    push(0);
    while (*word) {
        if (*word == ' ') { return 1; }
        if ((*word < '0') || ('9' < *word)) {
            pop();
            return 0;
        }
        T = (T*BASE)+(*word-'0');
        word++;
    }
    return 1;
}

char *getWord(char *line, char *word) {
    char *x = word;
    while (*line == ' ') { ++line; }
    while ((*line) && (*line != ' ')) {
        *(word++) = *(line++);
    }
    *word = 0;
    // p("-"); p(x); p("-");
    return line;
}

void parse(char *line) {
    p("\n"); p(line);
    char wd[32];
    while (1) {
        line = getWord(line, wd);
        if (wd[0] == 0) {
            return;
        }
        DICT_T *dp = find(wd);
        if (dp) {
            if ((STATE == 0) || (dp->flags == 2)) {
                run(dp->XT);
            } else {
                cComma(5);
                comma((CELL)dp->XT);
            }
            continue;
        }
        if (isNum(wd)) {
            if (STATE) {
                cComma(4);
                comma(pop());
            }
            continue;
        }
        if (strEquals(wd, ":")) {
            line = getWord(line, wd);
            if (wd[0]) {
                create(wd);
                STATE = 1;
            }
            continue;
        }
        if (strEquals(wd, ";")) {
            cComma(';');
            STATE = 0;
            continue;
        }
    }
}

