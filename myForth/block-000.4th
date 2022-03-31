: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ ; : here ha @ ;
: T1 dup $20 < if drop '.' then ;
: .code user dup here + 1- for i c@ T1 emit next drop ;

667 load
