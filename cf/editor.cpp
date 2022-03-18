// editor.cpp - A simple block editor
//
// NOTE: A huge thanks to Alain Theroux. This editor was inspired by
//       his editor and is a shameful reverse-engineering of it. :D

#include "cf.h"

#define LLEN        100
#define MAX_X       (LLEN-1)
#define MAX_Y       20
#define BLOCK_SZ    (LLEN)*(MAX_Y+1)
#define MAX_CUR     (BLOCK_SZ-1)
int line, off, blkNum;
int cur, isDirty = 0;
char theBlock[BLOCK_SZ];
const char* msg = NULL;

int edGetChar() {
    int c = getChar();
    // in PuTTY, cursor keys are <esc>, '[', [A..D]
    // other keys are <esc>, '[', [1..6] , '~'
    if (c == 27) {
        c = getChar();
        if (c == '[') {
            c = getChar();
            if (c == 'A') { return 'w'; } // up
            if (c == 'B') { return 's'; } // down
            if (c == 'C') { return 'd'; } // right
            if (c == 'D') { return 'a'; } // left
            if (c == '1') { if (getChar() == '~') { return 'q'; } } // home
            if (c == '4') { if (getChar() == '~') { return 'e'; } } // end
            if (c == '5') { if (getChar() == '~') { return 't'; } } // top (pgup)
            if (c == '6') { if (getChar() == '~') { return 'l'; } } // last (pgdn)
            if (c == '3') { if (getChar() == '~') { return 'x'; } } // del
        }
        return c;
    }
    else {
        // in Windows, cursor keys are 224, [HPMK]
        // other keys are 224, [GOIQS]
        if (c == 224) {
            c = getChar();
            if (c == 'H') { return 'w'; } // up
            if (c == 'P') { return 's'; } // down
            if (c == 'M') { return 'd'; } // right
            if (c == 'K') { return 'a'; } // left
            if (c == 'G') { return 'q'; } // home
            if (c == 'O') { return 'e'; } // end
            if (c == 'I') { return 't'; } // top pgup (pgup)
            if (c == 'Q') { return 'l'; } // last (pgdn)
            if (c == 'S') { return 'x'; } // del
            return c;
        }
    }
    return c;
}

void clearBlock() {
    for (int i = 0; i < BLOCK_SZ; i++) {
        theBlock[i] = 32;
        // if ((i % 50) == 0) { theBlock[i] = 10; }
    }
}

void edRdBlk() {
    clearBlock();
    char buf[24];
    sprintf(buf, "block-%03d.cf", blkNum);
    msg = "-noFile-";
    FILE* fp = fopen(buf, "rb");
    if (fp) {
        fread(theBlock, 1, BLOCK_SZ, fp);
        msg = "-loaded-";
        fclose(fp);
    }
    //push(blkNum);
    //push((CELL)theBlock);
    //push(BLOCK_SZ);
    // blockRead();
    //msg = (pop()) ? "-loaded-" : "-noFile-";
    cur = isDirty = 0;
}

void edSvBlk() {
    //push(blkNum);
    //push((CELL)theBlock);
    //push(BLOCK_SZ);
    //push(0);
    // blockWrite();
    char buf[24];
    sprintf(buf, "block-%03d.cf", blkNum);
    msg = "-err-";
    FILE* fp = fopen(buf, "wb");
    if (fp) {
        fwrite(theBlock, 1, BLOCK_SZ, fp);
        msg = "-saved-";
        fclose(fp);
    }
    // msg = (pop()) ? "-saved-" : "-errWrite-";
    cur = isDirty = 0;
}

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void Color(int c, int bg) {
    printStringF("%c[%d;%dm", 27, (30 + c), bg ? bg : 40);
}

void showGuide() {
    printString("\r\n    +");
    for (int i = 0; i <= MAX_X; i++) { printChar('-'); }
    printChar('+');
}

void showFooter() {
    printString("     Block Editor v0.1 - ");
    printStringF("Block# %03d %c", blkNum, isDirty ? '*' : ' ');
    printStringF(" %-20s", msg ? msg : "");
    printString("\r\n     (q)home (w)up (e)end (a)left (s)down (d)right (t)top (l)last");
    printString("\r\n     (x)del char (i)insert char (r)replace char (I)Insert (R)Replace");
    printString("\r\n     (n)LF (S)Save (L)reLoad (+)next (-)prev (Q)quit");
    printString("\r\n-> \x8");
}

