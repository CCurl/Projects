[create] cr     [comment] ( -- )  [compile] 13 EMIT 10 EMIT RET
[create] bc     [comment] ( c-- ) [compile] 45 SWAP OVER EMIT EMIT EMIT RET
[create] testJ  [comment] ( t f-- ) [compile] 105 bc DO J . I . LOOP 111 bc cr RET
[create] testI  [comment] ( t f-- ) [compile] DO 6 1 testJ LOOP RET
[exec] 76 DUP EMIT EMIT 77 EMIT cr
[create] start [comment] ( -- ) [compile] 65 EMIT RET
[create] end   [comment] ( -- ) [compile] 66 EMIT RET
[create] test2 [comment] ( t f-- ) [compile] start cr DO LOOP end cr RET
[create] elapsed [comment] ( t-- ) [compile] TIMER SWAP - RET
[create] life  [compile] 21 11 testI cr TIMER 500 1000 DUP * * 0 test2 elapsed . cr cr WORDS cr RET
[exec] life EDIT
