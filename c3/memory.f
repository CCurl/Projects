' cmove loaded?

: cmove ( dst src num-- )
    +regs s3 s2 s1
    r3 0 do r2 c@ r1 c! i2 i1 loop
    -regs ;

: fill ( dst num ch-- )
    +regs s3 s2 s1
    r2 0 do r3 r1 c! i1 loop
    -regs ;
