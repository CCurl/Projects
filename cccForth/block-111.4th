// Conway's game of life

1 load

variable (r) 100 (r) !
variable (c) 300 (c) !
: rows   (r) @ ; 
: rows!  (r) ! ; 
: cols   (c) @ ;
: cols!  (c) ! ;
: pop-sz rows 2 + cols 2 + * ;

variable pop pop-sz allot
variable bak pop-sz allot
variable ln cols allot

: T5 ( a--b ) DUP C@ 1+ OVER C! 1+ ;
: b++ ( -- )
    r4 1- cols - T5 T5 T5 drop
    r4 1-        T5 1+ T5 drop
    r4 1- cols + T5 T5 T5 drop ;

: alive? ( a b -- c )
    dup 2 = if drop exit then
    3 = if drop 1 exit then
    drop 0 ;

: rand-pop pop s0 pop-sz 0 do rand $FF and #200 > r0 c! i0 loop ;
: clr-bak bak s1 pop-sz 0 do 0 r1 c! i1 loop ;
: bak->pop pop-sz 0 do
        i bak + s1 i pop + s2
        r2 c@ r1 c@ alive? r2 c! 
        0 r1 c!
    loop ;

: ->p ( c r -- v ) cols * + pop + ;
: ->b ( c r -- v ) cols * + bak + ;

: .pop 1 dup ->XY
    rows 1+ 1 do ln s6 
        cols 1+ 1 do i j ->p c@ if '*' else bl then r6 c! i6 loop
        0 r6 c! ln qtype cr
    loop ;

: one-gen 1 1 ->b s4
    cols rows ->p 1 1 ->p
    do i c@ if b++ then i4 loop
    bak->pop .pop r7 . i7 ;

// 65 (r) ! 200 (c) !
60 rows! 150 cols!
: gen? one-gen key? ;
: gens 0 do gen? if key drop unloop exit then loop ;
: life 1 s7 clr-bak rand-pop 
   CURSOR-OFF
   CLS 2000 gens
   CURSOR-ON ;
: reload 111 load ;
life