\ Support for local variables, 5 at a time
\
\ Examples:
\ : btw  ( n h l--f ) +locs  >3 >2 >1  x2 x1 <   x1 x3 <   and  -locs ;
\ : btwi ( n h l--f ) +locs  >3 >2 >1  x2 x1 <=  x1 x3 <=  and  -locs ;

' locs loaded?

variable locs 50 cells allot
val T8
(val) T9
0 T9 !

: +locs T8 5 + 45 min T9 ! ;
: -locs T8 5 -  0 max T9 ! ;
: T0 T8 + cells locs + @ ;
: T1 T8 + cells locs + ! ;

: x1  0 T0 ;   : >1  0 T1 ;
: x2  1 T0 ;   : >2  1 T1 ;
: x3  2 T0 ;   : >3  2 T1 ;
: x4  3 T0 ;   : >4  3 T1 ;
: x5  4 T0 ;   : >5  4 T1 ;
