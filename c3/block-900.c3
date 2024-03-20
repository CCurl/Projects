\ block 900 - a sandbox

' rand loaded?

variable seed
0 seed !

: rand ( -- n )
  seed @ DUP 0= IF DROP timer THEN
  DUP 8192 * XOR
  DUP 131072 / XOR
  DUP 32 * XOR
  DUP seed ! ;

: rand-max ( max -- n )
  rand ABS SWAP MOD ;
