\ The Mandelbrot Set

variable (x) : x (x) @ ; : >x (x) ! ;
variable (y) : y (y) @ ; : >y (y) ! ;

: header ." The Mandelbrot Set%n" ;

: sqr dup * ; inline

: calc  r1 sqr r8 / s3 
        r2 sqr r8 / s4
        r3 r4 + r7 > if r6 s5 exit then
        r1 r2 * 100 / y + s2
        r3 r4 - x + s1 i6 ;

: output-ch dup '~' > if drop bl then emit ;

: l-loop 0 s1 0 s2 0 s6 r8 s5 begin i9 calc r6 r5 = until ;
: m-loop l-loop r6 40 + output-ch ;
: x-loop -490 >x 95 0 do m-loop x  8 + >x loop cr ;
: y-loop -340 >y 35 0 do x-loop y 20 + >y loop ;

: mbrot header 
    0 s9 200 s8 1000000 s7
    timer y-loop timer
    swap - cr r9 ." %d iterations, time=%d" ;
