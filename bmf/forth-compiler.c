#include "Shared.h"
//#include <string.h>
//#include <stdarg.h>
//#include <stdlib.h>
//#include <ctype.h>
#include "forth-vm.h"
#include "string.h"

char input_fn[256];
char output_fn[256];
FILE *input_fp = NULL;
FILE *output_fp = NULL;
int mem_size_KB = 64;

CELL HERE, LAST;
CELL STATE, BASE = 10;

CELL ADDR_CELL     = 0x08;
CELL ADDR_HERE     = 0x10;
CELL ADDR_LAST     = 0x14;
CELL ADDR_BASE     = 0x18;
CELL ADDR_STATE    = 0x20;
CELL ADDR_MEM_SZ   = 0x24;

#define  Store(loc, val) CELL_AT(loc) = val
#define CStore(loc, val) BYTE_AT(loc) = val
#define WStore(loc, val) WORD_AT(loc) = val

#define  Comma(val) CELL_AT(HERE) = val; HERE += CELL_SZ
#define CComma(val) BYTE_AT(HERE) = val; HERE += 1
#define WComma(val) WORD_AT(HERE) = val; HERE += 2

#define  Fetch(loc) CELL_AT(loc)
#define CFetch(loc) BYTE_AT(loc)
#define WFetch(loc) WORD_AT(loc)

extern int _QUIT_HIT;

OPCODE_T theOpcodes[] = {
          { "LITERAL",          LITERAL,          "LITERAL",        0 }
        , { "CLIT",             CLITERAL,         "CLIT",           IS_INLINE }
        , { "WLIT",             WLITERAL,         "WLIT",           IS_INLINE }
        , { "FETCH",            FETCH,            "FETCH",          IS_INLINE }
        , { "STORE",            STORE,            "STORE",          IS_INLINE }
        , { "WSTORE",           WSTORE,           "WSTORE",         IS_INLINE }
        , { "SWAP",             SWAP,             "SWAP",           IS_INLINE }
        , { "DROP",             DROP,             "DROP",           IS_INLINE }
        , { "DUP",              DUP,              "DUP",            IS_INLINE }
        , { "SLITERAL",         SLITERAL,         "SLITERAL",       IS_INLINE }
        , { "JMP",              JMP,              "JMP",            0 }
        , { "JMPZ",             JMPZ,             "JMPZ",           0 }
        , { "JMPNZ",            JMPNZ,            "JMPNZ",          0 }
        , { "CALL",             CALL,             "CALL",           0 }
        , { "RET",              RET,              "RET",            0 }
        , { "OR",               OR,               "OR",             IS_INLINE }
        , { "CFETCH",           CFETCH,           "CFETCH",         IS_INLINE }
        , { "CSTORE",           CSTORE,           "CSTORE",         IS_INLINE }
        , { "ADD",              ADD,              "ADD",            IS_INLINE }
        , { "SUB",              SUB,              "SUB",            IS_INLINE }
        , { "MUL",              MUL,              "MUL",            IS_INLINE }
        , { "DIV",              DIV,              "DIV",            IS_INLINE }
        , { "LT",               LT,               "LT",             IS_INLINE }
        , { "EQ",               EQ,               "EQ",             IS_INLINE }
        , { "GT",               GT,               "GT",             IS_INLINE }
        , { "DICTP",            DICTP,            "DICTP",          IS_INLINE }
        , { "EMIT",             EMIT,             "EMIT",           IS_INLINE }
        , { "OVER",             OVER,             "OVER",           IS_INLINE }
        , { "FOR",              FOR,              "FOR",            IS_INLINE }
        , { "NEXT",             NEXT,             "NEXT",           IS_INLINE }
        , { "INDEX",            INDEX,            "INDEX",          IS_INLINE }
        , { "COMPARE",          COMPARE,          "COMPARE",        IS_INLINE }
        , { "FOPEN",            FOPEN,            "FOPEN",          IS_INLINE }
        , { "FREAD",            FREAD,            "FREAD",          IS_INLINE }
        , { "FREADLINE",        FREADLINE,        "FREADLINE",      IS_INLINE }
        , { "FWRITE",           FWRITE,           "FWRITE",         IS_INLINE }
        , { "FCLOSE",           FCLOSE,           "FCLOSE",         IS_INLINE }
        , { "DTOR",             DTOR,             "DTOR",           IS_INLINE }
        , { "RTOD",             RTOD,             "RTOD",           IS_INLINE }
        , { "COM",              COM,              "COM",            IS_INLINE }
        , { "AND",              AND,              "AND",            IS_INLINE }
        , { "PICK",             PICK,             "PICK",           IS_INLINE }
        , { "DEPTH",            DEPTH,            "DEPTH",          IS_INLINE }
        , { "GETCH",            GETCH,            "GETCH",          IS_INLINE }
        , { "COMPAREI",         COMPAREI,         "COMPAREI",       IS_INLINE }
        , { "SLASHMOD",         SLASHMOD,         "SLASHMOD",       IS_INLINE }
        , { "NOT",              NOT,              "NOT",            IS_INLINE }
        , { "RFETCH",           RFETCH,           "RFETCH",         IS_INLINE }
        , { "INC",              INC,              "INC",            IS_INLINE }
        , { "RDEPTH",           RDEPTH,           "RDEPTH",         IS_INLINE }
        , { "DEC",              DEC,              "DEC",            IS_INLINE }
        , { "GETTICK",          GETTICK,          "GETTICK",        IS_INLINE }
        , { "SHIFTLEFT",        SHIFTLEFT,        "LSHIFT",         IS_INLINE }
        , { "SHIFTRIGHT",       SHIFTRIGHT,       "RSHIFT",         IS_INLINE }
        , { "PLUSSTORE",        PLUSSTORE,        "PLUSSTORE",      IS_INLINE }
        , { "OPENBLOCK",        OPENBLOCK,        "OPENBLOCK",      IS_INLINE }
        , { "DBGDOT",           DBGDOT,           "DBGDOT",         IS_INLINE }
        , { "DBGDOTS",          DBGDOTS,          "DBGDOTS",        IS_INLINE }
        , { "NOP",              NOP,              "NOP",            IS_INLINE }
        , { "BREAK",            BREAK,            "BREAK",          IS_INLINE }
        , { "RESET",            RESET,            "RESET",          IS_INLINE }
        , { "BYE",              BYE,              "BYE",            IS_INLINE }
        , { NULL,               0,                 "",              IS_INLINE }
 };


