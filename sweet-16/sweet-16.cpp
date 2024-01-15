// sweet-16.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>

typedef unsigned char byte;

byte mem[65536];
short reg[16];
byte carry;
byte zflg;
byte pos;
byte neg1;

#define WORDAT(loc) (*(short *)&mem[loc])
#define RTN   0x0
#define SET   0x10
#define LD    0x20
#define ST    0x30
#define LDI   0x40
#define STI   0x50
#define LDD   0x60
#define STD   0x70
#define POP   0x80
#define STP   0x90
#define ADD   0xA0
#define SUB   0xB0
#define POPD  0xC0
#define CPR   0xD0
#define INR   0xE0
#define DCR   0xF0

short setFlags(short val, int cf) {
    carry = cf;
    zflg = (val == 0) ? 1 : 0;
    neg1 = (val == -1) ? 1 : 0;
    pos = (val <= 0) ? 1 : 0;
    return val & 0xFFFF;
}

short doBranch(byte hi, byte lo, short pc) {
    return pc;
}

void run(short start) {
    reg[15] = start;
    long x;
    while (1) {
        byte ir = mem[reg[15]];
        // byte hi = ir & 0xF0;
        byte lo = ir & 0x0F;
        switch (ir & 0xF0) {
        case 0: // BRANCH
            if (lo == 0) { return; }
            reg[15] = doBranch((ir >> 4), lo, reg[15]);
            break;
        case SET: // SET
            reg[lo] = mem[reg[15]+1] | (mem[reg[15]+2] << 8);
            setFlags(reg[lo], 0);
            reg[15] += 2;
            break;
        case LD: // LOAD
            reg[0] = setFlags(reg[lo], 0);
            break;
        case ST: // STORE
            reg[lo] = setFlags(reg[0], 0);
            break;
        case LDI: // LOAD INDIRECT
            reg[0] = mem[reg[lo]];
            reg[lo]++;
            setFlags(reg[0], 0);
            break;
        case STI: // STORE INDIRECT
            mem[reg[lo]++] = reg[0] & 0xFF;
            reg[lo]++;
            setFlags(reg[0], 0);
            break;
        case LDD: // LOAD DOUBLE INDIRECT
            reg[0] = mem[reg[lo]] | (mem[reg[lo] + 1] << 8);
            setFlags(reg[0], 0);
            reg[lo] += 2;
            break;
        case STD: // STORE DOUBLE INDIRECT
            mem[reg[lo]] = reg[0] & 0xFF;
            mem[reg[lo]+1] = reg[0] >> 8;
            setFlags(reg[0], 0);
            reg[lo] += 2;
            break;
        case POP: // POP
            --reg[lo];
            reg[0] = mem[reg[lo]];
            setFlags(reg[0], 0);
            break;
        case STP: // STP
            --reg[lo];
            mem[reg[lo]] = reg[0] & 0xFF;
            setFlags(reg[0], 0);
            break;
        case ADD: // ADD
            x = (long)reg[0] + (long)reg[lo];
            reg[0] = setFlags((short)x, (x >> 16) ? 1 : 0);
            break;
        case SUB: // SUB
            x = (~reg[lo]) + 1;
            x += reg[0];
            reg[0] = setFlags((short)x, (x >> 16) ? 1 : 0);
            break;
        case POPD: // POPD
            reg[0] = mem[reg[lo-2]] | (mem[reg[lo-1]] << 8);
            setFlags(reg[0], 0);
            reg[lo] -= 2;
            break;
        case CPR: // CPR
            setFlags(reg[0] - reg[lo], (reg[0] < reg[lo]) ? 1  : 0);
            break;
        case INR: // INR
            reg[lo] = setFlags(reg[lo] + 1, 0);
            break;
        case DCR: // DCR
            reg[lo] = setFlags(reg[lo] - 1, 0);
            break;
        }
        ++reg[15];
    }
}

void asm1(byte v) { mem[reg[14]++] = v; }
void asm2(short v) { asm1(v & 0xFF); asm1(v >> 8); }
void asm3(short op, short reg) { asm1(op | reg); }

int main()
{
    reg[14] = 0;
    asm3(SET, 1);
    asm2(8);
    asm3(SET, 2);
    asm2(7);
    asm3(LD, 1);
    asm3(SUB, 2);
    asm3(ST, 3);
    asm3(SET, 0);
    asm2(255);
    asm1(RTN);
    run(0);
    for (int i = 0; i < reg[14]; i++) { printf("%d: [%02x] ", i, mem[i]); }
    for (int i = 0; i < 16; i++) { printf("\nR%-2X: %d", i, reg[i]); }
}
