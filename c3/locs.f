\ Support for local variables, 5 at a time
\
\ Examples:
\ : btw  ( n h l--f ) +locs  >l3 >l2 >l1  l2 l1 <   l1 l3 <   and  -locs ;
\ : btwi ( n h l--f ) +locs  >l3 >l2 >l1  l2 l1 <=  l1 l3 <=  and  -locs ;

: loaded? if drop drop forget-1 99 state ! then ;
' locs loaded?

var locs 50 cells allot
var T8
(var) T9
0 T9 !

: +locs T8 5 + 45 min T9 ! ;
: -locs T8 5 -  0 max T9 ! ;
: T0 T8 + cells locs + @ ;
: T1 T8 + cells locs + ! ;

: l1  0 T0 ;   : >l1  0 T1 ;
: l2  1 T0 ;   : >l2  1 T1 ;
: l3  2 T0 ;   : >l3  2 T1 ;
: l4  3 T0 ;   : >l4  3 T1 ;
: l5  4 T0 ;   : >l5  4 T1 ;
