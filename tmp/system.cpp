#include "c4.h"

#ifdef IS_WINDOWS

#include <conio.h>
int qKey() { return _kbhit(); }
int key() { return _getch(); }
void ttyMode(int isRaw) {}

#endif

#ifdef IS_LINUX // Support for Linux

#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

void ttyMode(int isRaw) {
    static struct termios origt, rawt;
    static int curMode = -1;
    if (curMode == -1) {
        curMode = 0;
        tcgetattr( STDIN_FILENO, &origt);
        cfmakeraw(&rawt);
    }
    if (isRaw != curMode) {
        if (isRaw) {
            tcsetattr( STDIN_FILENO, TCSANOW, &rawt);
        } else {
            tcsetattr( STDIN_FILENO, TCSANOW, &origt);
        }
        curMode = isRaw;
    }
}
int qKey() {
    struct timeval tv;
    fd_set rdfs;
    ttyMode(1);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    int x = FD_ISSET(STDIN_FILENO, &rdfs);
    // ttyMode(0);
    return x;
}
int key() {
    ttyMode(1);
    int x = fgetc(stdin);
    // ttyMode(0);
    return x;
}

#endif // IS_LINUX

#ifdef IS_PC
cell timer() { return (cell)clock(); }
void zType(const char* str) { fputs(str, outputFp ? (FILE*)outputFp : stdout); }
void emit(const char ch) { fputc(ch, outputFp ? (FILE*)outputFp : stdout); }

int main(int argc, char *argv[]) {
	Init();
    inner(0);
	return 0;
}

#endif // is_PC
