\ Random words

: loaded? if drop drop forget-1 99 state ! then ;
' seed loaded?

var seed cell allot

: random ( --n )
    seed @ dup 0= if drop timer then
    dup 8192 * xor
    dup 131072 / xor
    dup 32 * xor
    dup seed ! ;

: rand-max ( max--n ) random abs swap mod ;
