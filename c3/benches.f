\ Some benchmarks

: ms . ." usec" ;
: elapsed timer swap - ms ;
: mil #1000 dup * * ;

: prime?  ( n 3--f )
    begin
        over over /mod swap
        if over < if drop exit then
        else  drop = exit
        then
        1+ 1+
    again ;

variable num
: num-primes 4 s9 11 do
        i 3 prime? if i9 then 1 +i
    loop r9 . ;

: bm1 cr ." Bench 1: -while loop, " dup . ." iterations ... "
    timer swap begin 1- -while drop elapsed ;

: bm2 cr ." Bench 2: while/repeat loop, " dup . ." iterations ... "
    timer swap begin dup while 1- repeat drop elapsed ;

: bm3 cr ." Bench 3: register decrement loop, " dup . ." iterations ... "
    s1 timer begin r1- 0= until elapsed ;

: bm4 cr ." Bench 4: empty do/loop, " dup . ." iterations ... "
    timer swap 0 do loop elapsed ;

: bm5 cr ." Bench 5: empty for/next, " dup . ." iterations ... "
    timer swap for next elapsed ;

: bm6 cr ." Bench 6: number of primes in " dup . ." ... "
    timer swap num-primes elapsed ;

\ load-abort

250 mil bm1
250 mil bm2
250 mil bm3
250 mil bm4
250 mil bm5
  2 mil bm6

\ Mandelbrot
: m-hdr ." The Mandelbrot Set" cr ;
: sqr dup * ; inline

1000000 constant TGT

: doQ r1 sqr r6 / s3
      r2 sqr r6 / s4
      r3 r4 + TGT > if r5 s7 exit then 
      r1 r2 * 100 / r9 + s2
      r3 r4 - r8 + s1
      i5 ;
: doL 0 s1 0 s2 0 s5 r6 s7 begin i0 doQ r5 r7 < 0= until ;
: doO doL r5 40 + dup 126 > if drop 32 then emit ;
: doX -490 s8 95 0 do doO r8  8 + s8 loop cr ;
: doY -340 s9 35 0 do doX r9 20 + s9 loop ;
200 s6
: mbrot m-hdr 0 s0 timer doY timer swap -
    cr r0 . ."  iterations, time=" ms ;
