: vh v @ ; : here h @ ;
// : 2dup over over ;
// : 2drop drop drop ;
// : .c dup c@ dup $20 < if drop '.' then emit 1+ ;
// : dump 1 for .c next drop ;
: tt if 'Y' else 'N' then emit ;
cr 1 tt 0 tt
// u $32 dump

: mil 1000 dup * * ;
: bm timer swap 1 for next timer swap - . ;
