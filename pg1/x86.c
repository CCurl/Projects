/*---------------------------------------------------------------------------*/
/* Virtual machine. */

#include <stdio.h>
#include <stdint.h>

#define BTWI(n,l,h) ((l<=n)&&(n<=h))
typedef void (*voidfn_t)();
typedef struct { uint8_t mod, r, m; } MODRM_T;

#define EAX reg[0]
#define ECX reg[1]
#define EDX reg[2]
#define EBX reg[3]
#define ESP reg[4]
#define EBP reg[5]
#define ESI reg[6]
#define EDI reg[7]

#define VM_SZ 100000
uint8_t vm[VM_SZ];
int32_t reg[8], disp, arg1;
uint32_t ip, here, *src, *tgt;
uint8_t ir, flags[8], dbg = 1, stkDepth;
MODRM_T modrm;

#define ACASE    goto again; case
#define BCASE    break; case
#define RCASE    return; case

static void    s1(int32_t a, int32_t v) { vm[a] = (v&0xff); }
static void    s4(int32_t a, int32_t v) { *(int32_t*)(&vm[a]) = v; }
static int32_t f1(int32_t a) { return vm[a]; }
static int32_t f2(int32_t a) { return *(int16_t*)(&vm[a]); }
static int32_t f4(int32_t a) { return *(int32_t*)(&vm[a]); }
static int32_t ip4() { int32_t x = f4(ip); ip += 4; return x; }

void initVM() {
    ESP = VM_SZ;
    ip = 0;
    here = 0;
    stkDepth = 0;
}


/*     ModR/M    */
void toModRM() {
    uint8_t v = f1(ip++);
    modrm.mod = (v >> 6) & 0x03;  // bits 6-7 - mode
    modrm.r =   (v >> 3) & 0x07;  // bits 3-5 - reg
    modrm.m =   (v >> 0) & 0x07;  // bits 0-2 - reg/mem
    printf("\nModRM: (%02x) mod=%d r=%d m=%d", v, modrm.mod, modrm.r, modrm.m);
}

/*
  Group 1 Opcodes (0x80, 0x81, 0x83):
  values for ".r" in group 1
  000 = ADD
  001 = OR
  010 = ADC (Add with Carry)
  011 = SBB (Subtract with Borrow)
  100 = AND
  101 = SUB
  110 = XOR
  111 = CMP
*/
void Group1(int bits) {
    toModRM();
    tgt = (uint32_t*)&reg[modrm.m];
    if (bits == 8) { arg1 = f1(ip++); }
    else if (bits == 16) { arg1 = f2(ip); ip += 2; }
    else if (bits == 32) { arg1 = f4(ip); ip += 4; }

    switch (modrm.r) {
        case 0: *tgt += arg1; break;
        case 1: *tgt |= arg1; break;
        case 2: *tgt += arg1; break;
        case 3: *tgt -= arg1; break;
        case 4: *tgt &= arg1; break;
        case 5: *tgt -= arg1; break;
        case 6: *tgt ^= arg1; break;
        case 7: *tgt = arg1; break;
    }
}
/*
Group 2 Opcodes 
values for ".r" in group 1
000 = ROL (Rotate Left)
001 = ROR (Rotate Right)
010 = RCL (Rotate Through Carry Left)
011 = RCR (Rotate Through Carry Right)
100 = SHL (Shift Logical Left)
101 = SHR (Shift Logical Right)
110 = (Reserved)
111 = SAR (Shift Arithmetic Right)

Opcode Prefixes:
0xD0: Shift/Rotate by 1
0xD1: Shift/Rotate 16/32-bit by 1
0xD2: Shift/Rotate by CL (8-bit)
0xD3: Shift/Rotate by CL (16/32-bit)
0xC0: Shift/Rotate 8-bit by immediate
0xC1: Shift/Rotate 16/32-bit by immediate
*/

void ModRM() {
    toModRM();

    switch (modrm.mod) {
        case  0: ; // memory, no displacement
        BCASE 1: tgt = (uint32_t*)&reg[modrm.m] + f1(ip++);  // m+8-bit
        BCASE 2: tgt = (uint32_t*)&reg[modrm.m] + ip4();     // m+32-bit
        BCASE 3:  // register to register
            arg1 = reg[modrm.r];
            tgt = (uint32_t*)&reg[modrm.m];
            break;
    }
}

/*    /r     */
void SlashR() {
    toModRM();
}

