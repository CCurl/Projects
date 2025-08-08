// An extremely simple x86 emulator to test my compiler.
// This implements a simple stack-based machine
// It only implement the small subset of x86 opcodes,
// those that the code generator uses.

// EAX is the "top of stack" TOS
// EBP is the "data stack" pointer
// ESP is the "return stack" pointer

#include <stdio.h>
#include <stdint.h>

#define VM_SZ     10000
#define DSTK_SZ     128

#define BTWI(n,l,h) ((l<=n)&&(n<=h))
typedef void (*voidfn_t)();
typedef struct { uint8_t val, mod, r, m; } MODRM_T;

#define _DEBUG_ 1

#ifdef _DEBUG_
    #define DBG(str) printf("-%s-", str)
    #define DBG1(str, num) printf("-%s%d-", str, num)
    int dbg = 1;
#else 
    #define DBG(str)
    #define DBG1(str, num)
    int dbg = 0;
#endif

#define EAX reg[0]
#define ECX reg[1]
#define EDX reg[2]
#define EBX reg[3]
#define ESP reg[4]
#define EBP reg[5]
#define ESI reg[6]
#define EDI reg[7]

uint8_t vm[VM_SZ];
int32_t reg[8], disp, arg1, memBase, EBPbase;
uint32_t ip, here, *src, *tgt;
uint8_t ir, flags[8], stkDepth;
MODRM_T modrm;

#define ACASE    goto again; case
#define BCASE    break; case
#define RCASE    return; case

void initVM() {
    memBase = 0x08048000; // Default Linux code start address
    ESP = memBase + VM_SZ;
    EBPbase = ESP-(DSTK_SZ*4);
    EBP = EBPbase - 4;
    ip = 0;
    here = memBase;
}

static void    s1(int32_t a, int32_t v) { vm[a-memBase] = (v&0xff); }
static void    s2(int32_t a, int32_t v) { *(int16_t*)(&vm[a-memBase]) = (v&0xffff); }
static void    s4(int32_t a, int32_t v) { *(int32_t*)(&vm[a-memBase]) = v; }
static int32_t f1(int32_t a) { return vm[a-memBase]; }
static int32_t f2(int32_t a) { return *(int16_t*)(&vm[a-memBase]); }
static int32_t f4(int32_t a) { return *(int32_t*)(&vm[a-memBase]); }
static int32_t ip4() { int32_t x = f4(ip); ip += 4; return x; }

// mod/rm
void toModRM() {
    uint8_t v = f1(ip++);
    modrm.val = v;
    modrm.mod = (v >> 6) & 0x03;  // bits 6-7 - mode
    modrm.r =   (v >> 3) & 0x07;  // bits 3-5 - reg
    modrm.m =   (v >> 0) & 0x07;  // bits 0-2 - reg/mem
#ifdef _DEBUG_
    printf(" ModRM: (%02x) mod=%d r=%d m=%d", v, modrm.mod, modrm.r, modrm.m);
#endif // _DEBUG_
}


void push(uint32_t v) { ESP -= 4; s4(ESP, v); }
uint32_t pop() { ESP += 4; return f4(ESP-4); }

