\ Math operations in SL

\ Simple arithmetic
5 3 + .     \ 8
10 4 - .    \ 6
6 7 * .     \ 42
20 4 / .    \ 5
17 5 MOD .  \ 2
CR

\ Stack operations
1 2 3 DUP . . . .  \ 3 3 2 1
CR

5 10 SWAP . .      \ 5 10
CR

\ Comparison
5 5 = .    \ -1 (true)
5 3 = .    \ 0 (false)
3 5 < .    \ -1 (true)
5 3 > .    \ -1 (true)
CR
