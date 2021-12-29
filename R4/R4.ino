#include "r4.h"

#if __SERIAL__
    int charAvailable() { return mySerial.available(); }
    int getChar() { 
        while (!charAvailable()) {}
        return mySerial.read();
    }
    void printChar(char c) { mySerial.print(c); }
    void printString(const char* str) { mySerial.print(str); }
#else
    int charAvailable() { return 0; }
    int getChar() { return 0; }
    void printString(const char* str) { }
    void printChar(char c) { }
#endif

CELL getSeed() {
  return millis();
}

addr doPinRead(addr pc) {
    byte ir = *(pc++);
    CELL pin = pop();
    switch (ir) {
    case 'A': push(analogRead(pin));          break;
    case 'D': push(digitalRead(pin));         break;
    default:
        isError = 1;
        printString("-pinRead-");
    }
    return pc;
}

addr doPinWrite(addr pc) {
    byte ir = *(pc++);
    CELL pin = pop();
    CELL val = pop();
    switch (ir) {
    case 'A': analogWrite(pin, val);          break;
    case 'D': digitalWrite(pin, val);         break;
    default:
        isError = 1;
        printString("-pinWrite-");
    }
    return pc;
}

addr doPin(addr pc) {
    CELL t1 = *(pc++);
    switch (t1) {
    case 'I': pinMode(pop(), INPUT);          break;
    case 'O': pinMode(pop(), OUTPUT);         break;
    case 'U': pinMode(pop(), INPUT_PULLUP);   break;
    case 'R': pc = doPinRead(pc);             break;
    case 'W': pc = doPinWrite(pc);            break;
    default:
        isError = 1;
        printString("-notPin-");
    }
    return pc;
}

addr doCustom(byte ir, addr pc) {
    switch (ir) {
    case 'N': push(micros());          break;
    case 'P': pc = doPin(pc);          break;
    case 'T': push(millis());          break;
    case 'W': delay(pop());            break;
    default:
        isError = 1;
        printString("-notExt-");
    }
    return pc;
}

void loadCode(const char* src) {
    addr here = (addr)HERE;
    addr here1 = here;
    while (*src) {
        *(here1++) = *(src++);
    }
    *here1 = 0;
    run(here);
}

void input_push(FILE *fp) { }
FILE *input_pop() { return NULL; }

// ********************************************
// * HERE is where you load your default code *
// ********************************************

#define SOURCE_STARTUP \
    X(1000, ":C xIAU xIH1-[rIc@#,59=(rI1+c@58=(E))];") \
    X(1010, ":R 0 xIR1-[rIa@#(rIeN\": \".E1)\\];") \
    X(1020, ":F 0 xIF1-[rI4*xIAF+@#(rI#.\"-\"eN\": \".E1)\\];") \
    X(1030, ":N 26S$26S$'A+,'A+,'A+,;") \
    X(1040, ":U xIH xIAU-.;") \
    X(1045, ":SI E\"System info: \"xIR.\" registers, \"xIF.\" functions, \"xIU.\" bytes user memory.\";") \
    X(2000, ":Q iT rA#*rS/sC rB#*rS/sD rCrD+rK>(rJsM;)rArB*100/rY+sB rCrD-rX+sA iJ;") \
    X(2010, ":L 0sA 0sB 0sJ rS sM 1{\\eQ rJ rM<};") \
    X(2020, ":O eLrJ40+#126>(\\32),;") \
    X(2030, ":X 490NsX 1 95[  eO rX 8+sX];") \
    X(2040, ":Y 300NsY 1 31[E eX rY20+sY];") \
    X(2050, ":M eI 0sT xT eY xT$- E rT.\" iterations, \" . \" ms\";") \
    X(2060, ":I 200 sS 1000000 sK;")

#define X(num, val) const PROGMEM char str ## num[] = val;
SOURCE_STARTUP

#undef X
#define X(num, val) str ## num,
const char *bootStrap[] = {
    SOURCE_STARTUP
    NULL
};

void loadBaseSystem() {
    for (int i = 0; bootStrap[i] != NULL; i++) {
        loadCode(bootStrap[i]);
    }
}

void ok() {
    printString("\r\nr4:("); 
    dumpStack(); 
    printString(")>");
}

int isBackspace(char c) {
  return (c == 127) ? 1 : 0;
}

void handleInput(char c) {
    static addr here = (addr)NULL;
    static addr here1 = (addr)NULL;
    if (here == NULL) { 
        here = (addr)HERE; 
        here1 = here; 
    }
    if (c == 13) {
        printString(" ");
        *(here1) = 0;
        run(here);
        here = (addr)NULL;
        ok();
        return;
    }

    if (isBackspace(c) && (here < here1)) {
        here1--;
        char b[] = {8, 32, 8, 0};
        printString(b);
        return;
    }
    if (c == 9) { c = 32; }
    if (32 <= c) {
        *(here1++) = (byte)c;
        char b[] = {c, 0};
        printString(b);
    }
}

void setup() {
#ifdef __SERIAL__
    while (!mySerial) {}
    mySerial.begin(19200);
    while (mySerial.available()) { char c = mySerial.read(); }
#endif
    vmInit();
    loadBaseSystem();
    ok();
}

void loop() {
    static int iLed = 0;
    static long nextBlink = 0;
    static int ledState = LOW;
    long curTm = millis();
    
    if (iLed == 0) {
        iLed = ILED;
        pinMode(iLed, OUTPUT);
    }
    if (nextBlink < curTm) {
        ledState = (ledState == LOW) ? HIGH : LOW;
        digitalWrite(iLed, ledState);
        nextBlink = curTm + 1111;
    }

    while ( charAvailable() ) { handleInput(getChar()); }

    // addr a = functionAddress("R");
    // if (a) { run(a); }
}