void uOP() { printf("\n-opcode:%02X/%d?-", ir, ir); }
void op00() { uOP(); }
void op01() { toModRM(); // ADD
    if (modrm.val == 0xd8) { DBG("ADD EBX"); EAX += EBX; }
}
void op02() { uOP(); }
void op03() { uOP(); }
void op04() { uOP(); }
void op05() { uOP(); }
void op06() { uOP(); }
void op07() { uOP(); }
void op08() { uOP(); }
void op09() { toModRM(); // OR
    if (modrm.val == 0xd8) { DBG("AND EBX"); EAX |= EBX; }
}
void op0A() { uOP(); }
void op0B() { uOP(); }
void op0C() { uOP(); }
void op0D() { uOP(); }
void op0E() { uOP(); }
void op0F() { uOP(); }
void op10() { uOP(); }
void op11() { uOP(); }
void op12() { uOP(); }
void op13() { uOP(); }
void op14() { uOP(); }
void op15() { uOP(); }
void op16() { uOP(); }
void op17() { uOP(); }
void op18() { uOP(); }
void op19() { uOP(); }
void op1A() { uOP(); }
void op1B() { uOP(); }
void op1C() { uOP(); }
void op1D() { uOP(); }
void op1E() { uOP(); }
void op1F() { uOP(); }
void op20() { uOP(); }
void op21() { toModRM(); // AND
    if (modrm.val == 0xd8) { DBG("AND EBX"); EAX &= EBX; }
}
void op22() { uOP(); }
void op23() { uOP(); }
void op24() { uOP(); }
void op25() { uOP(); }
void op26() { uOP(); }
void op27() { uOP(); }
void op28() { uOP(); }
void op29() { toModRM(); // SUB
    if (modrm.val == 0xd8) { DBG("SUB EBX"); EAX -= EBX; }
}
void op2A() { uOP(); }
void op2B() { uOP(); }
void op2C() { uOP(); }
void op2D() { uOP(); }
void op2E() { uOP(); }
void op2F() { uOP(); }
void op30() { uOP(); }
void op31() { toModRM(); // XOR
    if (modrm.val == 0xd8) { DBG("XOR EBX"); EAX ^= EBX; }
}
void op32() { uOP(); }
void op33() { uOP(); }
void op34() { uOP(); }
void op35() { uOP(); }
void op36() { uOP(); }
void op37() { uOP(); }
void op38() { uOP(); }
void op39() { uOP(); }
void op3A() { uOP(); }
void op3B() { uOP(); }
void op3C() { uOP(); }
void op3D() { uOP(); }
void op3E() { uOP(); }
void op3F() { uOP(); }
void op40() { uOP(); }
void op41() { uOP(); }
void op42() { uOP(); }
void op43() { uOP(); }
void op44() { uOP(); }
void op45() { uOP(); }
void op46() { uOP(); }
void op47() { uOP(); }
void op48() { uOP(); }
void op49() { uOP(); }
void op4A() { uOP(); }
void op4B() { uOP(); }
void op4C() { uOP(); }
void op4D() { uOP(); }
void op4E() { uOP(); }
void op4F() { uOP(); }
void op50() { DBG("PUSH EAX"); push(EAX); }
void op51() { DBG("PUSH ECX"); push(ECX); }
void op52() { DBG("PUSH EDX"); push(EDX); }
void op53() { DBG("PUSH EBX"); push(EBX); }
void op54() { uOP(); }
void op55() { uOP(); }
void op56() { uOP(); }
void op57() { uOP(); }
void op58() { DBG("POP EAX"); EAX = pop(); }
void op59() { DBG("POP ECX"); ECX = pop(); }
void op5A() { DBG("POP EDX"); EDX = pop(); }
void op5B() { DBG("POP EBX"); EBX = pop(); }
void op5C() { uOP(); }
void op5D() { uOP(); }
void op5E() { uOP(); }
void op5F() { uOP(); }
void op60() { uOP(); }
void op61() { uOP(); }
void op62() { uOP(); }
void op63() { uOP(); }
void op64() { uOP(); }
void op65() { uOP(); }
void op66() { uOP(); }
void op67() { uOP(); }
void op68() { uOP(); }
void op69() { uOP(); }
void op6A() { uOP(); }
void op6B() { uOP(); }
void op6C() { uOP(); }
void op6D() { uOP(); }
void op6E() { uOP(); }
void op6F() { uOP(); }
void op70() { uOP(); }
void op71() { uOP(); }
void op72() { uOP(); }
void op73() { uOP(); }
void op74() { uOP(); }
void op75() { uOP(); }
void op76() { uOP(); }
void op77() { uOP(); }
void op78() { uOP(); }
void op79() { uOP(); }
void op7A() { uOP(); }
void op7B() { uOP(); }
void op7C() { uOP(); }
void op7D() { uOP(); }
void op7E() { uOP(); }
void op7F() { uOP(); }
void op80() { uOP(); }
void op81() { uOP(); }
void op82() { uOP(); }
void op83() { toModRM();
    if (modrm.val == 0x45) { DBG1("MOV EAX #", f4(ip)); EAX += ip4(); }
    else if (modrm.val == 0xc5) { DBG1("ADD EBP #", f1(ip)); EBP += f1(ip++); }
    else if (modrm.val == 0xed) { DBG1("SUB EBP #", f1(ip)); EBP -= f1(ip++); }
}
void op84() { uOP(); }
void op85() { uOP(); }
void op86() { uOP(); }
void op87() { toModRM();  // xchg reg1, reg2;
    arg1 = reg[modrm.m];
    reg[modrm.m] = reg[modrm.r];
    reg[modrm.r] = arg1;
}
void op88() { uOP(); }
void op89() { toModRM();
    if (modrm.val == 0x45) { DBG("MOV [EBP], EAX"); s4(EBP, EAX); ip++; }
    else if (modrm.val == 0xc5) { DBG1("???ADD EBP #", f1(ip)); EBP += f1(ip++); }
    else if (modrm.val == 0xc3) { DBG("MOV EBX, EAX"); EBX = EAX; }
    // EAX = f4(EBP); ip += 2;
}
void op8A() { uOP(); }
void op8B() { toModRM();
    if (modrm.val == 0x45) { DBG("MOV EAX, [EBP]"); EAX = f4(EBP); ip++; }
    // s4(EBP, EAX); ip += 2;
}  // mov mem/reg to REG - 8B 45 00 - mov eax, [ebp]
void op8C() { uOP(); }
void op8D() { uOP(); }
void op8E() { uOP(); }
void op8F() { uOP(); }
void op90() { uOP(); }
void op91() { uOP(); }
void op92() { uOP(); }
void op93() { uOP(); }
void op94() { uOP(); }
void op95() { uOP(); }
void op96() { uOP(); }
void op97() { uOP(); }
void op98() { uOP(); }
void op99() { uOP(); }
void op9A() { uOP(); }
void op9B() { uOP(); }
void op9C() { uOP(); }
void op9D() { uOP(); }
void op9E() { uOP(); }
void op9F() { uOP(); }
void opA0() { EAX = f1(ip4()); } // mov AL, [mem]
void opA1() { EAX = f4(ip4()); } // mov EAX, [mem]
void opA2() { s1(ip4(), EAX); }  // mov [mem], AL
void opA3() { s4(ip4(), EAX); }  // mov [mem], EAX
void opA4() { uOP(); }
void opA5() { uOP(); }
void opA6() { uOP(); }
void opA7() { uOP(); }
void opA8() { uOP(); }
void opA9() { uOP(); }
void opAA() { uOP(); }
void opAB() { uOP(); }
void opAC() { uOP(); }
void opAD() { uOP(); }
void opAE() { uOP(); }
void opAF() { uOP(); }
void opB0() { uOP(); }
void opB1() { uOP(); }
void opB2() { uOP(); }
void opB3() { uOP(); }
void opB4() { uOP(); }
void opB5() { uOP(); }
void opB6() { uOP(); }
void opB7() { uOP(); }
void opB8() { DBG1("MOV EAX #", f4(ip));  EAX = ip4(); }  // mov EAX, <imm>
void opB9() { DBG1("MOV ECX #", f4(ip));  ECX = ip4(); }  // mov ECX, <imm>
void opBA() { DBG1("MOV EDX #", f4(ip));  EDX = ip4(); }  // mov EDX, <imm>
void opBB() { DBG1("MOV EBX #", f4(ip));  EBX = ip4(); }  // mov EBX, <imm>
void opBC() { DBG1("MOV ESP #", f4(ip));  ESP = ip4(); }  // mov ESP, <imm>
void opBD() { DBG1("MOV EBP #", f4(ip));  EBP = ip4(); }  // mov EBP, <imm>
void opBE() { DBG1("MOV ESI #", f4(ip));  ESI = ip4(); }  // mov ESI, <imm>
void opBF() { DBG1("MOV EDI #", f4(ip));  EDI = ip4(); }  // mov EDI, <imm>
void opC0() { uOP(); }
void opC1() { uOP(); }
void opC2() { uOP(); }
void opC3() { uOP(); }
void opC4() { uOP(); }
void opC5() { uOP(); }
void opC6() { uOP(); }
void opC7() { uOP(); }
void opC8() { uOP(); }
void opC9() { uOP(); }
void opCA() { uOP(); }
void opCB() { uOP(); }
void opCC() { uOP(); }
void opCD() { uOP(); }
void opCE() { uOP(); }
void opCF() { uOP(); }
void opD0() { uOP(); }
void opD1() { uOP(); }
void opD2() { uOP(); }
void opD3() { uOP(); }
void opD4() { uOP(); }
void opD5() { uOP(); }
void opD6() { uOP(); }
void opD7() { uOP(); }
void opD8() { uOP(); }
void opD9() { uOP(); }
void opDA() { uOP(); }
void opDB() { uOP(); }
void opDC() { uOP(); }
void opDD() { uOP(); }
void opDE() { uOP(); }
void opDF() { uOP(); }
void opE0() { uOP(); }
void opE1() { uOP(); }
void opE2() { uOP(); }
void opE3() { uOP(); }
void opE4() { uOP(); }
void opE5() { uOP(); }
void opE6() { uOP(); }
void opE7() { uOP(); }
void opE8() { uOP(); }
void opE9() { uOP(); }
void opEA() { uOP(); }
void opEB() { uOP(); }
void opEC() { uOP(); }
void opED() { uOP(); }
void opEE() { uOP(); }
void opEF() { uOP(); }
void opF0() { uOP(); }
void opF1() { uOP(); }
void opF2() { uOP(); }
void opF3() { uOP(); }
void opF4() { uOP(); }
void opF5() { uOP(); }
void opF6() { uOP(); }
void opF7() { toModRM();  // IMUL / IDIV
    if (modrm.val == 0xeb) { DBG("IMUL EBX"); EAX *= EBX; }
    else if (modrm.val == 0xfb) { DBG("IDIV EBX"); EAX /= EBX; }
}
void opF8() { uOP(); }
void opF9() { uOP(); }
void opFA() { uOP(); }
void opFB() { uOP(); }
void opFC() { uOP(); }
void opFD() { uOP(); }
void opFE() { uOP(); }
void opFF() { uOP(); }

