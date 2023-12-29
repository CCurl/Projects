\ mf.c3 - some Machine-Forth inspired words

' !a+ loaded?

val a  (val) T0  : >a T0 ! ;
: a+1  a 1+ >a ; : a+c   a cell+ >a ;
: @a   a @ ;     : @a+   @a  a+c ;
: !a   a ! ;     : !a+   !a  a+c ;
: c@a  a c@ ;    : c@a+  c@a a+1 ;
: c!a  a c! ;    : c!a+  c!a a+1 ;

val b  (val) T0  : >b T0 ! ;
: b+1  b 1+ >b ; : b+c   b cell+ >b ;
: @b   b @ ;     : @b+   @b  b+c ;
: !b   b ! ;     : !b+   !b  b+c ;
: c@b  b c@ ;    : c@b+  c@b b+1 ;
: c!b  b c! ;    : c!b+  c!b b+1 ;
