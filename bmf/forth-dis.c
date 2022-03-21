#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Shared.h"
#include "forth-vm.h"

CELL ORG = 0x0040;

char input_fn[64];
char output_fn[64];
FILE *input_fp = NULL;
FILE *output_fp = NULL;
CELL PC;
BYTE IR;

CELL ADDR_CELL     = 0x08;
CELL ADDR_HERE     = 0x10;
CELL ADDR_LAST     = 0x14;
CELL ADDR_BASE     = 0x18;
CELL ADDR_INPUTFP  = 0x1C;
CELL ADDR_STATE    = 0x20;
CELL ADDR_MEM_SZ   = 0x24;

// ------------------------------------------------------------------------------------------
void dis_range(CELL start, CELL end, char *bytes)
{
	char x[8];
	while (start <= end)
	{
		BYTE val = the_memory[start++];
		sprintf(x, " %02x", (int)val);
		strcat(bytes, x);
	}
}

// ------------------------------------------------------------------------------------------
void dis_start(CELL start, int num, char *bytes)
{
    dis_range(start, start + num - 1, bytes);
}

// ------------------------------------------------------------------------------------------
void dis_rangeCell(CELL start, CELL end, char *bytes)
{
	char x[8];
	while (start <= end)
	{
		CELL val = CELL_AT(start);
		start += 4;
		sprintf(x, " %08lx", (CELL)val);
		strcat(bytes, x);
	}
}

// ------------------------------------------------------------------------------------------
void dis_PC2(int num, char *bytes)
{
	char x[8];
	for (int i = 0; i < num; i++)
	{
		BYTE val = the_memory[PC++];
		sprintf(x, " %02x", (int)val);
		strcat(bytes, x);
	}
}

// ------------------------------------------------------------------------------------------
CELL GetSysVarAddr(char *name)
{
	// printf("Looking for [%s] ...\n", name);
	CELL addr = ORG;
	CELL def_sz = 0x0f;
	while ( addr < 0x1000 )
	{
		if ( the_memory[addr] != DICTP )
		{
			printf("[%s] NOT FOUND!\n", name);
			return 0;
		}
		CELL tmp = CELL_AT(addr+1);
		DICT_T *dp = (DICT_T *)&the_memory[tmp];
		// printf("0x%04lX, 0x%04lX, [%s] = [%s]?\n", addr, dp, name, dp->name);
		if (strcmp(dp->name, name) == 0)
		{
			tmp = addr + 11;
			// printf("[%s] FOUND at 0x%04lx\n", name, tmp);
			return tmp;
		}
		addr += def_sz;
	}
	return 0;
}

// ------------------------------------------------------------------------------------------
CELL dis_vars(FILE *write_to)
{
	CELL addr = ORG;
	char bytes[256];
	char desc[256];
	CELL def_sz = 0x0f;
	while ( true )
	{
		if ( the_memory[addr] != DICTP )
		{
			return addr;
		}
		CELL tmp = CELL_AT(addr+1);
		CELL tmp2;
		DICT_T *dp = (DICT_T *)&the_memory[tmp];

		sprintf(bytes, "%04lx:", addr);
		dis_start(addr, 5, bytes);
		sprintf(desc, "DICTP %s (%04lx)", dp->name, tmp);
		fprintf(write_to, "%-32s ; %s\n", bytes, desc);

		tmp = addr+5;
		tmp2 = CELL_AT(tmp+1);
		sprintf(bytes, "%04lx:", tmp);
		dis_start(tmp, 5, bytes);
		sprintf(desc, "LITERAL %d (0x%04lx)", tmp2, tmp2);
		fprintf(write_to, "%-32s ; %s\n", bytes, desc);

		tmp = tmp+5;
		sprintf(bytes, "%04lx:", tmp);
		dis_start(tmp, 1, bytes);
		sprintf(desc, "RET");
		fprintf(write_to, "%-32s ; %s\n", bytes, desc);

		tmp = tmp+1;
		tmp2 = CELL_AT(tmp);
		sprintf(bytes, "%04lx:", tmp);
		dis_start(tmp, 4, bytes);
		sprintf(desc, "dw 0x%04lx", tmp2);
		fprintf(write_to, "%-32s ; %s\n;\n", bytes, desc);

		addr += def_sz;
	}
	return 0;
}

