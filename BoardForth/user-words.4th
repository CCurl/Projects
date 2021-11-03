forth definitions
: all 0 ;
: ? @ . ;
: .wordl-hdr cr ."  addr    xt      f L l   name" ;
: _t0 wordsl ; : wordsl .wordl-hdr _t0 ;
: .ch dup bl < if drop '.' then dup $7f = if drop '.' then emit ;
: dump-ch low->high for i c@ .ch next ;

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

: _t0 27 emit '[' emit ;
: gotoXY _t0 (.) ';' emit (.) 'H' emit ;
: cls _t0 ." 2J" 1 dup gotoXY ;

all definitions
marker
