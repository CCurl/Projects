\ Conway's game of life

' b++ loaded?

load-f random.c3
load-f screen.c3

val rows  >val rows!
val cols  >val cols!
: pop-sz rows 2 + cols 2 + * ;

val s-rows   >val s-rows!
val s-cols   >val s-cols!

100 rows! 300 cols!
variable pop pop-sz allot
variable bak pop-sz allot
variable ln cols allot

: T6 ( a--b ) DUP C@ 1+ OVER C! 1+ ;
: b++ ( -- )
    r4 1- cols - T6 T6 T6 DROP
    r4 1-        T6 1+ T6 DROP
    r4 1- cols + T6 T6 T6 DROP ;

: alive? ( a b -- c )
    DUP 2 = IF DROP EXIT THEN
    3 = IF DROP 1 EXIT THEN
    DROP 0 ;

: rand-pop pop s0 pop-sz 0 DO $FF rand-max #200 > r0+ c! LOOP ;
: clr-bak bak s1 pop-sz 0 DO 0 r1+ c! LOOP ;
: bak->pop pop-sz 0 DO
        I bak + s1 I pop + s2
        r2 C@ r1 C@ alive? r2 c! 
        0 r1 c!
    LOOP ;

: ->p ( c r -- v ) cols * + pop + ;
: ->b ( c r -- v ) cols * + bak + ;

: .pop 1 DUP ->XY
    s-rows 1+ 1 DO ln s6 
        s-cols 1+ 1 DO BL I J ->p C@ IF DROP '*' THEN r6+ c! LOOP
        0 r6 c! ln QTYPE cr
    LOOP ;

: one-gen 1 1 ->b s4
    cols rows ->p 1 1 ->p
    DO I C@ IF b++ THEN i4 LOOP
    bak->pop .pop r7+ . ;

: pc     50 s-rows! 150 s-cols! ;
: laptop 25 s-rows!  70 s-cols! ;
: huge   95 s-rows! 290 s-cols! ;

: gen? one-gen ?key ;
: gens 0 DO gen? IF key DROP UNLOOP EXIT THEN LOOP ;
: life 1 s7 clr-bak rand-pop 
   CUR-OFF
   CLS 2000 gens
   CUR-ON ;

pc life
