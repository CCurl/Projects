// Support for local variables, 5 at a time
//
// Examples:
// : btw  ( n h l--f ) +locs  >3 >2 >1  x2 x1 <   x1 x3 <   and  -locs ;
// : btwi ( n h l--f ) +locs  >3 >2 >1  x2 x1 <=  x1 x3 <=  and  -locs ;

' +locs loaded?

variable locs 50 cells allot
val lb   (val) (lb)   : >lb ;
0 (lb) !

: +locs lb 5 + 45 min (lb) ! ;
: -locs lb 5 -  0 max (lb) ! ;

: T0 ( n--a )  lb + cells locs + ;
: T1 ( n--x )  T0 @ ;
: T2 ( x n-- ) T0 ! ;
: T3 ( n--x )  T0 dup @ swap ++ ;
: T4 ( n--x )  T0 dup @ swap -- ;

: rA  0 T1 ;   : sA  0 T2 ;   : rA++  0 T3 ;   : rA--  0 T4 ;
: rB  1 T1 ;   : sB  1 T2 ;   : rB++  1 T3 ;   : rB--  1 T4 ;
: rC  2 T1 ;   : sC  2 T2 ;   : rC++  2 T3 ;   : rC--  2 T4 ;
: rD  3 T1 ;   : sD  3 T2 ;   : rD++  3 T3 ;   : rD--  3 T4 ;
: rE  4 T1 ;   : sE  4 T2 ;   : rE++  4 T3 ;   : rE--  4 T4 ;
