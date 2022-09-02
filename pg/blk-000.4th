[define] test  [comment] ( t f-- ) [compile] DO I . LOOP RET
[define] test1 [comment] ( -- ) [compile] test RET
[define] cr    [comment] ( -- ) [compile] 10 EMIT RET
[exec] 76 DUP EMIT EMIT 77 EMIT cr
[define] start [comment] ( -- ) [compile] 65 EMIT RET
[define] end   [comment] ( -- ) [compile] 66 EMIT RET
[define] test2 [comment] ( t f-- ) [compile] start cr DO LOOP end RET
[define] life  [compile] 21 1 test cr 50000000 0 test2 cr cr WORDS cr RET
[exec] life EDIT
