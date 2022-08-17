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
: words-l  last num-words 0 DO DUP T1 20 - LOOP DROP ;
: last-few  last 10 0 DO DUP T1 20 - LOOP DROP ;

: min ( a b--x ) OVER OVER > .if SWAP .then DROP ;
: max ( a b--x ) OVER OVER < .if SWAP .then DROP ;

// Screen stuff
: ->XY ( x y-- ) ." %e[%d;%dH" ;
: CLS ." %e[2J" 1 DUP ->XY ;
: COLOR ( bg fg -- ) ." %e[%d;%dm" ;
: FG ( fg -- ) 40 SWAP COLOR ;
: CURSOR-ON  ( -- ) ." %e[?25h" ;
: CURSOR-OFF ( -- ) ." %e[?25l" ;