// ------------------------------------------------------------------------------------------
void SyncMem(bool isSet)
{
    if (isSet) {
        Store(ADDR_LAST,  LAST);
        Store(ADDR_HERE,  HERE);
        Store(ADDR_BASE,  BASE);
        Store(ADDR_STATE, STATE);
        Store(ADDR_MEM_SZ, MEM_SZ);
    } else {
        LAST  = Fetch(ADDR_LAST);
        HERE  = Fetch(ADDR_HERE);
        BASE  = Fetch(ADDR_BASE);
        STATE = Fetch(ADDR_STATE);
    }
}

void DefineWord(LPCTSTR word, BYTE flags)
{
    CELL curLAST = LAST;
    LAST -= ((2 * 2) + 3 + string_len(word));

    DICT_T* dp = (DICT_T*)(&the_memory[LAST]);
    dp->next = (USHORT)curLAST;
    dp->XT = (USHORT)HERE;
    dp->flags = flags;
    dp->len = string_len(word);
    string_copy(dp->name, word);
    SyncMem(true);
}

DICT_T *FindWord(LPCTSTR word)
{
    DICT_T *dp = (DICT_T *)(&the_memory[LAST]);
    while (dp->next > 0)
    {
        if (string_equals(word, dp->name)) {
            return dp;
        }
        dp = (DICT_T *)(&the_memory[dp->next]);
    }

    return 0;
}

bool MakeNumber(LPCTSTR word, CELL *the_num)
{
    CELL base = BASE, my_num = 0;
    bool is_neg = false;
    char *w = word;

    if (w[0] == '%') { base = 2; ++w; }
    if (w[0] == '#') { base = 10; ++w; }
    if (w[0] == '$') { base = 16; ++w; }

    if ((w[0]=='\'') && (w[2]==w[0]) && (w[3]==0)) {
        the_num[0] = w[1];
        return true;
    }

    // One leading minus sign is OK in decimal
    if ((base == 10) && (*w == '-')) { is_neg = true; w++; }

    if (*w == 0) { return false; }
    while (*w) {
        int t = -1;
        char a = *(w++);
        if ((base == 2) && betw(a, '0', '1')) { t = a - '0'; }
        if ((9 < base) && betw(a, '0', '9')) { t = a - '0'; }
        if ((base == 16) && betw(a, 'a', 'f')) { t = a - 'a' + 10; }
        if ((base == 16) && betw(a, 'A', 'F')) { t = a - 'A' + 10; }
        if (t < 0) { return false; }
        my_num = (my_num * base) + t;
    }

    if (is_neg) { my_num = -my_num; }
    the_num[0] = my_num;
    return true;
}

