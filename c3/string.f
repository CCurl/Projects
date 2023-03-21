\ string.f - some string handling words

' s-len loaded?

load memory.f

: s-len ( str--len ) c@ ; inline
: s-end ( str--end ) dup s-len + 1+ ; inline
: s-cpy ( dst src-- ) dup s-len 2+ cmove ;
: s-trunc ( str-- ) 0 swap 2dup 1+ c! c! ;
: s-cat ( dst src-- )
    +regs s2 s1
    r1 s-end r2 count 1+ cmove
    r1 s-len r2 s-len + r1 c!
    -regs ;
: s-catc ( str ch-- )
    +regs s2 s1 r1 s-end s3
    r2 r3 c! 0 r3 1+ c!
    r1 c++ -regs ;
: s-catd ( str d-- )
    dup 9 > if 7 + then
    s-catc ;
: s-catn ( dst n-- ) 
    swap >r <# #s #>
    begin r@ swap s-catc ?dup while
    rdrop ;
: s-scatn ( n dst-- )   swap s-catn ;
: s-findc ( str ch--a|0 ) \ NB: str is not counted
    +regs s2 s1 0 s8
    r1 s-len 0 do
        r1 c@ s3 r3 r2 = r3 0= or
        if r1 s8 999 +i then i1
    loop r8 -regs ;