// ------------------------------------------------------------------------------------------
// Where all the work is done
// ------------------------------------------------------------------------------------------
CELL dis_one(char *bytes, char *desc)
{
	IR = the_memory[PC];
	sprintf(bytes, "%04lx: %02x", PC, (int)IR);
	++PC;

	switch (IR)
	{
	case LITERAL:
		arg1 = CELL_AT(PC);
		dis_PC2(CELL_SZ, bytes);
		sprintf(desc, "LITERAL %ld (%0lx)", arg1, arg1);
		return CELL_SZ;

	case CLITERAL:
		arg1 = the_memory[PC];
		dis_PC2(1, bytes);
		sprintf(desc, "CLITERAL %ld", arg1);
		return 1;

	case FETCH:
		sprintf(desc, "FETCH");
		return 0;

	case STORE:
		sprintf(desc, "STORE");
		return 0;

	case SWAP:
		sprintf(desc, "SWAP");
		return 0;

	case DROP:
		sprintf(desc, "DROP");
		return 0;

	case DUP:
		sprintf(desc, "DUP");
		return 0;

	case OVER:
		sprintf(desc, "OVER");
		return 0;

	case PICK:
		sprintf(desc, "PICK");
		return 0;

	case JMP:
		arg1 = CELL_AT(PC);
		sprintf(desc, "JMP %04lx", arg1);
		if (the_memory[arg1] == DICTP)
		{
			arg2 = CELL_AT(arg1+1);
			DICT_T *dp = (DICT_T *)&(the_memory[arg2]);
			sprintf(desc, "JMP %s (%04lx)\n;", dp->name, arg1);
		}
		dis_PC2(CELL_SZ, bytes);
		return CELL_SZ;

	case JMPZ:
		sprintf(desc, "JMPZ %04lx", CELL_AT(PC));
		dis_PC2(CELL_SZ, bytes);
		return CELL_SZ;

	case JMPNZ:
		sprintf(desc, "JMPNZ %04lx", CELL_AT(PC));
		dis_PC2(CELL_SZ, bytes);
		return CELL_SZ;

	case CALL:
		arg1 = CELL_AT(PC);
		arg2 = CELL_AT(arg1+1);
		DICT_T *dp = (DICT_T *)&(the_memory[arg2]);
		sprintf(desc, "CALL %s (%04lx)", dp->name, arg1);
		dis_PC2(CELL_SZ, bytes);
		return CELL_SZ;

	case RET:
		sprintf(desc, "RET");
		if (the_memory[PC] == DICTP)
		{
			strcat(desc, "\n;");
		}
		return 0;

	case COMPARE:
		sprintf(desc, "COMPARE");
		return 0;

	case COMPAREI:
		sprintf(desc, "COMPAREI");
		return 0;

	case SLASHMOD:
		sprintf(desc, "SLASHMOD");
		return 0;

	case SLITERAL:
		// count, bytes, NULL - NULL delimited counted string
		// 0100 0101 0102 0103 0104 0105
		//   07   03   65   66   67   00
		// SLIT   03    A    B    C   00
		// PC starts at 0101, should be set to 0106

		arg1 = the_memory[PC]; // count-byte (and the beginning of the counted string)
		arg2 = arg1 + 2;  // count-byte + count + NULL
		sprintf(desc, "SLITERAL (%04lx) [%s]", PC, (char *)&the_memory[PC+1]);
		dis_PC2(arg2, bytes);
		return PC-arg1;

	case CFETCH:
		sprintf(desc, "CFETCH");
		return 0;

	case CSTORE:
		sprintf(desc, "CSTORE");
		return 0;

	case ADD:
		sprintf(desc, "ADD");
		return 0;

	case SUB:
		sprintf(desc, "SUB");
		return 0;

	case MUL:
		sprintf(desc, "MUL");
		return 0;

	case DIV:
		sprintf(desc, "DIV");
		return 0;

	case LT:
		sprintf(desc, "LT");
		return 0;

	case EQ:
		sprintf(desc, "EQ");
		return 0;

	case GT:
		sprintf(desc, "GT");
		return 0;

	case DICTP:
		arg1 = CELL_AT(PC);
		sprintf(desc, "DICTP %s (%04lx)", &(the_memory[arg1+10]), arg1);
		dis_PC2(CELL_SZ, bytes);
		return CELL_SZ;

	case EMIT:
		sprintf(desc, "EMIT");
		return 0;

	case FOPEN:
		sprintf(desc, "FOPEN");
		return 0;

	case FREAD:			// ( addr num fp -- count ) - fp == 0 means STDIN
		sprintf(desc, "FREAD");
		return 0;

	case FREADLINE:
		sprintf(desc, "FREADLINE");
		return 0;

	case FWRITE:
		sprintf(desc, "FWRITE");
		return 0;

	case FCLOSE:
		sprintf(desc, "FCLOSE");
		return 0;

	case GETCH:
		sprintf(desc, "GETCH");
		return 0;

	case DTOR:
		sprintf(desc, "DTOR");
		return 0;

	case RTOD:
		sprintf(desc, "RTOD");
		return 0;

	case DEPTH:
		sprintf(desc, "DEPTH");
		return 0;

	case AND:
		sprintf(desc, "AND");
		return 0;

	case OR:
		sprintf(desc, "OR");
		return 0;

	case NOT:
		sprintf(desc, "NOT");
		return 0;

	case RFETCH:
		sprintf(desc, "RFETCH");
		return 0;

	case INC:
		sprintf(desc, "INC");
		return 0;

	case RDEPTH:
		sprintf(desc, "RDEPTH");
		return 0;

	case DEC:
		sprintf(desc, "DEC");
		return 0;

	case GETTICK:
		sprintf(desc, "GETTICK");
		return 0;

	case SHIFTLEFT:
		sprintf(desc, "SHIFTLEFT");
		return 0;

	case SHIFTRIGHT:
		sprintf(desc, "SHIFTRIGHT");
		return 0;

	case PLUSSTORE:
		sprintf(desc, "PLUSSTORE");
		return 0;

	case NOP:
		sprintf(desc, "NOP");
		return 0;

	case BREAK:
		sprintf(desc, "BREAK");
		return 0;

	case BYE:
		sprintf(desc, "BYE");
		return 0;

	case RESET:
	default:
		sprintf(desc, "RESET");
		return 0;
	}
	return 0;
}

