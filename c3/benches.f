\ Some benchmarks

: ms . ." usec" ;
: elapsed timer swap - ms ;
: mil #1000 dup * * ;

: prime? begin ( n 3--f )
        over over /mod swap if  \ test 2
            over < if drop exit then
        else 
            drop = exit
        then
        1+ 1+
    again ;

var num cell allot
: num-primes 4 num ! 11 do
        i 3 prime? if num ++ then 1 +i
    loop num ? ;

: bm1 cr ." Bench: decrement loop, " dup . ." iterations ... "
    timer swap begin 1- dup while drop elapsed ;
: bm2 cr ." Bench: empty do loop, " dup . ." iterations ... "
    timer swap 0 do loop elapsed ;
: bm3 cr ." Bench: register decrement loop, " dup . ." iterations ... "
    s9 timer begin d9 r9 while elapsed ;
: bm4 cr ." Bench: number of primes in " dup . ." ... "
    timer swap num-primes elapsed ;

250 mil bm1
250 mil bm2
250 mil bm3
  2 mil bm4

\ Mandelbrot
: m-hdr ." The Mandelbrot Set" cr ;
: sqr dup * ; inline

1000000 const TGT

: doQ r1 sqr r6 / s3
      r2 sqr r6 / s4
      r3 r4 + TGT > if r5 s7 exit then 
      r1 r2 * 100 / r9 + s2
      r3 r4 - r8 + s1
      i5 ;
: doL 0 s1 0 s2 0 s5 r6 s7 begin i0 doQ r5 r7 < while ;
: doO doL r5 40 + dup 126 > if drop 32 then emit ;
: doX -490 s8 95 0 do doO r8  8 + s8 loop cr ;
: doY -340 s9 35 0 do doX r9 20 + s9 loop ;
200 s6
: mbrot m-hdr 0 s0 timer doY timer swap -
    cr r0 . ."  iterations, time=" ms ;
