\ dis.c3 - generate a dis-assembly

here s7

variable (jt) 256 cells allot
: >jt ( a n-- ) cells (jt) + ! ;
: jt@ ( n--a )  cells (jt) + @ ;
: jtx ( n-- )   jt@ ?dup if exec else ." (data)" then ;

: lp '(' emit ; inline
: rp ')' emit ; inline
: .p dup lp <# # #S #> #P rp space ;
: find-xt ( xt--a ) +regs s2 last s1 mem-end s3
	begin r1 r3 < while
		r1 @ r2 = if r1 -regs exit then
		r1 word-sz + s1
	repeat 0 -regs ;
: .word-xt ( xt-- ) find-xt ?dup if cell + 1+ ."  ; " count type then ;
: .xt dup . .word-xt ;
: .wn .word-xt ;
: +CELL r9 CELL + s9 ;

:noname ." LIT1 " r9+ c@ . ; 1 >jt
:noname ." LIT4 " r9 @ . r9 cell + s9 ; 2 >jt
:noname ." RET" ; 3 >jt
:noname ." CALL "  r9 @ .xt +CELL ;  4 >jt
:noname ." JMP "   r9 @ . +CELL ;    5 >jt
:noname ." JMPZ "  r9 @ . +CELL ;    6 >jt
:noname ." JMPNZ " r9 @ . +CELL ;    7 >jt
:noname ." STORE" ;   8 >jt
:noname ." CSTORE" ;  9 >jt
:noname ." FETCH" ;  10 >jt
:noname ." CFETCH" ; 11 >jt
:noname ." DUP" ;    12 >jt
:noname ." SWAP" ;   13 >jt
:noname ." OVER" ;   14 >jt
:noname ." DROP" ;   15 >jt
:noname ." ADD" ;    16 >jt
:noname ." MULT" ;   17 >jt
:noname ." SLMOD" ;  18 >jt
:noname ." SUB" ;    19 >jt
:noname ." INC" ;    20 >jt
:noname ." DEC" ;    21 >jt
:noname ." LT" ;     22 >jt
:noname ." EQ" ;     23 >jt
:noname ." GT" ;     24 >jt
:noname ." LNOT" ;   25 >jt
:noname ." DTOR" ;   26 >jt
:noname ." RFROM" ;  27 >jt
:noname ." RTOD" ;   28 >jt
:noname ." DO";      29 >jt
:noname ." LOOP" ;   30 >jt
:noname ." LOOP2" ;  31 >jt
:noname ." ADDRI" ;  32 >jt
:noname ." COM" ;    33 >jt
:noname ." AND" ;    34 >jt
:noname ." OR" ;     35 >jt
:noname ." XOR" ;    36 >jt
:noname ." EMIT" ;   37 >jt
:noname ." TIMER" ;  38 >jt
:noname ." KEY" ;    39 >jt
:noname ." QKEY" ;   40 >jt
:noname ." TYPE" ;   41 >jt
:noname ." TYPEZ" ;  42 >jt
:noname ." DEFWORD" ;   43 >jt
:noname ." ENDWORD" ;   44 >jt
:noname ." REG " r9+ c@ (.)    ; 50 >jt
:noname ." REGDEC " r9+ c@ (.) ; 51 >jt
:noname ." REGINC " r9+ c@ (.) ; 52 >jt
:noname ." SET " r9+ c@ (.)    ; 53 >jt
:noname ." REGSADD"  ; 54 >jt
:noname ." REGSDEL"  ; 55 >jt

: dis-1 cr r9+ dup . dup c@ .p jtx .wn ;

: dis ( t f-- ) s9 s1 begin r9 r1 < while dis-1 repeat ;

here s8
r8 r7 dis

: rl forget s" dis.c3" (load) ;
: ed s" nvim dis.c3" system ;
