/*
    MOUSE - A Language for Microcomputers (interpreter)
    as described by Peter Grogono in July 1979 BYTE Magazine
*/
#define  _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <conio.h>
#include <stdio.h>

#define num(ch) (ch - 'A')
#define val(ch) (ch - '0')
#define nextchar() (ch = prog[chpos++])
#define peekchar() (prog[chpos])
#define btw(n, a, b) ((a <= n) && (n <= b))

#define MACRO        1
#define PARAM        2
#define LOOP         3
#define STACK_SZ   256
#define PGM_SZ    4096
struct frame {
    byte tag;
    int pos, off;
};

FILE* infile;
char prog[PGM_SZ], ch;
int definitions[26];
int calstack[STACK_SZ], data[STACK_SZ], cal, chpos, level, offset, parnum, parbal;
int t1, t2;
struct frame stack[STACK_SZ];
int parms[10], pstack[100], psp;

void pushcal(int datum) { calstack[cal++] = datum; }
int popcal() { return (cal) ? calstack[--cal] : 0; }

void ppush(int v) { pstack[psp++] = v; }
int ppop() { return (psp) ? pstack[--psp] : 0; }

int getChar() {
    char c;
    if (infile == stdin) { 
        c = _getch(); 
        if (31 < c) { putchar(c); }
    } else {
        fread(&c, 1, 1, infile);
    }
    return c;
}

int charAvailable() { 
    if (infile == stdin) { _kbhit(); }
    return feof(infile);
}

void push(byte tagval) {
    stack[level].tag = tagval;
    stack[level].pos = chpos;
    stack[level++].off = offset;
}

void pop() {
    chpos = stack[--level].pos;
    offset = stack[level].off;
}

void skip(char lch, char rch) {
    int cnt = 1;
    do {
        nextchar();
        if (ch == lch) cnt++;
        else if (ch == rch) cnt--;
    } while (cnt != 0);
}

void load() {
    char th, last, in = 0;
    int charnum;
    for (charnum = 0; charnum < 26; charnum++)
        definitions[charnum] = 0;
    charnum = 0;
    th = ' ';
    do {
        last = th;
        th = getChar();
        if (th == 10) { th = 13; }
        if (th == 9) { th = 32; }
        if (th == '\'') {
            do {
                th = getChar();
                // printf("x%dx", th);
            } while (th != 13);
        }
        else {
            prog[charnum] = th;
            if (th >= 'A' && th <= 'Z' && last == '$') {
                definitions[num(th)] = charnum + 1;
            }
            if (th == '\"') { in = !in; }
            if ((32 < th) || in || (btw(last, '0', '9'))) { charnum++; }
        }
    } while (th != '$' || last != '$');
}

int main(int argc, char* argv[]) {
    if (argc < 2) { infile = stdin; }
    else {
        infile = fopen(argv[1], "r");
        if (infile == NULL) {
            puts("Error: cannot load program file\n");
            return 0;
        }
    }
    load();
    if (infile != stdin) fclose(infile);
    chpos = level = offset = cal = 0;
    int go = 1;
    while (go) {
        nextchar();
        // printf("-%c-", ch);
        switch (ch) {
        case ' ': break;
        case '!': t1 = peekchar();
            if (t1 == '\'') { printf("%c", popcal()); nextchar(); }
            else if (t1 == 'n') { printf("\n"); nextchar(); }
            else { printf("%d ", popcal()); }
            break;
        case '"': nextchar();
            while (ch != '"') {
                printf("%c", (ch == '!') ? '\n' : ch);
                nextchar();
        }
            break;
        case '#': nextchar();
            if (btw(ch, '0', '9')) {
                ch = ch - '0';
                parms[ch] = popcal();
            }
            break;
        case '$': go = (peekchar() != '$');
            skip('$', ';');
            break;
        case '%': nextchar();
            if (btw(ch,'0','9')) {
                ch = ch - '0';
                pushcal(parms[ch]);
            }
            break;
        case '&':
            break;
        case '\'':
            break;
        case '(':
            break;
        case ')':
            break;
        case '*': pushcal(popcal() * popcal());
            break;
        case '+': pushcal(popcal() + popcal());
            break;
        case ',':
            break;
        case '-': pushcal(popcal() - popcal());
            break;
        case '.': 
            break;
        case '/': pushcal(popcal() / popcal());
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            t1 = 0;
            while (btw(ch, '0', '9')) {
                t1 = 10 * t1 + val(ch);
                nextchar();
            }
            pushcal(t1);
            chpos--;
            break;
        case ':': nextchar();
            break;
        case ';': pop();
            for (int i = 9; i >= 0; i--) {
                parms[i] = ppop();
            }

            break;
        case '<': pushcal(popcal() < popcal());
            break;
        case '=': t1 = popcal(); t2 = popcal();
            data[t1] = t2;
            break;
        case '>': pushcal(popcal() > popcal());
            break;
        case '?':
            break;
        case '@': pushcal(data[popcal()]);
            break;
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            t1 = num(ch); t2 = peekchar();
            if (t2 != '@') { pushcal(t1); }
            else { nextchar(); pushcal(data[t1]); }
            break;
        case '[':
            break;
        case '\\':
            break;
        case ']':
            break;
        case '^':
            break;
        case '_': pushcal(-popcal());
            break;
        case '`':
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            break;
        case '{':
            break;
        case '|':
            break;
        case '}':
            break;
        case '~': nextchar();
            if (btw(ch,'A','Z') && (definitions[num(ch)])) {
                push(MACRO);
                for (int i = 0; i < 10; i++) {
                    ppush(parms[i]);
                    parms[i] = 0;
                }
                chpos = definitions[num(ch)];
            }
            break;
        }
    }
    return 0;
}
