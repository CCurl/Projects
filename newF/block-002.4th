// Sandbox

forget

: w ." vhere: " here (.) ." , vhere: " vhere (.) ." , last: " last (.) ;

: rand-mod rand swap mod ; inline
: rand-and rand and ;      inline
: rand-1k rand $3ff and ;  inline

cr ."  100 mod: " #100 rand-mod .
cr ."  $FF and: " $0fF rand-and .
cr ."  1k     : " rand-1k .

: rl 2 load ;
