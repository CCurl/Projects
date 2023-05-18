\ Random words

' seed loaded?

variable seed

: random ( --n )
    seed @ dup 0= if drop timer then
    dup 8192 * xor
    dup 131072 / xor
    dup 32 * xor
    dup seed ! ;

: rand-max ( max--n ) random abs swap mod ;
