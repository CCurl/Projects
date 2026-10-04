12.2 444.5 + . : t1 11 . ; : t2 22 . ;
: t3 t1 t2 t1 ; t3
: space 32 emit ;
cr 355 113 / . cr
: hello ." hi there" ; hello
s" -test" type ." -another"
: tt if 65 emit exit then 66 emit ; cr 1 tt 0 tt
: tf 10 for i . next ; space tf
: bm 100000000 for next ; cr bm
." -done" cr words