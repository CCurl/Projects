forth definitions
: all 0 ;

variable seed
timer seed !

: random    ( -- x )
    seed @
    dup 13 lshift xor
    dup 17 rshift xor
    dup 5  lshift xor
    dup seed ! ;
 
: rand ( n1 -- n2 ) 
    random abs swap mod ;

: pc 64 ; 
pc definitions

: csi 27 emit '[' emit ;
: gotoXY csi (.) ';' emit (.) 'H' emit ;
: cls csi ." 2J" 0 dup gotoXY ;

all definitions
marker