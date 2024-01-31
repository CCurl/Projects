\ see.c3 - Be able to see the definition of a word

marker

variable (jt) 256 cells allot
: >jt ( a n-- ) cells (jt) + ! ;
: jt@ ( n--a )  cells (jt) + @ ;
: jtx ( n-- )   jt@ ?dup if execute else ." (data)" then ;
: jt-init 256 0 do 0 i >jt loop ;
jt-init

: lp '(' emit ; inline
: rp ')' emit ; inline
: .p dup lp (.) rp space ;
: find-xt ( xt--a ) +regs s2 last s1 code-end s3
	begin r1 r3 < while
		r1 @ r2 = if r1 -exit then
		r1 word-sz + s1
	repeat 0 -regs ;
: .word-xt ( xt-- ) find-xt ?dup if ."  ; " cell+ 2+ qtype then ;
: .H ." $%x " ;
: .W $FFFFF AND .H ;
: .xt dup .W .word-xt ;
: +CELL r9 CELL+ s9 ;
: .NB r9+ c@ (.) ;
: .NC r9 @ .W +CELL ;

:noname ." LIT1 "  .NB ;             1 >jt
:noname ." LIT "   .NC ;             2 >jt
:noname ." RET" ;                    3 >jt
:noname ." CALL "  r9 @ .xt +CELL ;  4 >jt
:noname ." JMP "   .NC ;             5 >jt
:noname ." JMPZ "  .NC ;             6 >jt
:noname ." JMPNZ " .NC ;             7 >jt
:noname ." STORE" ;      8 >jt
:noname ." CSTORE" ;     9 >jt
:noname ." FETCH" ;     10 >jt
:noname ." CFETCH" ;    11 >jt
:noname ." DUP" ;       12 >jt
:noname ." SWAP" ;      13 >jt
:noname ." OVER" ;      14 >jt
:noname ." DROP" ;      15 >jt
:noname ." ADD" ;       16 >jt
:noname ." MULT" ;      17 >jt
:noname ." SLMOD" ;     18 >jt
:noname ." SUB" ;       19 >jt
:noname ." INC" ;       20 >jt
:noname ." DEC" ;       21 >jt
:noname ." LT" ;        22 >jt
:noname ." EQ" ;        23 >jt
:noname ." GT" ;        24 >jt
:noname ." LNOT" ;      25 >jt
:noname ." DTOR" ;      26 >jt
:noname ." RFROM" ;     27 >jt
:noname ." RTOD" ;      28 >jt
:noname ." DO";         29 >jt
:noname ." LOOP" ;      30 >jt
:noname ." LOOP2" ;     31 >jt
:noname ." ADDRI" ;     32 >jt
:noname ." COM" ;       33 >jt
:noname ." AND" ;       34 >jt
:noname ." OR" ;        35 >jt
:noname ." XOR" ;       36 >jt
:noname ." TYPE" ;      37 >jt
:noname ." TYPEZ" ;     38 >jt
:noname ." INCREG " .NB ; 39 >jt
:noname ." DECREG " .NB ; 40 >jt
:noname ." REG "    .NB ; 41 >jt
:noname ." REGDEC " .NB ; 42 >jt
:noname ." REGINC " .NB ; 43 >jt
:noname ." SET "    .NB ; 44 >jt
:noname ." +REGS"  ;      45 >jt
:noname ." -REGS"  ;      46 >jt

:noname r9+ c@ .p s0
	r0  0 = IF ."  INLINE" EXIT THEN
	r0  1 = IF ."  IMMEDIATE" EXIT THEN
	r0  2 = IF ."  (.)" EXIT THEN
  \ r0  3 = IF ." 3" EXIT THEN
	r0  4 = IF ."  ITOA" EXIT THEN
	r0  5 = IF ."  ATOI" EXIT THEN
	r0  6 = IF ."  COLON" EXIT THEN
	r0  7 = IF ."  SEMICOLON" EXIT THEN
	r0  8 = IF ."  CREATE" EXIT THEN
	r0  9 = IF ."  FIND-WORD" EXIT THEN
	r0 10 = IF ."  NEXT-WORD" EXIT THEN
	r0 11 = IF ."  TIMER" EXIT THEN
	r0 12 = IF ."  CCOMMA" EXIT THEN
	r0 13 = IF ."  COMMA" EXIT THEN
	r0 14 = IF ."  KEY" EXIT THEN
	r0 15 = IF ."  ?KEY" EXIT THEN
	r0 16 = IF ."  EMIT" EXIT THEN
	r0 17 = IF ."  QTYPE" EXIT THEN
	; 47 >jt

:noname r9+ c@ .p s0
	r0  0 = IF ."  S-TRUNC" EXIT THEN
	r0  1 = IF ."  LCASE" EXIT THEN
	r0  2 = IF ."  UCASE" EXIT THEN
  \ r0  3 = IF ." 3" EXIT THEN
	r0  4 = IF ."  S-CPY" EXIT THEN
	r0  5 = IF ."  S-CAT" EXIT THEN
	r0  6 = IF ."  S-CATC" EXIT THEN
	r0  7 = IF ."  S-LEN" EXIT THEN
	r0  8 = IF ."  S-EQ" EXIT THEN
	r0  9 = IF ."  S-EQI" EXIT THEN
	r0 10 = IF ."  S-EQN" EXIT THEN
	r0 11 = IF ."  S-LTRIM" EXIT THEN
	r0 12 = IF ."  S-RTRIM" EXIT THEN
	r0 13 = IF ."  S-FINDC" EXIT THEN
    ; 48 >jt

:noname r9+ c@ .p s0
	r0  0 = IF ."  F+" EXIT THEN
	r0  1 = IF ."  F-" EXIT THEN
	r0  2 = IF ."  F*" EXIT THEN
  \ r0  3 = IF ." 3" EXIT THEN
	r0  4 = IF ."  F/" EXIT THEN
	r0  5 = IF ."  F=" EXIT THEN
	r0  6 = IF ."  F<" EXIT THEN
	r0  7 = IF ."  F>" EXIT THEN
	r0  8 = IF ."  F>I" EXIT THEN
	r0  9 = IF ."  I>F" EXIT THEN
	r0 10 = IF ."  F." EXIT THEN
	r0 11 = IF ."  F-SQRT" EXIT THEN
	r0 12 = IF ."  F-TANH" EXIT THEN
	; 49 >jt

:noname ." SYSTEM" ;     100 >jt
:noname ." FOPEN" ;      101 >jt
:noname ." FCLOSE" ;     102 >jt
:noname ." FREAD" ;      103 >jt
:noname ." FWRITE" ;     104 >jt
:noname ." FLOAD" ;      105 >jt
:noname ." BLOAD" ;      106 >jt
:noname ." EDIT-BLK" ;   107 >jt
:noname ." EDIT-FILE" ;  108 >jt

: dis-1 ( -r9- -- ) cr r9+ dup .W dup c@ .p jtx .word-xt ;
: dis-range ( t f-- ) s9 s1 begin r9 r1 < while dis-1 repeat ;

: dis ( -- ) ' 0= if ." -nf-" exit then
    +regs drop s7 here s8
    r7 find-xt dup last >
	if word-sz - @ s8 then
	r8 r7 -regs dis-range ;

: rl forget " dis.c3" (load) ;
: ed " nvim dis.c3" system ;
