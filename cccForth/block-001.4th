reset

: hex $10 base ! ;
: decimal #10 base ! ;
: binary %10 base ! ;

: vhere va @ vb + ;
: here ha @ cb + ;
: last la @ 1- 20 * db + ;

: num-words la @ ;
: used here cb - ;

: T1 ( a-- ) DUP 4 + SWAP W@ ." %n%i: %s" ;
: words-l db num-words 0 DO DUP T1 20 + LOOP DROP ;

( n a b -betw- f )
: betw +TMPS s3 s2 s1  r2 r1 <=  r1 r3 <=  and -TMPS ;
: min ( a b--x ) OVER OVER > .if SWAP .then DROP ;
: max ( a b--x ) OVER OVER < .if SWAP .then DROP ;

: .char ( c-- ) DUP BL $7E betw .if EMIT EXIT .then ." (%d)" ;
: fill ( c f t-- ) SWAP DO DUP I C! LOOP DROP ;
: fill-n ( c a n-- ) OVER + fill ;
: ON 1 ; : OFF 0 ;

// Screen stuff
: ->XY ( x y-- ) ." %e[%d;%dH" ;
: CLS ." %e[2J" 1 dup ->XY ;
: COLOR ( bg fg -- ) ." %e[%d;%dm" ;
: FG ( fg -- ) 40 swap COLOR ;
: CURSOR-ON  ( -- ) ." %e[?25h" ;
: CURSOR-OFF ( -- ) ." %e[?25l" ;
: CURSOR ( f-- ) if CURSOR-ON else CURSOR-OFF then ;
