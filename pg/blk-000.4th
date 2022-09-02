[define] cr     [comment] ( -- )  [compile] 10 EMIT RET
[define] bc     [comment] ( c-- ) [compile] 45 SWAP OVER EMIT EMIT EMIT RET
[define] testJ  [comment] ( t f-- ) [compile] 105 bc DO J . I . LOOP 111 bc cr RET
[define] testI  [comment] ( t f-- ) [compile] DO 15 11 testJ LOOP RET
[exec] 76 DUP EMIT EMIT 77 EMIT cr
[define] start [comment] ( -- ) [compile] 65 EMIT RET
[define] end   [comment] ( -- ) [compile] 66 EMIT RET
[define] test2 [comment] ( t f-- ) [compile] start DO LOOP end RET
[define] life  [compile] 21 1 testI cr 50000000 0 test2 cr cr WORDS cr RET
[exec] life EDIT
