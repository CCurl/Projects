// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>
#include "mint.h"

#define __SERIAL__ 1
#define mySerial Serial

#if __SERIAL__
    int charAvailable() { return mySerial.available(); }
    char getChar() { 
        while (!charAvailable()) {}
        return mySerial.read();
    }
    void printChar(char c) { mySerial.print(c); }
    void printString(const char* str) { mySerial.print(str); }
#else
    int charAvailable() { return 0; }
    char getChar() { return 0; }
    void printString(const char* str) { }
    void printChar(char c) { }
#endif

addr doFile(addr pc) {
    return pc;
}

addr doPin(addr pc) {
    int ir = *(pc++);
    CELL pin = pop(), val = 0;
    switch (ir) {
    case 'I': pinMode(pin, INPUT);         break;
    case 'U': pinMode(pin, INPUT_PULLUP);  break;
    case 'O': pinMode(pin, OUTPUT);        break;
    case 'R': ir = *(pc++);
        if (ir == 'D') { push(digitalRead(pin)); }
        if (ir == 'A') { push(analogRead(pin)); }
        break;
    case 'W': ir = *(pc++); val = pop();
        if (ir == 'D') { digitalWrite(pin, val); }
        if (ir == 'A') { analogWrite(pin, val); }
        break;
    }
    return pc;
}

void ok() {
    printString("\r\nmint:(");
    dumpStack();
    printString(")>");
}

void setup() {
#ifdef __SERIAL__
    mySerial.begin(19200);
    while (!mySerial) {}
#endif
    vmInit();
    //loadBaseSystem();
    ok();
}

char *tib = 0, *tib1 = 0;

void handleChar(char c) {
  if (tib == 0) {
    tib = (char *)HERE;
    tib1 = tib;
    *(tib1) = 0;
  }
  if (c == 13) {
    printString(tib);
    printChar(' ');
    run((addr)tib);
    ok();
    tib = 0;
  }
  if (32 <= c) {
    *(tib1++) = c;
    *(tib1) = 0;
  }
}

void loop() {
  while (charAvailable()) {
    char c = getChar();
    handleChar(c);
  }
}
