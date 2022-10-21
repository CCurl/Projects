[define] cr     [comment] (--)  [compile] 13 EMIT 10 EMIT RET
[define] bc     [comment] (c--) [compile] 45 SWAP OVER EMIT EMIT EMIT RET
[define] testJ  [comment] (t f--) [compile] 105 bc DO J . I . LOOP 111 bc cr RET
[define] testI  [comment] (t f--) [compile] DO 15 11 testJ LOOP RET
[exec] 76 DUP EMIT EMIT 77 EMIT cr
[define] start   [comment] (--) [compile] 65 EMIT RET
[define] end     [comment] (--) [compile] 66 EMIT RET
[define] elapsed [comment] (t--) [compile] TIMER SWAP - RET
[define] bench   [comment] (n--) [compile] DUP . TIMER SWAP 0 DO LOOP elapsed . cr RET
[define] mil     [comment] (x--y) [compile] 1000 DUP * * RET
[define] main    [compile] 21 1 testI cr 500 mil bench cr WORDS cr RET
[exec] main EDIT