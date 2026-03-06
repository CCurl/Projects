( some tests )

( ANSI color codes )
: csi  27 emit '[' emit ;
: ->cr ( c r-- ) csi (.) ';' emit (.) 'H' emit ;
: cls  csi ." 2J" 1 dup ->cr ;
: fg   csi ." 38;5;" (.) 'm' emit ;
: black    0 fg ;      : red     203 fg ;
: green   40 fg ;      : yellow  226 fg ;
: blue    63 fg ;      : purple  201 fg ;
: cyan   117 fg ;      : grey    246 fg ;
: white  255 fg ;

: ll z" ls -l" system ;
: lg z" lazygit" system ;

( dumping and printing )
: .nwb ( n width base-- )
    base @ >r  base !  >r <# r> 1- for # next #s #> ztype  r> base ! ;
: .hex     ( n-- )  #2 $10 .nwb ;

: aemit ( ch-- )  dup #31 $7F btwi if0 drop '.' then emit ;
: t0    ( addr-- )  +L1 $10 for c@x+ aemit next -L ;
: dump  ( addr n-- )  0 +L3 y@ for
     z@+ if0 x@ cr .hex ." : " then c@x+ .hex space
     z@ $10 = if 0 z! space space x@ $10 - t0 then
   next -L ;

( some benchmarks )
: lap ( --n ) timer ; inline
: .lap ( n-- ) lap swap - space . ." ticks" cr ;

: mil 1000 dup * * ;
: fib ( n--fib ) 1- dup 2 < if drop 1 exit then dup fib swap 1- fib + ;
: t0 ( n a-- ) ztype '(' emit dup (.) ')' emit lap swap ;
: bm-while ( n-- ) z" while " t0 begin 1- -while drop .lap ;
: bm-loop  ( n-- ) z" loop "  t0 for next .lap ;
: bm-fib   ( n-- ) z" fib "   t0 fib space (.) .lap ;
: bm-fibs  ( n-- ) 1 +L1 for x@+ bm-fib next -L ;
: bb ( -- ) 1000 mil bm-loop ;
: bm-all ( -- ) 250 mil bm-while bb 30 bm-fib ;

( simple fixed point )
val fprec@   (val) t1   \ Precision digits
val fscale@  (val) t2   \ Scale value
: fprec! ( n-- ) 1 max t1 ! 1 fprec@ for 10 * next t2 ! ;
: d>f ( n f--m ) swap fscale@ * + ;
: f>d ( m--f n ) fscale@ /mod ;
: f. ( n-- )    f>d (.) '.' emit abs fprec@ 10 .nwb ;
: f* ( a b--c ) * fscale@ / ;
: f/ ( a b--c ) swap fscale@ * swap / ;
: f+ ( a b--c ) + ; inline
: f- ( a b--c ) - ; inline
: i>f ( n--m )  0 d>f ;
: f>i ( n--m )  fscale@ / ;
3 fprec!

10 load

( *** Banner *** )
: .version version <# # # #. # # #. #s 'v' hold #> ztype ;
: .banner
    yellow ." QWC " green .version white ."  - Chris Curl" cr
    yellow ."   Memory: " white mem-sz . ." bytes." cr
    yellow ."     Code: " white vars mem - cell / . ." cells, used: " here . cr
    yellow ."     Vars: " white last vars - . ." bytes, used: " vhere vars - . cr
    yellow ."     Dict: " white dict-end last - .  ." bytes used" cr 
    ." hello." cr ;
.banner
