: cr     13 EMIT 10 EMIT ;
cr cr 12345 . cr
BYE

: bc     ( c--)  45 SWAP OVER EMIT EMIT EMIT ;
: testJ  ( t f--)  105 bc DO J . I . LOOP 111 bc cr ;
: testI   ( t f--)  DO 15 11 testJ LOOP ;
76 DUP EMIT EMIT 77 EMIT cr
: start    ( --)  65 EMIT ;
: end      ( --)  66 EMIT ;
: elapsed  ( t--)  TIMER SWAP - ;
: bench    ( n--)  DUP . TIMER SWAP 0 DO LOOP elapsed . cr ;
: mil      ( x--y)  1000 DUP * * ;
: main    21 1 testI cr 500 mil bench cr WORDS cr ;
main 