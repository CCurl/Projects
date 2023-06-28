\ string.c3 - some string handling words

' s-len loaded?

load memory.c3

: lower ( c1--c2 ) dup 'A' 'Z' btw if 32 + then ;
: upper ( c1--c2 ) dup 'a' 'z' btw if 32 - then ;

\ Words for NULL-Terminated strings
\ Compare N chars ... case-sensitive
: sz-eq-n ( str1 str2 N--F )
    +regs s3 s2 s1 1
    begin r3- while
        r1+ c@ r2+ c@ = 0=
        if 1- dup s3 then
    repeat -regs ;

\ Compare N chars ... NOT case-sensitive
: sz-eqi-n ( str1 str2 N--F )
    +regs s3 s2 s1 1
    begin r3- while
        r1+ c@ lower r2+ c@ lower = 0=
        if 1- dup s3 then
    repeat -regs ;

: sz-len ( strz--len ) +regs s1 0 begin r1+ c@ while 1+ repeat -regs ;
: sz-eq  ( str1 str2--f ) dup sz-len 1+ sz-eq-n ;
: sz-eqi ( str1 str2--f ) dup sz-len 1+ sz-eqi-n ;

\ Words for Counted strings
: s-len  ( str--len ) c@ ; inline
: s-end ( str--end ) dup s-len + 1+ ; inline
: s-cpy ( dst src-- ) tuck s-len 2+ cmove ;
: s-trunc ( str-- ) 0 swap 2dup 1+ c! c! ;
: s-cat ( dst src-- )
    +regs s2 s1
    r2 1+ r1 s-end r2 s-len 1+ cmove
    r1 s-len r2 s-len + r1 c!
    -regs ;
: s-catc ( str ch-- )
    +regs s2 dup s1 s-end s3
    r2 r3+ c! 0 r3 c!
    r1 c++ -regs ;
: s-scatc ( c str-- )   swap s-catc ;
: s-catd ( str d-- )    #digit + s-catc ;
: s-catn ( str n-- ) 
    +regs <# #s #> #bufp @ s2 s1
    begin r2+ c@ ?dup while r1 s-scatc repeat
    -regs ;
: s-scatn ( n str-- )   swap s-catn ;
: s-findc ( strz ch--a|0 )
    +regs s2 s1 0 s8
    r1 s-len 0 do
        r1 c@ s3 r3 r2 = r3 0= or
        if r1 s8 999 +i then i1
    loop r8 -regs ;
: s-eq  ( s1 s2--f ) swap 1+ swap count 1+ sz-eq-n ;
: s-eqi ( s1 s2--f ) swap 1+ swap count 1+ sz-eqi-n ;

\ Make counted string from NULL-Terminated string
: s-from-sz ( s sz-- )
    +regs s2 s1   r1 s-trunc
    begin r2+ c@ ?dup while r1 s-scatc repeat
    -regs ;
: sz-to-s ( s sz-- ) swap s-from-sz ;
