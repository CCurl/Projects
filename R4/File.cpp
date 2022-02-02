#include "R4.h"

#ifndef __FILES__
#ifndef __LITTLEFS__
void noFile() { printString("-noFile-"); }
void fileInit() { noFile(); }
void fileOpen() { noFile(); }
void fileClose() { noFile(); }
void fileDelete() { noFile(); }
void fileRead() { noFile(); }
void fileWrite() { noFile(); }
addr codeLoad(addr x) { noFile(); return x; }
void codeSave(addr x, addr y) { noFile(); }
void blockLoad(CELL num) { noFile(); }
int fileReadLine(FILE* fh, char* buf) { noFile(); return -1; }
#endif // __LITTLEFS__
#else
static byte fdsp = 0;
static FILE* fstack[STK_SZ + 1];
FILE* input_fp;

void fpush(FILE* v) { if (fdsp < STK_SZ) { fstack[++fdsp] = v; } }
FILE* fpop() { return (fdsp) ? fstack[fdsp--] : 0; }

void fileInit() {}

// fO (nm md--fh) - File Open
// fh: File handle, nm: File name, md: mode
// fh=0: File not found or error
void fileOpen() {
    char* md = (char *)pop();
    char* fn = (char *)TOS;
    TOS = (CELL)fopen(fn, md);
}

// fC (fh--) - File Close
// fh: File handle
void fileClose() {
    FILE* fh = (FILE*)pop();
    if (fh) { fclose(fh); }
}

// fD (nm--) - File Delete
// nm: File name
// n=0: End of file or file error
void fileDelete() {
    char* fn = (char*)TOS;
    TOS = remove(fn) == 0 ? 1 : 0;
}

// fR (fh--c n) - File Read
// fh: File handle, c: char read, n: num chars read
// n=0: End of file or file error
void fileRead() {
    FILE* fh = (FILE*)TOS;
    NOS = TOS = 0;
    push(0);
    if (fh) {
        char c;
        TOS = fread(&c, 1, 1, fh);
        NOS = TOS ? c : 0;
    }
}

// fileReadLine(fh, buf)
// fh: File handle, buf: address
// returns: -1 if EOF, else len
int fileReadLine(FILE *fh, char *buf) {
    byte c, len = 0;
    while (1) {
        *(buf) = 0;
        int n = fread(&c, 1, 1, fh);
        if (n == 0) { return -1; }
        if (c == 10) { break; }
        if (c == 13) { break; }
        if (BetweenI(c, 32, 126)) {
            *(buf++) = c;
            ++len;
        }
    }
    return len;
}

// fW (c fh--n) - File Write
// fh: File handle, c: char to write, n: num chars written
// n=0: File not open or error
void fileWrite() {
    FILE* fh = (FILE*)pop();
    char c = (char)TOS;
    TOS = 0;
    if (fh) {
        TOS = fwrite(&c, 1, 1, fh);
    }
}

// fL (--) - File Load code
addr codeLoad(addr user, addr here) {
    FILE *fh = fopen("Code.R4", "rt");
    if (fh) {
        vmInit();
        int num = fread(user, 1, USER_SZ, fh);
        fclose(fh);
        here = user + num;
        run(user);
        printStringF("-loaded, (%d bytes)-", num);
    }
    else {
        printString("-loadFail-");
    }
    return here;
}

// fS (--) - File Save code
void codeSave(addr user, addr here) {
    FILE* fh = fopen("Code.R4", "wt");
    if (fh) {
        int count = here - user;
        fwrite(user, 1, count, fh);
        fclose(fh);
        printStringF("-saved (%d)-", count);
    }
    else {
        printString("-saveFail-");
    }
}

// fB (n--) - File: block load
// Loads a block file
void blockLoad(CELL num) {
    char buf[24];
    sprintf(buf, "Block-%03ld.r4", num);
    FILE* fp = fopen(buf, "rb");
    if (fp) {
        if (input_fp) { fpush(input_fp); }
        input_fp = fp;
    }
}

#endif // __FILES__
