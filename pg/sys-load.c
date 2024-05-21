extern int parseLine(const char *ln);

void sys_load() {
    parseLine(": CODE-ADDR  CODE + ;");
    parseLine(": VARS-ADDR  VARS + ;");
    parseLine(": DICT-ADDR  DICT + ;");
    parseLine(": @C DUP + CODE-ADDR W@ ;");
    parseLine(": !C DUP + CODE-ADDR W! ;");
    parseLine(": (HERE)  0 ; : HERE  (HERE)  @C ;");
    parseLine(": (LAST)  1 ; : LAST  (LAST)  @C ;");
    parseLine(": (VHERE) 2 ; : VHERE (VHERE) @C ;");
    parseLine(": BASE    3 ; : STATE 4 ;");
    parseLine(": ALLOT VHERE + (VHERE) !C ;");
    parseLine(": @V  VARS-ADDR @ ;");
    parseLine(": !V  VARS-ADDR ! ;");
    parseLine(": C@V VARS-ADDR C@ ;");
    parseLine(": C!V VARS-ADDR C! ;");
    parseLine(": CELLS CELL * ;");
    parseLine(": BEGIN HERE ; IMMEDIATE");
    parseLine(": AGAIN JMP, , ; IMMEDIATE");
    parseLine(": WHILE JMPNZ, , ; IMMEDIATE");
    parseLine(": UNTIL JMPZ, , ; IMMEDIATE");
    parseLine(": IF JMPZ, HERE 0 , ; IMMEDIATE");
    parseLine(": THEN HERE SWAP !C ; IMMEDIATE");
    parseLine(": 1+ 1 + ;");
    parseLine(": SPACE 32 EMIT ; : . (.) SPACE ;");
    parseLine(": CR 13 EMIT 10 EMIT ; : TAB 9 EMIT ;");
    parseLine(": >XT     W@ ;");
    parseLine(": >SIZE   2 + C@ ;");
    parseLine(": >FLAGS  3 + C@ ;");
    parseLine(": >LEX    4 + C@ ;");
    parseLine(": >NAME   5 + ;");
    parseLine(": TYPE 0 DO DUP C@ EMIT 1+ LOOP DROP ;");
    parseLine(": COUNT DUP 1 + SWAP C@ ;");
    parseLine(": WORDS LAST DICT-ADDR BEGIN DUP >NAME COUNT TYPE TAB DUP >SIZE + DUP DICT-SZ DICT-ADDR < WHILE DROP ;");
    parseLine("CELL ALLOT");
}
