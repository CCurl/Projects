\ Conway's game of life

load random.c3
load screen.c3

\ : LIFE 17 (lex) ! ;
\ LIFE

variable (r)
variable (c)

: rows   (r) @ ; : rows!  (r) ! ; 
: cols   (c) @ ; : cols!  (c) ! ;
: pop-sz rows 2 + cols 2 + * ;

100 rows! 300 cols!
variable pop pop-sz allot
variable bak pop-sz allot
variable ln cols allot

: x5 ( a--b ) DUP C@ 1+ OVER C! 1+ ; INLINE
: b++ ( -- )
    r4 1- cols - x5 x5 x5 DROP
    r4 1-        x5 1+ x5 DROP
    r4 1- cols + x5 x5 x5 DROP ;

: alive? ( a b -- c )
    DUP 2 = IF DROP EXIT THEN
    3 = IF DROP 1 EXIT THEN
    DROP 0 ;

: rand-pop pop s0 pop-sz 0 DO $FF rand-max #200 > r0 c! i0 LOOP ;
: clr-bak bak s1 pop-sz 0 DO 0 r1 c! i1 LOOP ;
: bak->pop pop-sz 0 DO
        I bak + s1 I pop + s2
        r2 C@ r1 C@ alive? r2 c! 
        0 r1 c!
    LOOP ;

: ->p ( c r -- v ) cols * + pop + ;
: ->b ( c r -- v ) cols * + bak + ;

: .pop 1 DUP ->XY
    rows 1+ 1 DO ln s6 
        cols 1+ 1 DO BL I J ->p C@ IF DROP '*' THEN r6 c! i6 LOOP
        0 r6 c! ln typez cr
    LOOP ;

: one-gen 1 1 ->b s4
    cols rows ->p 1 1 ->p
    DO I C@ IF b++ THEN i4 LOOP
    bak->pop .pop r7 . i7 ;

: pc 53 rows! 150 cols! ;
: laptop 25 rows! 50 cols! ;
: gen? one-gen ?key ;
: gens 0 DO gen? IF key DROP UNLOOP EXIT THEN LOOP ;
: life 1 s7 clr-bak rand-pop 
   CUR-OFF
   CLS 2000 gens
   CUR-ON ;

: reload " life" SLOAD ;

pc life
