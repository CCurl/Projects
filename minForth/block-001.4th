reset

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ ;
: here ha @ ;
: last la @ ;
: . space (.) ;

: betw +tmps s3 s2 s1 ( n a b--f )
    r1 r2 < 0= 
    r1 r3 > 0= 
    = -tmps ;

: T1 s9 r9 #32 #127 betw if r9 emit else r9 ." (%d)" then ;
: .code user dup here + 1- for i c@ T1 next ;

: ->XY #27 ." %c[%d;%dH" ;
: CLS #27 ." %c[2J" 1 dup ->XY ;