// Returns a pointer to the first char after the first word in the line
// NB: this is NOT a counted string
char *GetWord(char *line, char *word)
{
    char *cp = word;

    // Skip beginning WS
    while ((*line) && (*line <= ' ')) { line++; }

    // Copy chars up to ' ' or EOL or 1st non-printable char
    while ((*line) && (*line > ' ')) { *(cp++) = *(line++); }
    *cp = NULL;

    return line;
}

char *ParseWord(char *word, char *line)
{
    if (string_equals(word, ".QUIT")) {
        _QUIT_HIT = 1;
        return line;
    }

    if ((STATE < 0) || (STATE > 2)) {
        printf("STATE (%ld) is messed up!\n", STATE);
        STATE = 0;
    }

    if (string_equals(word, ":")) {
        line = GetWord(line, word);
        DefineWord(word, 0);
        //CComma(DICTP);
        //WComma((USHORT)LAST);
        STATE = 1;
        return line;
    }

    if (string_equals(word, ".VARIABLE.")) {
        line = GetWord(line, word);
        DefineWord(word, 0);
        //CComma(DICTP);
        //Comma(LAST);
        CComma(WLITERAL);
        WComma((USHORT)(HERE+3));
        CComma(RET);

        if (string_equals(word, "(HERE)")) ADDR_HERE = HERE;
        if (string_equals(word, "(LAST)")) ADDR_LAST = HERE;
        if (string_equals(word, "BASE")) ADDR_BASE = HERE;
        if (string_equals(word, "STATE")) ADDR_STATE = HERE;
        if (string_equals(word, "(MEM_SZ)")) ADDR_MEM_SZ = HERE;

        Comma(0);
        SyncMem(true);
        return line;
    }

    if (string_equals(word, ".INLINE.")) {
        DICT_T *dp = (DICT_T *)(&the_memory[LAST]);
        dp->flags |= IS_INLINE;
        return line;
    }

    if (string_equals(word, ".IMMEDIATE.")) {
        DICT_T *dp = (DICT_T *)(&the_memory[LAST]);
        dp->flags |= IS_IMMEDIATE;
        return line;
    }

    if (string_equals(word, ".EndOfVariables.")) {
        CComma(NOP);
        return line;
    }

    if (string_equals(word, ";")) {
        CComma(RET);
        STATE = 0;
        return line;
    }

    //if (string_equals(word, ".IF")) {
    //    CComma(JMPZ);
    //    push(HERE);
    //    WComma(0);
    //    return line;
    //}

    //if (string_equals(word, ".THEN")) {
    //    CELL tmp = pop();
    //    WStore(tmp, HERE);
    //    return line;
    //}

    if (string_equals(word, "S\"")) {
        CComma(SLITERAL);
        int count = 0;
        bool done = false;
        line += 1;
        CELL cur_here = HERE++;
        BYTE ch;
        while ((!done) && (!string_isEmpty(line)))
        {
            ch = *(line++);
            if (ch == '\"') {
                done = true;
            } else {
                CComma(ch);
                count++;
            }
        }
        CComma(0);
        CStore(cur_here, count);
        return line;
    }

    // "Assembler" words
    for (int i = 0; ; i++)
    {
        OPCODE_T* op = &(theOpcodes[i]);
        if (op->asm_instr == NULL) {
            break;
        }

        if (string_equals(word, op->asm_instr)) {
            if (STATE == 1) { // Compiling
                CComma(op->opcode);
            }
            else {
                CELL xt = HERE + 0x0100;
                CStore(xt, op->opcode);
                CStore(xt + 1, RET);
                SyncMem(true);
                cpu_loop((ADDR)xt);
                SyncMem(false);
            }
            return line;
        }
    }

    DICT_T *dp = FindWord(word);
    if (dp != NULL) {
        if (STATE == 1) {
            if (dp->flags & IS_IMMEDIATE) {
                SyncMem(true);
                cpu_loop(dp->XT);
                SyncMem(false);
            } else if (dp->flags & IS_INLINE) {
                // Skip the DICTP instruction
                CELL addr = dp->XT; //  +1 + CELL_SZ;

                // Copy bytes until the first RET
                while (true) {
                    BYTE b = CFetch(addr++);
                    if (b == RET) { break; }
                    CComma(b);
                }
            } else {
                CComma(CALL);
                WComma(dp->XT);
            }
        } else {
            SyncMem(true);
            cpu_loop(dp->XT);
            SyncMem(false);
        }
        return line;
    }

    CELL num = 0;
    if (MakeNumber(word, &num)) {
        if (STATE == 0) { push(num); return line; }
        if ((0 <= num) && (num <= 0xFF)) {
            CComma(CLITERAL);
            CComma((BYTE)num);
        } else if ((0x100 <= num) && (num <= 0xFFFF)) {
            CComma(WLITERAL);
            Comma(num);
        } else {
            CComma(LITERAL);
            Comma(num);
        }
        return line;
    }

    _QUIT_HIT = 1;
    printf("ERROR: '%s'??\n", word);
    return line;
}

