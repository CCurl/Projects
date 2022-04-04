reset

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ ;
: here ha @ ;
: last la @ ;
: . space (.) ;

: min over over < if drop then ;
: max over over > if drop then ;

// (x l h -- f)
: betw +tmps s3 s2 s1 
    r1 r2 < 0= 
    r1 r3 > 0= 
    = -tmps ;

: T1 s9 r9 bl '~' betw if r9 emit else r9 ." (%d)" then ;
: .code user dup here + 1- for i c@ T1 next ;
