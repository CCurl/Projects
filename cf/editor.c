// editor.cpp - A simple block editor
//
// NOTE: A huge thanks to Alain Theroux. This editor was inspired by
//       his editor and is a shameful reverse-engineering of it. :D

#include "cf.h"

#define COMMAND       1
#define INSERT        2
#define REPLACE       3

enum { curLEFT = 200, curRIGHT, curUP, curDOWN, curHOME, curEND, curPGUP, curPGDN };

#define LLEN       100
#define NUM_LINES   20
#define BLOCK_SZ    (NUM_LINES)*(LLEN)
#define MAX_CUR     (BLOCK_SZ-1)
#define SETC(c)     edLines[line][off]=c
int line, off, blkNum, mode;
int cur, isDirty = 0;
char theBlock[BLOCK_SZ];
const char* msg = NULL;
char edLines[NUM_LINES][LLEN];

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void Color(int c, int bg) {
    printStringF("%c[%d;%dm", 27, (30 + c), bg ? bg : 40);
}

void NormLO() {
    if (line < 0) { line = 0; }
    if (NUM_LINES <= line) { line = NUM_LINES - 1; }
    if (off < 0) { off = 0; }
    if (LLEN <= off) { off = LLEN - 1; }
}

char edChar(int l, int o) {
    char c = edLines[l][o];
    if (betw(c, 1, 7)) {
        Color(c, 40);
        //c += 15;
        c = 32;
    }
    return c ? c : ' ';
}

void showLine(int l) {
    // CursorOff();
    GotoXY(1, l + 1);
    for (int o = 0; o < LLEN; o++) {
        char c = edChar(l, o);
        printChar(c);
    }
    printString("   ");
    // CursorOn();
}

void showCursor() {
    char c = edChar(line, off);
    GotoXY(off + 1, line + 1);
    Color(0, 47);
    printChar(c ? c : 'X');
    //Color(WHITE, 0);
}

void mv(int l, int o) {
    showLine(line);
    line += l;
    off += o;
    NormLO();
    showLine(line);
    showCursor();
}

void edSetCh(char c) {
    SETC(c);
    mv(0, 1);
    isDirty = 1;
}

