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
: bm3 cr ." Bench: number of primes in " dup . ." ... "
    timer swap num-primes elapsed ;

250 mil bm1
250 mil bm2
  2 mil bm3

\ Mandelbrot
load regs.f

: m-hdr ." The Mandelbrot Set" cr ;
: sqr dup * ; inline

: doQ rA sqr rS / sC 
      rB sqr rS / sD
      rC rD + rK > if rJ sM exit then 
      rA rB * 100 / rY + sB
      rC rD - rX + sA
      iJ ;
: doL 0 sA 0 sB 0 sJ rS sM begin iT doQ rJ rM < while ;
: doO doL rJ 40 + dup 126 > if drop 32 then emit ;
: doX -490 sX 95 0 do doO rX  8 + sX loop cr ;
: doY -340 sY 35 0 do doX rY 20 + sY loop ;
200 sS 1000000 sK
: mbrot m-hdr 0 sT timer doY timer swap -
    cr rT . ."  iterations, time=" ms ;
