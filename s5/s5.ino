#include "s4.h"

#ifdef __SERIAL__
#define mySerial SerialUSB
int _getch() { return (mySerial.available()) ? mySerial.read() : 0; }
void printString(const char* str) { mySerial.print(str); }
#else
int _getch() { return 0; }
void printString(const char* str) { }
#endif

#define TIB_SZ    80
#define TIB      (CODE_SZ - TIB_SZ)
int tibEnd;

void loadCode(const char* src) {
    int i = TIB;
    while (*src) { setCodeByte(i++, *(src++)); }
    setCodeByte(i, 0);
    run(TIB);
}

// ********************************************
// * HERE is where you load your default code *
// ********************************************

void loadBaseSystem() {
    loadCode("0( MB: ManageButton )");
    loadCode("0( SC: SetContext )");
    loadCode("0( RD: Read pin, put val in to reg v )");
    loadCode("0( CQ: Changed? )");
    loadCode("0( RV: RememberValue )");
    loadCode("0( RA: todo )");
}

void ok() {
    printString("\r\ns4:"); dumpStack(0); printString(">");
}

void setup() {
#ifdef __SERIAL__
    while (!mySerial) {}
    mySerial.begin(19200);
    while (mySerial.available()) { char c = mySerial.read(); }
#endif
    vmInit();
    tibEnd = TIB;
    loadBaseSystem();
    ok();
}

void loop() {
    static int iLed = 0;
    static ulong nextBlink = 0;
    static int ledState = LOW;
    ulong curTm = millis();
    
    if (iLed == 0) {
        iLed = ILED;
        pinMode(iLed, OUTPUT);
    }
    if (nextBlink < curTm) {
        ledState = (ledState == LOW) ? HIGH : LOW;
        digitalWrite(iLed, ledState);
        nextBlink = curTm + 1111;
    }

#ifdef __SERIAL__
    while (mySerial.available()) {
        char c = mySerial.read();
        if (c == 9) { c = 32; }
        if (c == 13) {
            printString(" ");
            if (TIB < tibEnd) {
                setCodeByte(tibEnd, 0);
                run(TIB);
            }
            ok();
            tibEnd = TIB;
        } else {
            if ((32 <= c) && (tibEnd < CODE_SZ)) {
                setCodeByte(tibEnd++, c);
                char b[2]; b[0] = c; b[1] = 0;
                printString(b);
            }
        }
    }
#endif
    int addr = functionAddress(0);
    if (addr) { run(addr); }
}