int edGetChar() {
    CursorOn();
    int c = key();
    CursorOff();
    // in PuTTY, cursor keys are <esc>, '[', [A..D]
    // other keys are <esc>, '[', [1..6] , '~'
    if (c == 27) {
        c = key();
        if (c == '[') {
            c = key();
            if (c == 'A') { return 'w'; } // up
            if (c == 'B') { return 's'; } // down
            if (c == 'C') { return 'd'; } // right
            if (c == 'D') { return 'a'; } // left
            if (c == '1') { if (key() == '~') { return 'q'; } } // home
            if (c == '4') { if (key() == '~') { return 'e'; } } // end
            if (c == '5') { if (key() == '~') { return 't'; } } // top (pgup)
            if (c == '6') { if (key() == '~') { return 'l'; } } // last (pgdn)
            if (c == '3') { if (key() == '~') { return 'x'; } } // del
        }
        return c;
    }
    else {
        // in Windows, cursor keys are 224, [HPMK]
        // other keys are 224, [GOIQS]
        if (c == 224) {
            c = key();
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

void toBlock() {
    int c = 0;
    for (int y = 0; y < NUM_LINES; y++) {
        for (int x = 0; x < LLEN; x++) {
            theBlock[c++] = edLines[y][x];
        }
    }
}

void toLines() {
    int c = 0;
    for (int y = 0; y < NUM_LINES; y++) {
        for (int x = 0; x < LLEN; x++) {
            char ch = theBlock[c++];
            edLines[y][x] = ch ? ch : ' ';
        }
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
    toLines();
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
    toBlock();
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

void showFooter() {
    GotoXY(1, NUM_LINES);
    printString("- Block Editor v0.1 - ");
    printStringF("Block# %03d %c", blkNum, isDirty ? '*' : ' ');
    printStringF(" %s -\r\n", msg ? msg : "");
    printString("\r\n  (q)home (w)up (e)end (a)left (s)down (d)right (t)op (l)ast");
    printString("\r\n  (x)del char (r)eplace (i)nsert");
    printString("\r\n  (W)rite (L)reLoad (+)next (-)prev (Q)uit");
    printString("\r\n  (D)efine (C)ompile (I)nterp (A)sm (M)Comment");
    printString("\r\n-> \x8");
}

void showEditor() {
    int cp = 0, x = 1, y = 1;
    CursorOff();
    Color(WHITE, 0);
    msg = NULL;
    int color = PURPLE;
    for (int i = 0; i < NUM_LINES; i++) {
        showLine(i);
    }
    showCursor();
    GotoXY(1, NUM_LINES);
    Color(WHITE, 0);
}

void lineEdit() {
    for (int i = cur; i < MAX_CUR; i++) { theBlock[i] = theBlock[i + 1]; }
}

void deleteChar() {
    for (int o = off; o < (LLEN - 2); o++) {
        edLines[line][o] = edLines[line][o + 1];
    }
    isDirty = 1;
    showLine(line);
    showCursor();
}

void insertChar(char c, int refresh) {
    for (int o = LLEN-1; o > off; o--) {
        edLines[line][o] = edLines[line][o - 1];
    }
    SETC(c);
    isDirty = 1;
    if (refresh) {
        showLine(line);
        showCursor();
    }
}

void doType(int isInsert) {
    CursorOff();
    while (1) {
        char c = key();
        if (c == 27) { return; }
        int isBS = ((c == 127) || (c == 8));
        if (isBS) {
            if (off) {
                --off;
                if (isInsert) { deleteChar(); }
                else { SETC(' '); }
            }
        } else {
            if (isInsert) { insertChar(' ', 0); }
            if (betw(c, 8, 31)) { c = ' '; }
            edSetCh(c);
        }
        showLine(line);
        showCursor();
        isDirty = 1;
    }
}

void insertMode() { mode = INSERT; }
int isCursorMove(int c) { return betw(c,curLEFT,curEND) ? 1 : 0; }

int doInsertReplace(char c) {
    if (mode == INSERT) { insertChar(c, 1); }
    else { edSetCh(c); }
    return 1;
}

int moveCursor(int c) {
    switch (c) {
        case 9: mv(0,8);                            break;
        case 'a': mv(0,-1);                         break;
        case 'd': mv(0,1);                          break;
        case 'w': mv(-1,0);                         break;
        case 's': mv(1,0);                          break;
        case 'q': mv(0,-off);                       break;
        case 'e': mv(0,99);                         break;
        case 't': mv(-99,-99);                      break;
        case 'l': mv(99,99);                        break;
    }
    return 1;
}

int processEditorChar(char c) {
    if (c==27) { mode = COMMAND; return 1; }
    if (betw(c,32,126) && betw(mode,COMMAND,INSERT)) {
        return doInsertReplace(c);
    }
    if (isCursorMove(c)) { return moveCursor(c); }
    printChar(c);
    cur = (line*LLEN) + off;
    mode = COMMAND;
    switch (c) {
    case 'Q': toBlock(); return 0;              break;
    case 'i': doType(1);                        break;
    case 'r': doType(0);                        break;
    case 'x': deleteChar();                     break;
    case 'L': edRdBlk();                        break;
    case 'W': edSvBlk();                        break;
    case 'M': edSetCh(COMMENT); insertMode();   break;
    case 'C': edSetCh(COMPILE); insertMode();   break;
    case 'D': edSetCh(DEFINE);  insertMode();   break;
    case 'I': edSetCh(INTERP);  insertMode();   break;
    case 'A': edSetCh(ASM);     insertMode();   break;
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

void doEditor(CELL blk) {
    line = 0;
    off = 0;
    blkNum = blk;
    if (0 <= blkNum) { edRdBlk(); }
    blkNum = (0 <= blkNum) ? blkNum : 0;
    CLS();
    showEditor();
    showFooter();
    while (processEditorChar(edGetChar())) {
        NormLO();
        Color(WHITE, 0);
        showFooter();
    }
    CursorOn();
}
