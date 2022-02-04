#include "R4.h"

#ifdef __LITTLEFS__
#include "LittleFS.h"

#define MAX_FILES 10
File *files[MAX_FILES];
int numFiles = 0;
File f;

void fpush(FILE *fp) {}
FILE *fpop() { return 0; }

int freeFile() {
  for (int i = 1; i <= MAX_FILES; i++) {
    if (files[i-1] == NULL) { return i; }
  }
  isError = 1;
  printString("-fileFull-");
  return 0;
}

void fileInit() {
    for (int i = 0; i < MAX_FILES; i++) { files[i] = NULL; }
    LittleFS.begin();
    FSInfo fs_info;
    LittleFS.info(fs_info);
    printStringF("\r\nLittleFS: Total: %ld", fs_info.totalBytes);
    printStringF("\r\nLittleFS: Used: %ld", fs_info.usedBytes);
}

void fileOpen() {
    char* md = (char*)pop();
    char* fn = (char*)pop();
    int i = freeFile();
    if (i) {
        f = LittleFS.open(fn, md);
        if (f) { files[i-1] = &f; } 
        else { 
            i = 0;
            isError = 1; 
            printString("-openFail-");
        }
    }
    push(i);
}

void fileClose() {
    int fn = (int)pop();
    if ((0 < fn) && (fn <= MAX_FILES) && (files[fn-1] != NULL)) {
        files[fn-1]->close();
        files[fn-1] = NULL;
    }
}

void fileDelete() {
    char* fn = (char*)TOS;
    TOS = LittleFS.remove(fn) ? 1 : 0;
}

void fileRead() {
    int fn = (int)pop();
    push(0);
    push(0);
    if ((0 < fn) && (fn <= MAX_FILES) && (files[fn-1] != NULL)) {
        byte c;
        TOS = files[fn-1]->read(&c, 1);
        NOS = (CELL)c;
    }
}

// fileReadLine(fh, buf)
// fh: File handle, buf: address
// return: -1 if EOF, else len
int fileReadLine(int fh, char* buf) {
    byte c, len = 0;
    while (1) {
        *(buf) = 0;
        int n = files[fh - 1]->read(&c, 1);
        if (n == 0) { return -1; }
        if (c == 13) { break; }
        if (BetweenI(c, 32, 126)) {
            *(buf++) = c;
            ++len;
        }
    }
    return len;
}

void fileWrite() {
    int fn = (int)pop();
    byte c = (byte)TOS;
    TOS = 0;
    if ((0 < fn) && (fn <= MAX_FILES) && (files[fn - 1] != NULL)) {
        TOS = files[fn-1]->write(&c, 1);
    }
}

addr codeLoad(addr u, addr h) {
    File f = LittleFS.open("/Code.R4", "r");
    if (f) {
        vmInit();
        int num = f.read(u, USER_SZ);
        f.close();
        h = u + num;
        run(u);
        printStringF("-loaded, (%d)-", num);
    }
    else {
        printString("-loadFail-");
    }
    return h;
}

void codeSave(addr u, addr h) {
    File f = LittleFS.open("/Code.R4", "w");
    if (f) {
        int count = h - u;
        f.write(u, count);
        f.close();
        printString("-saved-");
    }
    else {
        printString("-saveFail-");
    }
}

void blockLoad(CELL num) {
    printString("-noBlock-");
    pop();
}

int readBlock(int blk, char* buf, int sz) {
    char fn[24];
    sprintf(fn, "/Block-%03d.R4", blk);
    for (int i = 0; i < sz; i++) { buf[i] = 32; }
    File f = LittleFS.open(fn, "r");
    if (f) {
        int n = f.read((uint8_t*)buf, sz);
        f.close();
        return 1;
    }
    else {
        return 0;
    }
}

int writeBlock(int blk, char* buf, int sz) {
    char fn[24];
    sprintf(fn, "/Block-%03d.R4", blk);
    for (int i = 0; i < sz; i++) { buf[i] = 32; }
    File f = LittleFS.open(fn, "w");
    if (f) {
        int n = f.write((uint8_t *)buf, sz);
        f.close();
        return 1;
    }
    else {
        return 0;
    }
}

#endif // __LITTLEFS__
