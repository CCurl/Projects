: vh v @ ; : here h @ ;
: T1 dup $20 < if drop '.' then ;
: .code u u here + 1- for i c@ T1 emit next drop ;

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: mil 1000 dup * * ;
: bm timer swap 1 for next timer swap - . ;

: T1 if 'Y' else 'N' then emit ;
: T2 1 2 3 + + . 19 42 for i . next ;
: T3 20 begin dup . 1- while ;
: T4 +tmps 123 s0 445 s1     r0 . r1 . -tmps ;
: T5 +tmps 666 s0 777 s1 T4 r0 . r1 . -tmps ;

cr 1 T1 0 T1 cr T2 cr T3 cr T4 cr T5

cr 'H' emit '=' emit h @ .
words
cr .code