voidfn_t opcodes[256] = {
op00, op01, op02, op03, op04, op05, op06, op07, op08, op09, op0A, op0B, op0C, op0D, op0E, op0F,
op10, op11, op12, op13, op14, op15, op16, op17, op18, op19, op1A, op1B, op1C, op1D, op1E, op1F,
op20, op21, op22, op23, op24, op25, op26, op27, op28, op29, op2A, op2B, op2C, op2D, op2E, op2F,
op30, op31, op32, op33, op34, op35, op36, op37, op38, op39, op3A, op3B, op3C, op3D, op3E, op3F,
op40, op41, op42, op43, op44, op45, op46, op47, op48, op49, op4A, op4B, op4C, op4D, op4E, op4F,
op50, op51, op52, op53, op54, op55, op56, op57, op58, op59, op5A, op5B, op5C, op5D, op5E, op5F,
op60, op61, op62, op63, op64, op65, op66, op67, op68, op69, op6A, op6B, op6C, op6D, op6E, op6F,
op70, op71, op72, op73, op74, op75, op76, op77, op78, op79, op7A, op7B, op7C, op7D, op7E, op7F,
op80, op81, op82, op83, op84, op85, op86, op87, op88, op89, op8A, op8B, op8C, op8D, op8E, op8F,
op90, op91, op92, op93, op94, op95, op96, op97, op98, op99, op9A, op9B, op9C, op9D, op9E, op9F,
opA0, opA1, opA2, opA3, opA4, opA5, opA6, opA7, opA8, opA9, opAA, opAB, opAC, opAD, opAE, opAF,
opB0, opB1, opB2, opB3, opB4, opB5, opB6, opB7, opB8, opB9, opBA, opBB, opBC, opBD, opBE, opBF,
opC0, opC1, opC2, opC3, opC4, opC5, opC6, opC7, opC8, opC9, opCA, opCB, opCC, opCD, opCE, opCF,
opD0, opD1, opD2, opD3, opD4, opD5, opD6, opD7, opD8, opD9, opDA, opDB, opDC, opDD, opDE, opDF,
opE0, opE1, opE2, opE3, opE4, opE5, opE6, opE7, opE8, opE9, opEA, opEB, opEC, opED, opEE, opEF,
opF0, opF1, opF2, opF3, opF4, opF5, opF6, opF7, opF8, opF9, opFA, opFB, opFC, opFD, opFE, opFF
};

