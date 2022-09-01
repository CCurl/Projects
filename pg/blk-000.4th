[def] test  ( t f-- ) DO I . LOOP RET
[def] test1 ( -- ) test RET
[def] cr    ( -- ) 10 EMIT RET
[def] start ( -- ) 65 EMIT RET
[def] end   ( -- ) 66 EMIT RET
[def] test2 ( t f-- ) start DO LOOP end RET
[def] life 21 1 test cr 10000000 0 test2 cr cr WORDS cr RET
