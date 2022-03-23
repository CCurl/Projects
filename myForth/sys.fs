: vh v @ ;
: 2dup over over ;
: 2drop drop drop ;
: .c dup c@ dup $20 < if drop '.' then emit 1+ ;
// : dump 1 for .c next drop ;
: tt if 'Y' else 'N' then emit ;
1 tt 0 tt
// u $32 dump