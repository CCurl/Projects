\ Words for saving / restoring registers

var T0 100 cells allot
var T1   1 cells allot
0 T1 !

: T2 ( -- )   T1 @ 1+  0 max T1 ! ;
: T3 ( n-- )  T2 T1 @ cells T0 + ! ;

: T4 ( -- )   T1 @ 1- 99 min T1 ! ;
: T5 ( --n )  T1 @ cells T0 + @ T4 ;

: +regs ( n-- )  dup 
    0 do i cells regs + @ T3 loop ;

: -regs ( n-- )  dup 
    begin 1- T5 over cells regs + ! dup while drop ;

: .regs 10 0 do i cells regs + @ . loop ;