void ParseLine(char *line)
{
    char *source = line;
    char word[128];

    while (string_len(line) > 0)
    {
        line = GetWord(line, word);
        if (string_equals(word, "\\")) { return; }
        if (string_equals(word, "//")) { return; }

        if (string_len(word) == 0) {
            return;
        }

        line = ParseWord(word, line);
    }
}


void CompilerInit()
{
    init_vm();
    the_memory[0] = RET;
    isEmbedded = true;

    HERE = 0x0040;
    LAST = MEM_SZ - CELL_SZ;
    STATE = 0;

    Store(LAST, 0);
    Store(ADDR_CELL, CELL_SZ);
    Store(ADDR_BASE, BASE);
}

void Compile(FILE *fp_in)
{
    int line_no = 0;
    char buf[128];
    char line[128];

    while (fgets(buf, sizeof(buf), fp_in) == buf)
    {
        string_rtrim(buf);
        ++line_no;
        string_copy(line, buf);
        ParseLine(buf);
        if (_QUIT_HIT == 1) {
            printf("QUIT hit on line %d: %s\n", line_no, line);
            break;
        }
    }
    fclose(fp_in);

    DICT_T *dp = FindWord("main");
    if (dp == NULL)	{
        dp = FindWord("MAIN");
        if (dp == NULL) {
            dp = (DICT_T *)&the_memory[LAST];
        }
    }

    CStore(0, JMP);
    Store(1, dp->XT);

    SyncMem(true);
}

void do_compile()
{
    printf("compiling from %s...\n", input_fn);
    CompilerInit();
    // return;

    input_fp = fopen(input_fn, "rt");
    if (!input_fp) {
        printf("can't open input file!");
        // exit(1);
    }

    Compile(input_fp);
    Store(ADDR_BASE, 10);
    fclose(input_fp);
    input_fp = NULL;
}

// *********************************************************************
void write_output_file()
{
    printf("writing output file %s ... ", output_fn);

    output_fp = fopen(output_fn, "wb");
    if (!output_fp) {
        printf("ERROR: Can't open output file!");
        return;
    }

    int num = fwrite(the_memory, 1, MEM_SZ, output_fp);
    fclose(output_fp);
    output_fp = NULL;
    printf("%d bytes written.\n", num);
}

// *********************************************************************
void process_arg(char *arg)
{
    if (*arg == 'i') {
        arg = arg+2;
        string_copy(input_fn, arg);
    } else if (*arg == 'o') {
        arg = arg+2;
        string_copy(output_fn, arg);
    } else if (*arg == 'm') {
        arg = arg+2;
        // mem_size_KB = atoi(arg);
    } else if (*arg == '?') {
        printf("usage: forth-compiler [args]\n");
        printf("  -i:inputFile (full or relative path)\n");
        printf("      default inputFile is forth.src\n");
        printf("  -o:outputFile (full or relative path)\n");
        printf("      default outputFile is forth.bin\n");
        printf("  -m:<KB> - Memory size in KB\n");
        printf("      default value is 64\n");
        printf("  -? (prints this message)\n");
        // exit(0);
    } else {
        printf("unknown arg '-%s'\n", arg);
    }
}

int main (int argc, char **argv)
{
    string_copy(input_fn, "forth.src");
    string_copy(output_fn, "forth.bin");

    for (int i = 1; i < argc; i++) {
        char *cp = argv[i];
        if (*cp == '-') {
            process_arg(++cp);
        }
    }

    // MEM_SZ = mem_size_KB * 1024;
    do_compile();
    write_output_file();

    return 0;
}
