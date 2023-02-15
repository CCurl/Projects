\ String words

: str-end      ( s--e )         dup c@ + 1+ ;
: str-nullterm ( s--s )         0 over str-end c! ;
: str-trunc    ( s--s )         0 over c! str-nullterm ;
: str-catc     ( c dst--dst )   >r r@ c++ r@ str-end c! r> str-nullterm ;
: str-catd     ( d dst--dst )   swap dup 9 > if 7 + then '0' + swap str-catc ;
\ : str-catn     ( n dst-- )      0 SWAP str-catnw ;


1000 1001 1002 1003 1004
2     a     b    0
