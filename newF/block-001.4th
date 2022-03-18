: +! swap over @ + swap ! ;

: here  (here)  @ ;
: c, here c! 1 (here) +! ;
: , here ! cell (here) +! ;

: emit [ ',' c, ] ; inline
: . (.) 32 emit ; inline
: nip swap drop ; inline
: test 1 2 3 + + . ;
test
333 444 over . nip .