// ------------------------------------------------------------------------------------------
void dis_dict(FILE *write_to, CELL dict_addr)
{
	char bytes[128], desc[128];
	DICT_T *dp = (DICT_T *)&the_memory[dict_addr];
	DICT_T *next_dp = (DICT_T *)&the_memory[dp->next];
	CELL addr = dict_addr;

	if (dp->next == 0)
	{
		sprintf(bytes, "%04lx:", addr);
		dis_start(addr, CELL_SZ, bytes);
		fprintf(write_to, "%-32s ; End.\n", bytes);
		return;
	}

	// Next
	sprintf(bytes, "%04lx:", addr);
	dis_start(addr, CELL_SZ, bytes);
	sprintf(desc, "%s - (next: %04lx %s)", dp->name, dp->next, (next_dp->next > 0) ? next_dp->name : "<end>");
	fprintf(write_to, "%-32s ; %s\n", bytes, desc);
	addr += CELL_SZ;

	// XT, Flags
	sprintf(bytes, "%04lx:", addr);
	dis_start(addr, CELL_SZ+1, bytes);
	sprintf(desc, "XT=%04lx, flags=%02x", dp->XT, dp->flags);
	fprintf(write_to, "%-32s ; %s\n", bytes, desc);
	addr += CELL_SZ+1;

	// Name
	sprintf(bytes, "%04lx: %02x", addr++, dp->len);
	dis_start(addr, dp->len+1, bytes);
	sprintf(desc, "%d, %s", (int)dp->len, dp->name);
	fprintf(write_to, "%-32s ; %s\n;\n", bytes, desc);
}

