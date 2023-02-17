\ String words

: str-end ( s--e ) dup c@ + 1+ ;
: str-nullterm ( s--s ) 0 over str-end c! ;
: str-catc ( c dst--dst ) tuck str-end c! dup c++ str-nullterm ;
: str-catd ( d dst--dst ) swap dup 9 > if 7 + then '0' + swap str-catc ;
: str-cat ( src dst--dst )
    dup >r str-end >d >s s++ c@
    begin
        dup if   s++ c@ d++ c! r@ c++ 1-
        else   d c! r> exit
        then
    again ;
: str-trunc ( s--s ) 0 2dup over c! 1+ c! ;
: str-cpy ( src dst--dst ) str-trunc str-cat ;
: str-catn ( n dst-- ) 
    >r <# #s drop
    begin r@ str-catc drop ?dup while
     r> ;
: str-scat ( dst src--dst ) swap str-cat ;
: str-scatn ( dst src--dst ) swap str-catn ;
