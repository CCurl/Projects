' cmove loaded?

: cmove ( dst src num-- )
    +regs s3 s2 s1
    r3 0 do r2 c@ r1 c! i2 i1 loop
    -regs ;
