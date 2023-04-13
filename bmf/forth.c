#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "Shared.h"
#include "forth-vm.h"
#include "string.h"

CELL ADDR_MEM_SZ   = 0x24;

char input_fn[256];
FILE *input_fp = NULL;

// *********************************************************************
bool load_vm()
{
	TRACE("loading VM from %s ...", input_fn);
	input_fp = fopen(input_fn, "rb");
	if (!input_fp)
	{
		printf("cannot open file %s", input_fn);
		return false;
	}

    fseek(input_fp, 0L, SEEK_END);
    long file_sz = ftell(input_fp);
    TRACE("file_sz: %ld bytes, ", file_sz);
    fseek(input_fp, 0L, SEEK_SET);

    int memory_size = file_sz;
    init_vm(file_sz);

	int num_read = fread(the_memory, 1, memory_size, input_fp);
    TRACE("%ld bytes read\n", num_read);
	fclose(input_fp);
	input_fp = NULL;
	TRACE(" done.\n");
	return true;
}

// *********************************************************************
void process_arg(char *arg)
{
    if (*arg == 'i') 
    {
        arg = arg+2;
        strcpy(input_fn, arg);
    }
    else if (*arg == '?') 
    {
        printf("usage forth [args]\n");
        printf("  -i:imagefile - Forth VM image\n");
        printf("     default imagefile is 'forth.bin'\n");
        printf("  -? - Prints this message\n");
        exit(0);
    }
    else
    {
        printf("unknown arg '-%s'\n", arg);
    }
}

// *********************************************************************
int main(int argc, char **argv)
{
    strcpy(input_fn, "forth.bin");

    for (int i = 1; i < argc; i++)
    {
        char *cp = argv[i];
        if (*cp == '-')
        {
            process_arg(++cp);
        }
    }

	if (load_vm())
    {
		cpu_loop(0);
	}

    return 0;
}
