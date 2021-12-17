// J2 is a 32-bit derivative of J1, whose white paper is here: https://excamera.com/files/j1.pdf

#include "j2.h"

CELL DSP, RSP, PC = 0;
CELL dstk[STK_SZ+1], rstk[STK_SZ+1], the_memory[MEM_SZ];

void push(CELL val) { if (DSP < STK_SZ) { dstk[++DSP] = val; } }
CELL pop() { return (DSP) ? dstk[DSP--] : 0; }
void j2_init() { DSP = RSP = PC = 0; }

void j2_emu(CELL start) {
	PC = start;
	RSP = 0;
	while (0 <= RSP) {
        CELL IR = the_memory[PC++];
		if ((IR & opLIT) == opLIT) {
			push(IR & 0x7FFF);
		}
		if ((IR & INSTR_MASK) == opJMP) {
			PC = IR & ADDR_MASK;
		}
		if ((IR & INSTR_MASK) == opJMPZ) {
			PC = (T == 0) ? (IR & ADDR_MASK) : PC;
			pop();
		}
		if ((IR & INSTR_MASK) == opCALL) {
			if (RSP < STK_SZ) { rstk[++RSP] = PC; }
			PC = (IR & ADDR_MASK);
		}
		if ((IR & INSTR_MASK) == opALU) {
			CELL currentT = T;
			CELL currentN = N;
			CELL currentR = R;
			CELL newT = 0;

			switch ((IR & 0x0F00) >> 8) {
			case tpTgetsT:     newT = T;                            break;
			case tpTgetsN:     newT = N;                            break;
			case tpTplusN:     newT = (T + N);                      break;
			case tpTandN:      newT = (T & N);                      break;
			case tpTorN:       newT = (T | N);                      break;
			case tpTxorN:      newT = (T ^ N);                      break;
			case tpNotT:       newT = (~T);                         break;
			case tpTeqN:       newT = (N == T) ? 1 : 0;             break;
			case tpTltN:       newT = (N < T) ? 1 : 0;              break;
			case tpSHR:        newT = (N >> T);                     break;
			case tpDecT:       newT = (T - 1);                      break;
			case tpTgetsR:     newT = R;                            break;
			case tpFetch:      newT = the_memory[T];                break;
			case tpSHL:        newT = (N << T);                     break;
			case tpDepth:      newT = DSP;                          break;
			case tpNuLtT:      newT = (UCELL)N < (UCELL)T ? 1 : 0;  break;
			}

			if (IR & bitIncRSP) { RSP += (RSP < STK_SZ) ? 1 : 0; }
			if (IR & bitDecRSP) { RSP -= 1; }
			if (IR & bitIncDSP) { DSP += (DSP < STK_SZ) ? 1 : 0; }
			if (IR & bitDecDSP) { DSP -= (0 < DSP) ? 1 : 0; }

			if (IR & bitRtoPC) { PC = currentR; }
			if (IR & bitTtoR) { R = currentT; }
			if (IR & bitTtoN) { N = currentT; }
			if ((IR & bitStore)) {
				if ((0 <= currentT) && (currentT < MEM_SZ)) { the_memory[currentT] = currentN; }
				else { writePort(currentT, currentN); }
			}
			T = newT;
		}
	}
}
