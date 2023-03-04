\ Support for temporary variables
\
\ Examples:
\ : btw  ( n h l--f ) +tmps  s3 s2 s1  r2 r1 <   r1 r3 <   and  -tmps ;
\ : btwi ( n h l--f ) +tmps  s3 s2 s1  r2 r1 <=  r1 r3 <=  and  -tmps ;

var tmps 100 cells allot
var tb (var) (tb)

: >tb (tb) ! ;
0 >tb

: +tmps tb 10 + 90 min >tb ;
: -tmps tb 10 -  0 max >tb ;
: tmp> tb + cells tmps + @ ;
: >tmp tb + cells tmps + ! ;

: r0  0 tmp> ;   : s0  0 >tmp ;
: r1  1 tmp> ;   : s1  1 >tmp ;
: r2  2 tmp> ;   : s2  2 >tmp ;
: r3  3 tmp> ;   : s3  3 >tmp ;
: r4  4 tmp> ;   : s4  4 >tmp ;
: r5  5 tmp> ;   : s5  5 >tmp ;
: r6  6 tmp> ;   : s6  6 >tmp ;
: r7  7 tmp> ;   : s7  7 >tmp ;
: r8  8 tmp> ;   : s8  8 >tmp ;
: r9  9 tmp> ;   : s9  9 >tmp ;
