// Sandbox

forget

: w ." vhere: " here (.) ." , vhere: " vhere (.) ." , last: " last (.) ;

: test ." hi there" ; test

: rand-1k rand $3ff and ;
: rand-rng rand swap mod ;

: rl 2 load ;
