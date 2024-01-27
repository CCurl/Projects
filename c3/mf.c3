\ mf.c3 - some Machine-Forth inspired words

' !a+ loaded?

val a               >val >a
: a+c  a cell+ >a ; : a+    a 1+ >a ;
: @a   a @ ;        : c@a   a c@ ;
: !a   a ! ;        : c!a   a c! ;
: @a+  @a  a+c ;    : c@a+  c@a a+ ;
: !a+  !a  a+c ;    : c!a+  c!a a+ ;

val b               >val >b
: b+c  b cell+ >b ; : b+    b 1+ >b ;
: @b   b @ ;        : c@b   b c@ ;
: !b   b ! ;        : c!b   b c! ;
: @b+  @b  b+c ;    : c@b+  c@b b+ ;
: !b+  !b  b+c ;    : c!b+  c!b b+ ;