void GotoRC(int r, int c) { printf("\x1B[%d;%dH", r, c); }

void seeCPU() {
    char *x[8] = {"EAX", "ECX" , "EDX" , "EBX" , "ESP" , "EBP" , "ESI" , "EDI" };
    int y[8] = { 0,3,1,2,4,5,6,7 };
    char *clr = "\x1B[K";
    printf("\x1B[s");
    for (int i = 0; i < 8; i++) {
        GotoRC(i+1, 90);
        printf("%s: 0x%x/%d%s", x[y[i]], reg[y[i]], reg[y[i]], clr);
    }
    GotoRC(10, 90); printf(" IP: 0x%x/%d%s", ip, ip, clr);
    GotoRC(11, 90); printf(" IR: 0x%x/%d%s", f1(ip), f1(ip), clr);
    GotoRC(12, 90); printf("  H: 0x%x/%d%s", here, here, clr);
    int depth = (EBP<EBPbase) ? 0 : (EBP - EBPbase) / 4;
    GotoRC(12, 90); printf("TOS: 0x%x (%d)%s", f4(EBP), depth, clr);
    printf("\x1B[u");

    if (BTWI(ip, 0, here)) {
        y[0]++; // break here if wanted
    }
}

void runCPU(uint32_t st) {
    ip = st;
    while (ip < here) {
        if (dbg) { seeCPU(); }
        ir = f1(ip++);
        if (dbg) { printf("\nIP: %08lX, IR: %02x ", ip - 1, ir); }
        opcodes[ir]();
    }
    if (dbg) { seeCPU(); printf("\n"); }
}

