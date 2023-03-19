\ String words

' str-end loaded?

val s  (val) (s)  : >s (s) ! ;  : s++ s (s) ++ ;
val d  (val) (d)  : >d (d) ! ;  : d++ d (d) ++ ;

: str-end ( s--e ) dup c@ + 1+ ; inline
: str-nullterm ( s--s ) 0 over str-end c! ;
: str-catc ( c dst--dst ) tuck str-end c! dup c++ str-nullterm ;
: str-catd ( d dst--dst ) swap dup 9 > if 7 + then '0' + swap str-catc ;
: str-cat ( src dst--dst )
    over c@ 0= if nip exit then
    dup >r str-end >d >s s++ c@
    begin
        dup if  s++ c@ d++ c! r@ c++ 1-
        else    d c! r> exit
        then
    again ;
: str-trunc ( s--s ) 0 2dup over c! 1+ c! ;
: str-cpy ( src dst--dst ) str-trunc str-cat ;
: str-catn ( n dst--dst ) 
    >r <# #s #>
    begin r@ str-catc drop ?dup while
    r> ;
: str-scat ( dst src--dst ) swap str-cat ;
: str-scatn ( dst src--dst ) swap str-catn ;
: str-findc ( s c--a|0 ) +regs 0 s8 s2 count swap s1 
    0 do r1 i + s3 r3 c@ s4 
        r4 r2 = if r3 s8 999 +i then
        r4  0=  if 999 +i then
    loop r8 -regs ;
