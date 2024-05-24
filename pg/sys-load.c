extern int parseLine(const char *ln);

void sys_load() {
    parseLine("\
: 0= 0 = ; \
: @C DUP + C-ADDR W@ ; \
: !C DUP + C-ADDR W! ; \
: (HERE)  0 ; : HERE  (HERE)  @C ; \
: (LAST)  1 ; : LAST  (LAST)  @C ; \
: (VHERE) 2 ; : VHERE (VHERE) @C ; \
: BASE    3 ; : STATE 4 ; \
: BEGIN HERE ; IMMEDIATE \
: AGAIN (JMP)   , , ; IMMEDIATE \
: WHILE (JMPNZ) , , ; IMMEDIATE \
: UNTIL (JMPZ)  , , ; IMMEDIATE \
: IF (JMPZ) , HERE 0 , ; IMMEDIATE \
: THEN HERE SWAP !C ; IMMEDIATE \
: ( BEGIN \
    >IN @ C@  \
    DUP  0= IF DROP EXIT THEN \
    >IN @ 1+ >IN ! \
    ')' = IF EXIT THEN \
  AGAIN ; IMMEDIATE \
: ALLOT VHERE + (VHERE) !C ; \
: @V  V-ADDR @ ; \
: !V  V-ADDR ! ; \
: C@V V-ADDR C@ ; \
: C!V V-ADDR C! ; \
: CELLS CELL * ; \
: SPACE 32 EMIT ; : . (.) SPACE ; \
: CR 13 EMIT 10 EMIT ; : TAB 9 EMIT ; \
: >XT     W@ ; \
: >SIZE   2 + C@ ; \
: >FLAGS  3 + C@ ; \
: >LEX    4 + C@ ; \
: >NAME   5 + ; \
: A+ A DUP 1+ >A ; \
: NIP SWAP DROP ; \
: TUCK SWAP OVER ; \
: ?DUP DUP IF DUP THEN ; \
: +! TUCK @ + SWAP ! ; \
: WORDS 0 +A LAST D-ADDR BEGIN \
    DUP >NAME COUNT TYPE TAB \
    A+ 8 > IF CR 0 >A THEN \
    DUP >SIZE + DUP DICT-SZ D-ADDR < \
  WHILE -A DROP ; \
");
} 
