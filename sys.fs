: . 32 emit (.) ;
: 1+ 1 + ;
: 2dup over over ;
: 2drop drop drop ;
: .c dup dc@ dup . dup 32 < if drop 46 then emit 1+ ;
: dd 2dup > if 2drop leave then swap .c swap dd ;
