#include "cf.h"

char* toIn;

char peekCh() { return *toIn; }

char getOneCh() {
    if (*toIn) { return *(toIn++); }
    return 0;
}

int getWord(char *buf) {
    char* b = buf;
    int l = 0;
    while (peekCh() == ' ') { getOneCh(); }
    while (' ' < peekCh()) {
        *(b++) = getOneCh();
        l++;
    }
    *b = 0;
    return l;
}

void doDefine(const char* wd) { printStringF("-def:%s-", wd); }
void doCompile(const char * wd) { printStringF("-com:%s-", wd); }
void doInterpret(const char * wd) { printStringF("-int:%s-", wd); }

void doCompiler(char *cp) {
    char buf[32];
    toIn = cp;
    int here = 0, color = WHITE;
    while (1) {
        char c = peekCh();
        if (!c) { return; }
        if (betw(c, 1, 7)) { color = getOneCh(); continue; }
        if (getWord(buf) == 0) { continue; }
        switch (color) {
        case DEFINE:  doDefine(buf);                    break;
        case COMPILE: doCompile(buf);                   break;
        case INTERP:  doInterpret(buf);                 break;
        case COMMENT: printStringF("-skp:%s-", buf);    break;
        default: break;
        }
    }
}