void showEditor() {
    int cp = 0, x = 1, y = 1;
    CursorOff();
    GotoXY(x, y);
    Color(WHITE, 0);
    msg = NULL;
    int color = PURPLE;
    // showGuide();
    for (int i = 0; i < BLOCK_SZ; i++) {
        unsigned char c = theBlock[cp];
        int bg = 0;
        if (betw(c, 1, 7)) { color = c; Color(c, 0); }
        if (c == 13) { c = 174; }
        if (c == 10) { c = 241; }
        if (c < 32) { c = 32; }
        int isCur = (cur == cp) ? 1 : 0;
        if (isCur) { Color(color, 47); }
        printChar((char)c);
        if (x++ == LLEN) { x = 1; y++; GotoXY(x, y); }
        if (isCur) { Color(color, 0); }
        ++cp;
    }
    // showGuide();
    GotoXY(1, 21);
    CursorOn();
    Color(WHITE, 0);
}

void deleteChar() {
    for (int i = cur; i < MAX_CUR; i++) { theBlock[i] = theBlock[i + 1]; }
    theBlock[MAX_CUR - 1] = 0;
    theBlock[MAX_CUR] = 0;
}

void insertChar(char c) {
    for (int i = MAX_CUR; cur < i; i--) { theBlock[i] = theBlock[i - 1]; }
    theBlock[cur] = c;
    theBlock[MAX_CUR] = 0;
}

void doType(int isInsert) {
    CursorOff();
    while (1) {
        char c = getChar();
        if (c == 27) { return; }
        int isBS = ((c == 127) || (c == 8));
        if (isBS) {
            if (cur) {
                theBlock[--cur] = ' ';
                if (isInsert) { deleteChar(); }
                showEditor();
            }
            continue;
        }
        if (isInsert) { insertChar(' '); }
        if (c == 13) { c = 10; }
        theBlock[cur++] = c;
        if (MAX_CUR < cur) { cur = MAX_CUR; }
        showEditor();
        CursorOff();
    }
}

int processEditorChar(char c) {
    printChar(c);
    switch (c) {
    case 'Q': return 0;                                  break;
    case 9: cur += 8;                                    break;
    case 'a': --cur;                                     break;
    case 'd': ++cur;                                     break;
    case 'w': cur -= LLEN;                               break;
    case 's': cur += LLEN;                               break;
    case 'q': cur -= (cur % LLEN);                       break;
    case 'e': cur -= (cur % LLEN); cur += MAX_X;         break;
    case 't': cur = 0;                                   break;
    case 'l': cur = MAX_CUR - MAX_X;                     break;
    case 'r': isDirty = 1; theBlock[cur++] = getChar();  break;
    case 'I': isDirty = 1; doType(1);                    break;
    case 'R': isDirty = 1; doType(0);                    break;
    case 'n': isDirty = 1; theBlock[cur++] = 10;         break;
    case 'x': isDirty = 1; deleteChar();                 break;
    case 'i': isDirty = 1; insertChar(' ');              break;
    case 'L': edRdBlk();                                 break;
    case 'W': edSvBlk();                                 break;
    case 'C': theBlock[cur++] = COMMENT;                 break;
    case 'G': theBlock[cur++] = COMPILE;                 break;
    case 'D': theBlock[cur++] = DEFINE;                  break;
    case 'Y': theBlock[cur++] = INTERP;                  break;
    case '+': if (isDirty) { edSvBlk(); }
            ++blkNum;
            edRdBlk();
            break;
    case '-': if (isDirty) { edSvBlk(); }
            blkNum -= (blkNum) ? 1 : 0;
            edRdBlk();
            break;
    }
    return 1;
}

void doEditor() {
    blkNum = pop();
    blkNum = (0 <= blkNum) ? blkNum : 0;
    CLS();
    edRdBlk();
    showEditor();
    showFooter();
    while (processEditorChar(edGetChar())) {
        if (cur < 0) { cur = 0; }
        if (MAX_CUR < cur) { cur = MAX_CUR; }
        showEditor();
        showFooter();
    }
}
