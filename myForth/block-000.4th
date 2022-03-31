: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ ;
: here ha @ ;
: . space (.) ;

: T0 '(' emit (.) ')' emit 0 ;
: T1 dup bl < if T0 then dup '~' > if T0 then emit ;
: .code user dup here + 1- for i c@ T1 next drop ;