void uOP() { printf("-opcode:%02X/%d?-", ir, ir); }

void push(uint32_t v) { ESP -= 4; s4(ESP, v); ++stkDepth; }
uint32_t pop() {
    uint32_t v = 0;
    if (0 < stkDepth) {
        --stkDepth;
        v = f4(ESP); ESP += 4;
    }
    return v;
}

void op00() { uOP(); }
void op01() { ModRM(); *tgt += arg1; } // ADD
void op02() { uOP(); }
void op03() { uOP(); }
void op04() { uOP(); }
void op05() { uOP(); }
void op06() { uOP(); }
void op07() { uOP(); }
void op08() { uOP(); }
void op09() { uOP(); }
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
void op21() { uOP(); }
void op22() { uOP(); }
void op23() { uOP(); }
void op24() { uOP(); }
void op25() { uOP(); }
void op26() { uOP(); }
void op27() { uOP(); }
void op28() { uOP(); }
void op29() { uOP(); }
void op2A() { uOP(); }
void op2B() { uOP(); }
void op2C() { uOP(); }
void op2D() { uOP(); }
void op2E() { uOP(); }
void op2F() { uOP(); }
void op30() { uOP(); }
void op31() { ModRM(); *tgt = (*tgt) ^ arg1; }
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
void op50() { push(EAX); }
void op51() { push(ECX); }
void op52() { push(EDX); }
void op53() { push(EBX); }
void op54() { uOP(); }
void op55() { uOP(); }
void op56() { uOP(); }
void op57() { uOP(); }
void op58() { EAX = pop(); }
void op59() { ECX = pop(); }
void op5A() { EDX = pop(); }
void op5B() { EBX = pop(); }
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
void op83() { // imm8, group 1
    Group1(8);
}
void op84() { uOP(); }
void op85() { uOP(); }
void op86() { uOP(); }
void op87() { uOP(); }
void op88() { uOP(); }
void op89() { uOP(); } // mov REG to mem/reg - 89 45 00 - mov [ebp], eax
void op8A() { uOP(); }
void op8B() { uOP(); } // mov mem/reg to REG - 8B 45 00 - mov eax, [ebp]
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
void opA2() { s1(ip4(), EAX); } // mov [mem], AL
void opA3() { s4(ip4(), EAX); } // mov [mem], EAX
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
void opB8() { EAX = f4(ip); ip += 4; }
void opB9() { ECX = f4(ip); ip += 4; }
void opBA() { EDX = f4(ip); ip += 4; }
void opBB() { EBX = f4(ip); ip += 4; }
void opBC() { uOP(); }
void opBD() { uOP(); }
void opBE() { uOP(); }
void opBF() { uOP(); }
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
void opF7() { uOP(); }
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
    GotoRC(12, 90); printf("TOS: 0x%x (%d)%s", f4(ESP), stkDepth, clr);

    if (BTWI(ip, 0, here)) {
        // printf("put breakpoint here ...");
    }
    printf("\x1B[u");
}

void runCPU(uint32_t st) {
    ip = st;
    while (ip < here) {
        if (dbg) { seeCPU(); }
        ir = f1(ip++);
        opcodes[ir]();
    }
    if (dbg) { seeCPU(); printf("\n"); }
}

void g1(int n) {
    vm[here++] = (n & 0xff);
}
#define g2(n) g1(n); g1(n>>8)
#define g4(n) g2(n); g2(n>>16)

void runTests() {
    here = 0;
    g1(0xb8); g4(0x110); // mov eax, 110
    g1(0xbb); g4(0x220); // mov ebx, 220
    g1(0xb9); g4(0x333); // mov ecx, 333
    g1(0xba); g4(0x444); // mov edx, 444
    g1(0x01); g1(0xd8);  // add eax, ebx
    g1(0x01); g1(0xc3);  // add ebx, eax
    g1(0x83); g1(0xec); g1(0x04); // sub esp, 4
    g1(0x83); g1(0xc4); g1(0x04); // add esp, 5
    g1(0x83); g1(0xc5); g1(0x04); // add ebp, 6
    g1(0x83); g1(0xed); g1(0x04); // sub ebp, 7
    g1(0x50); // push EAX
    g1(0x53); // push EBX
    g1(0x51); // push ECX
    g1(0x52); // push EDX
    g1(0x58); // pop EAX
    g1(0x5b); // pop EBX
    g1(0x59); // pop ECX
    g1(0x5a); // pop EDX
    runCPU(0);
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
