// J2 is a 32-bit derivative of J1, whose white paper is here: https://excamera.com/files/j1.pdf

#include "j2.h"

CELL DSP, RSP, PC = 0;
CELL dstk[STK_SZ+1], rstk[STK_SZ+1], the_memory[MEM_SZ];

void push(CELL val) { if (DSP < STK_SZ) { dstk[++DSP] = val; } }
CELL pop() { return (DSP) ? dstk[DSP--] : 0; }
void j2_init() { DSP = RSP = PC = 0; }

WORD deriveNewT(CELL IR) {
	switch ((IR & 0x0F00) >> 8) {
		case tpTgetsT:     return T;
		case tpTgetsN:     return N;
		case tpTplusN:     return (T + N);
		case tpTandN:      return (T & N);
		case tpTorN:       return (T | N);
		case tpTxorN:      return (T ^ N);
		case tpNotT:       return (~T);
		case tpTeqN:       return (N == T) ? 1 : 0;
		case tpTltN:       return (N < T) ? 1 : 0;
		case tpSHR:        return (N >> T);
		case tpDecT:       return (T-1);
		case tpTgetsR:     return R;
		case tpFetch:      return the_memory[T];
		case tpSHL:        return (N << T);
		case tpDepth:      return DSP;
		case tpNuLtT:      return 0;        // what is (Nu<T) ?
	}
	return 0;
}

void executeALU(CELL IR) {
	CELL currentT = T;
	CELL currentN = N;
	CELL currentR = R;
	CELL newT = deriveNewT(IR);

	if (IR & bitIncRSP) { RSP += (RSP < STK_SZ) ? 1 : 0; }
	if (IR & bitDecRSP) { RSP -= 1; }
	if (IR & bitIncDSP) { DSP += (DSP < STK_SZ) ? 1 : 0; }
	if (IR & bitDecDSP) { DSP -= (0 < DSP)      ? 1 : 0; }

	if (IR & bitRtoPC)  { PC = currentR; }
	if (IR & bitTtoR)   { R  = currentT; }
	if (IR & bitTtoN)   { N  = currentT; }
	if ((IR & bitStore)) {
		if ((0 <= currentT) && (currentT < MEM_SZ)) { the_memory[currentT] = currentN; }
		else { writePort(currentT, currentN); }
	}
	T = newT;
}

void j2_emu(CELL start) {
	PC = start;
	RSP = 0;
	while (0 <= RSP) {
        CELL IR = the_memory[PC++];
		if ((IR & opLIT) == opLIT) {
			push(IR & 0x7FFF);
			continue;
		}
		if ((IR & INSTR_MASK) == opJMP) {
			PC = IR & ADDR_MASK;
			continue;
		}
		if ((IR & INSTR_MASK) == opJMPZ) {
			PC = (T == 0) ? (IR & ADDR_MASK) : PC;
			pop();
		}
		if ((IR & INSTR_MASK) == opCALL) {
			RSP += (RSP < STK_SZ) ? 1 : 0;
			R = PC;
			PC = (IR & ADDR_MASK);
		}
		if ((IR & INSTR_MASK) == opALU) {
			executeALU(IR);
		}
	}
}
