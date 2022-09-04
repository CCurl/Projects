[create] cr     [comment] ( -- )  [compile] 10 EMIT RET
[create] bc     [comment] ( c-- ) [compile] 45 SWAP OVER EMIT EMIT EMIT RET
[create] testJ  [comment] ( t f-- ) [compile] 105 bc DO J I * . LOOP 111 bc RET
[create] testI  [comment] ( t f-- ) [compile] DO 4 1 testJ LOOP RET
[comment] 76 DUP EMIT EMIT 77 EMIT cr
[create] start [comment] ( -- ) [compile] 65 bc RET
[create] end   [comment] ( -- ) [compile] 66 bc RET
[create] bench [comment] ( t f-- ) [compile] start cr DO LOOP end cr RET
[create] elapsed [comment] ( t-- ) [compile] TIMER SWAP - RET
[create] mil [comment] ( a--b ) [compile] 1000 DUP * * RET
[exec] 500 [const] BM [var] ttt [exec] BM mil ttt !
[create] life  [compile] 16 10 testI cr TIMER ttt @ 0 bench elapsed . cr cr WORDS cr RET
[create] IF [compile] 987654 . RET [exec] IMMEDIATE
[create] THEN [compile] 456789 . RET [exec] IMMEDIATE
[create] fgh [compile] IF THEN RET
[exec] life EDIT
[exec] IF THEN