// ------------------------------------------------------------------------------------------
void dis_vm(FILE *write_to)
{
	int here = CELL_AT(ADDR_HERE);
	char bytes[128], desc[128];

	// Initial JMP
	PC = 0;
	dis_one(bytes, desc);
	fprintf(write_to, "%-32s ; %s\n", bytes, desc);

	sprintf(bytes, "0005:");
	dis_range(5, 7, bytes);
	fprintf(write_to, "%s\n", bytes);

	sprintf(bytes, "0008:");
	dis_rangeCell(0x08, 0x0f, bytes);
	fprintf(write_to, "%s\n", bytes);

	PC = 0x10;
	while (PC < ORG)
	{
		sprintf(bytes, "%04lx:", PC);
		dis_rangeCell(PC, PC+0x0f, bytes);
		fprintf(write_to, "%s\n", bytes);
		PC += 0x10;
	}

	fprintf(write_to, ";\n", bytes);
	fflush(write_to);

	// Code
	PC = dis_vars(write_to);

	while (PC < here)
	{
		dis_one(bytes, desc);
		fprintf(write_to, "%-32s ; %s\n", bytes, desc);
		fflush(write_to);
	}

	fprintf(write_to, ";\n; End of code, Dictionary:\n;\n");

	// Dictionary
	PC = CELL_AT(ADDR_LAST);
	while (PC > 0)
	{
		dis_dict(write_to, PC);
		fflush(write_to);
		PC = CELL_AT(PC);
	}
}

// *********************************************************************
void load_vm()
{
	printf("loading VM from %s ...", input_fn);
	input_fp = fopen(input_fn, "rb");
	if (!input_fp)
	{
		printf("\ncannot open file %s!", input_fn);
        exit(1);
	}

    fseek(input_fp, 0L, SEEK_END);
    long file_sz = ftell(input_fp);
    TRACE("file_sz: %ld bytes, ", file_sz);
    fseek(input_fp, 0L, SEEK_SET);

    memory_size = file_sz;
    init_vm(file_sz);

	int num_read = fread(the_memory, 1, memory_size, input_fp);
    TRACE("%ld bytes read\n", num_read);
	fclose(input_fp);
	input_fp = NULL;

	ADDR_HERE = GetSysVarAddr("(HERE)");
	ADDR_LAST = GetSysVarAddr("(LAST)");
	ADDR_BASE = GetSysVarAddr("BASE");
	ADDR_STATE = GetSysVarAddr("STATE");
	ADDR_MEM_SZ = GetSysVarAddr("(MEM_SZ)");

	printf(" done.\n");
}

// *********************************************************************
void do_dis()
{
    output_fp = fopen(output_fn, "wt");
    if (!output_fp)
    {
        printf("ERROR: Can't open assembly file.\n");
        return;
    }

    printf("disassembling to file %s... ", output_fn);
    fprintf(output_fp, "; memory-size: %ld bytes, (%04lx hex)\n", memory_size, memory_size);
	dis_vm(output_fp);

    fclose(output_fp);
    output_fp = NULL;

    dis_vm(output_fp);

    printf(" done.\n");
}

// *********************************************************************
void process_arg(char *arg)
{
    if (*arg == 'i') 
    {
        arg = arg+2;
        strcpy(input_fn, arg);
    }
    else if (*arg == 'o') 
    {
        arg = arg+2;
        strcpy(output_fn, arg);
    }
    else if (*arg == '?') 
    {
        printf("usage: forth-compiler [args]\n");
        printf("  -i:inputFile (full or relative path)\n");
        printf("      default inputFile is forth.bin\n");
        printf("  -o:outputFile (full or relative path)\n");
        printf("      default outputFile is forth.lst\n");
        printf("  -? (prints this message)\n");
		exit(0);
    }
    else
    {
        printf("unknown arg '-%s'\n", arg);
    }
}

int main (int argc, char **argv)
{
    strcpy(input_fn, "forth.bin");
	strcpy(output_fn, "forth.lst");

    for (int i = 1; i < argc; i++)
    {
        char *cp = argv[i];
        if (*cp == '-')
        {
            process_arg(++cp);
        }
    }

    load_vm();
	do_dis();

    return 0;
}
