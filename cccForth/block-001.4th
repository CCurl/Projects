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
: CLS ( -- ) ." %e[2J" 1 DUP ->XY ;
: CLR-EOL ( -- ) ." %e[K" ;
: CLR-EOS ( -- ) ." %e[J" ;
: COLOR ( bg fg -- ) ." %e[%d;%dm" ;
: FG ( fg -- ) 40 SWAP COLOR ;
: CURSOR-ON  ( -- ) ." %e[?25h" ;
: CURSOR-OFF ( -- ) ." %e[?25l" ;

// for Jump table stuff
// : V,  ( n-- )  vhere ! CELL va +! ;
// : VC, ( c-- )  vhere C! 1 va +! ;
// vhere constant jt
// ha @ V, : T0 ." test1 " ;
// ha @ V, : T0 ." test2 " ;
// ha @ V, : T0 ." test3 " ;
// ha @ V, : T0 ." test4 " ;
// ha @ V, : T0 ." test5 " ;
// 5 0 DO I CELLS jt + @ EXECUTE LOOP