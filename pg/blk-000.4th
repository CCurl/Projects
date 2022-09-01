: test  ( t f-- ) DO I . LOOP RET
: test1 ( -- ) test RET
: cr    ( -- ) 10 EMIT RET
: start ( -- ) 65 EMIT RET
: end   ( -- ) 66 EMIT RET
: test2 ( t f-- ) start DO LOOP end RET
: life 21 1 test cr 10000000 0 test2 cr cr WORDS cr RET
