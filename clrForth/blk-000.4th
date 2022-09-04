[create] cr     [comment] ( -- )  [compile] 10 EMIT RET
[create] bc     [comment] ( c-- ) [compile] 45 SWAP OVER EMIT EMIT EMIT RET
[create] testJ  [comment] ( t f-- ) [compile] 105 bc DO J . I . LOOP 111 bc cr RET
[create] testI  [comment] ( t f-- ) [compile] DO 15 11 testJ LOOP RET
[exec] 76 DUP EMIT EMIT 77 EMIT cr
[create] start [comment] ( -- ) [compile] 65 bc RET
[create] end   [comment] ( -- ) [compile] 66 bc RET
[create] test2 [comment] ( t f-- ) [compile] start cr DO LOOP end cr RET
[create] elapsed [comment] ( t-- ) [compile] TIMER SWAP - RET
[create] mil [comment] ( a--b ) [compile] 1000 DUP * * RET
[exec] 500 [const] BM
[var] ttt [exec] BM mil ttt !
[create] life  [compile] 21 1 testI cr TIMER ttt @ 0 test2 elapsed . cr cr WORDS cr RET
[exec] life EDIT