void g1(int n) { s1(here++, n); }
void g2(int n) { g1(n); g1(n >> 8); }
void g4(int n) { g2(n); g2(n >> 16); }
void gN(int n, uint8_t* bytes) { for (int i = 0; i < n; i++) { g1(bytes[i]); } }

// Generate code to push a value
// https://yozan233.github.io/Online-Assembler-Disassembler/
void gPush(int val) {
    gN(5, "\x87\xe5\x50\x87\xe5");  // xchg ebp, esp; push eax; xchg ebp, esp
    // gN(3, "\x83\xc5\x04");  // add ebp, 4
    // gN(3, "\x89\x45\x00");  // mov [ebp], eax
    g1(0xb8); g4(val);      // mov eax, <val>
}

// Generate code to drop TOS
void gPop() {
    gN(3, "\x8b\x45\x00");  // mov eax, [ebp]
    gN(3, "\x83\xc5\x04");  // sub ebp, 4
}

void gMath(int op, int modrm) {
    gN(2, "\x89\xC3");      // mov ebx, eax
    gN(3, "\x8b\x45\x00");  // mov eax, [ebp]
    g1(op); g1(modrm);      // -see below-
    gN(3, "\x83\xED\x04");  // sub ebp, 4
}

void gAdd() { gMath(0x01, 0xd8); }  // add  : op01, mod/rm=xd8
void gSub() { gMath(0x29, 0xd8); }  // sub  : op29, mod/rm=xd8
void gMul() { gMath(0xf7, 0xeb); }  // imul : opf7, mod/rm=xeb
void gDiv() { gMath(0xf7, 0xfb); }  // idiv : opf7, mod/rm=xfb
void gAnd() { gMath(0x21, 0xd8); }  // and  : op21, mod/rm=xd8
void gOr()  { gMath(0x09, 0xd8); }  // or   : op09, mod/rm=xd8
void gXor() { gMath(0x31, 0xd8); }  // xor  : op31, mod/rm=xd8

void assert(int32_t exp, char *msg) {
    runCPU(memBase);
    if (EAX != exp) { printf("\nFAIL: %s, expected %d, got %d", msg, exp, EAX); }
    else { printf("\nPASS: %s", msg); }
    here = memBase; ESP = EBPbase;
}

void runTests() {
    gPush(0x4); gPush(0x5); gAdd(); assert( 9, "add");
    gPush(0x7); gPush(0x8); gSub(); assert(-1, "sub");
    gPush(0x6); gPush(0x6); gMul(); assert(36, "mult");
    gPush(0x9); gPush(0x2); gDiv(); assert( 4, "div");
    gPush(0xa); gPush(0x7); gAnd(); assert( 2, "and");
    gPush(0x1); gPush(0x2); gOr();  assert( 3, "or");
    gPush(0xa); gPush(0xa); gXor(); assert( 0, "xor");
}

/*---------------------------------------------------------------------------*/
/* Main program. */

int main(int argc, char *argv[]) {
    char *fn = (argc > 1) ? argv[1] : "tc.out";
    FILE *fp = fopen(fn, "rb");
    initVM();
    runTests();
    return 0;
    if (!fp) { printf("can't open program"); }
    else {
        here = (int)fread(vm, 1, VM_SZ, fp);
        fclose(fp);
        //fp = fopen("vm-lin.lst", "wt");
        //fclose(fp);
        // runCPU(0);
    }
    return 0;
}
