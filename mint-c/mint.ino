// MINT - A Minimal Interpreter - for details, see https://github.com/monsonite/MINT

#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>
#include "mint.h"

void printChar(const char c) { Serial.print(c); }
void printString(const char* str) { Serial.print(str); }

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
}

void loop() {
}
