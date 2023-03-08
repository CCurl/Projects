\ Support for local variables, 5 at a time
\
\ Examples:
\ : btw  ( n h l--f ) +locs  >l3 >l2 >l1  l2 l1 <   l1 l3 <   and  -locs ;
\ : btwi ( n h l--f ) +locs  >l3 >l2 >l1  l2 l1 <=  l1 l3 <=  and  -locs ;

: loaded? if drop drop forget-1 99 state ! then ;
' locs loaded?

var locs 50 cells allot
var lb
(var) (lb)

: >lb (lb) ! ;
0 >lb

: +locs lb 5 + 45 min >lb ;
: -locs lb 5 -  0 max >lb ;
: loc> lb + cells locs + @ ;
: >loc lb + cells locs + ! ;

: l1  0 loc> ;   : >l1  0 >loc ;
: l2  1 loc> ;   : >l2  1 >loc ;
: l3  2 loc> ;   : >l3  2 >loc ;
: l4  3 loc> ;   : >l4  3 >loc ;
: l5  4 loc> ;   : >l5  4 >loc ;
