\ A example stack

16 cells var tstk      \ the stack start
vhere cell - const t9  \ t9 is the stack end
val tsp@   (val) t1    \ the stack pointer
: tsp! ( n-- ) t1 ! ;  \ set the stack pointer
tstk tsp!              \ Initialize
\ for a normal stack, use these definitions
\ : tsp++ ( -- ) tsp@ cell+ t9    min tsp! ;
\ : tsp-- ( -- ) tsp@ cell - tstk max tsp! ;
\ for a circular stack, use these definitions
: tsp++ ( -- )  tsp@ cell+   dup t9   > if drop tstk then tsp! ;
: tsp-- ( -- )  tsp@ cell -  dup tstk < if drop t9   then tsp! ;
: t!    ( n-- ) tsp@ ! ;
: t@    ( --n ) tsp@ @ ;
: >t    ( n-- ) tsp++ t! ;
: t>    ( --n ) tsp@ @  tsp-- ;
: t6    ( -- )  dup tsp@ = if ." sp:" then dup @ . cell+ ;
: .tstk ( -- )  '(' emit space tstk 16 for t6 next drop ')' emit ;

