: vh v @ ; : here h @ ;
: .c dup $20 < if drop '.' then emit ;
: .code u u here + 1- for i c@ .c next drop ;

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: tt if 'Y' else 'N' then emit ;
: mil 1000 dup * * ;
: bm timer swap 1 for next timer swap - . ;
: ttt cr 1 2 3 + + . 19 42 for i . next ;
: rr cr 20 begin dup . 1- while ;
cr 1 tt 0 tt
ttt rr
: lt1 +tmps 123 s0 445 s1     r0 . r1 . -tmps ;
: lt2 +tmps 666 s0 777 s1 lt1 r0 . r1 . -tmps ;
cr lt2
cr 'H' emit '=' emit h @ .
cr .code
