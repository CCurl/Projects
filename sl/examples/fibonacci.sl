\ Fibonacci-like sequence demonstration
\ This calculates the first few Fibonacci numbers manually

\ Start with 0 and 1
0 1

\ Calculate next: 0 1 -> 0 1 1
DUP . SWAP OVER + 

\ Calculate next: 0 1 1 -> 0 1 1 2
DUP . SWAP OVER +

\ Calculate next: 1 1 2 -> 1 1 2 3
DUP . SWAP OVER +

\ Calculate next: 1 2 3 -> 1 2 3 5
DUP . SWAP OVER +

\ Calculate next: 2 3 5 -> 2 3 5 8
DUP . SWAP OVER +

\ Calculate next: 3 5 8 -> 3 5 8 13
DUP . SWAP OVER +

\ Final results
DUP . SWAP OVER +
DUP .

CR
