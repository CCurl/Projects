[def] test 100 0 DO I . LOOP RET
[def] test1 test RET
[def] start 65 EMIT RET
[def] END [com] testing 1 2 3 [cmp] 66 EMIT RET
[exe] [com] 100 0 test
[def] test2 100000000 0 start DO LOOP END RET
