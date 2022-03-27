: vh v @ ; : here h @ ;
// : 2dup over over ;
// : 2drop drop drop ;
// : .c dup c@ dup $20 < if drop '.' then emit 1+ ;
// : dump 1 for .c next drop ;
: tt if 'Y' else 'N' then emit ;
: mil 1000 dup * * ;
: bm timer swap 1 for next timer swap - . ;
: ttt cr 1 2 3 + + . 19 42 for i . next ;
: rr cr 20 begin dup . 1- while ;
cr 1 tt 0 tt
ttt rr
: lt1 +locs 123 s0 445 s1     r0 . r1 . -locs ;
: lt2 +locs 666 s0 777 s1 lt1 r0 . r1 . -locs ;
cr lt2
