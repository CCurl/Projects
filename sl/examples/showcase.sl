\ ================================================================
\ SL (Simple Lang) - Feature Showcase
\ ================================================================

\ ----- Arithmetic Operations -----
65 EMIT 114 EMIT 105 EMIT 116 EMIT 104 EMIT 109 EMIT 101 EMIT 116 EMIT 105 EMIT 99 EMIT 58 EMIT 32 EMIT
5 3 + .         \ Addition: 8
10 4 - .        \ Subtraction: 6
6 7 * .         \ Multiplication: 42
20 4 / .        \ Division: 5
17 5 MOD .      \ Modulo: 2
CR

\ ----- Comparison Operations -----
67 EMIT 111 EMIT 109 EMIT 112 EMIT 97 EMIT 114 EMIT 101 EMIT 58 EMIT 32 EMIT
5 5 = .         \ Equal (true): -1
5 3 = .         \ Not equal (false): 0
3 5 < .         \ Less than (true): -1
5 3 > .         \ Greater than (true): -1
CR

\ ----- Stack Operations -----
83 EMIT 116 EMIT 97 EMIT 99 EMIT 107 EMIT 58 EMIT 32 EMIT
10 DUP . .      \ DUP: 10 10
20 30 SWAP . .  \ SWAP: 20 30
1 2 OVER . . .  \ OVER: 2 1 2
CR

\ ----- Complex Expression -----
69 EMIT 120 EMIT 112 EMIT 114 EMIT 58 EMIT 32 EMIT
2 3 + 4 * .     \ (2 + 3) * 4 = 20
CR

\ ----- Print "DONE!" -----
68 EMIT 79 EMIT 78 EMIT 69 EMIT 33 EMIT CR
