( Random number stuff )

cell var seed
timer seed !

: rand seed @ dup 8192 * xor dup 128 / xor dup 131072 * xor dup seed ! ;
: rand-mod rand abs swap mod ;

: vowels     ( --a )  z" aeiou" ;
: consonants ( --a )  z" bcdfghklmnprstvwz" ;
: rand-v     ( a--a ) 5 rand-mod vowels + c@ s-catc ;
: rand-c     ( a--a ) 17 rand-mod consonants + c@ s-catc ;
: rand-name  ( --a )  pad s-trunc rand-c rand-v rand-c rand-c rand-v rand-c ;
