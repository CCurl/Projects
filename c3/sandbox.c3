\ A sandbox for messing around

load string.c3

variable p 100 allot
variable q 100 allot
: ct cr count type ;
p s-trunc
p '-' s-catc p ct
p s" hi-" s-cpy p ct
p s" there-" s-cat p ct
p 'a' s-catc p ct
q s" hi-" s-cpy q ct
p q s-eq cr .
p s" hi-" s-cpy
p q s-eq cr .
p s" HI-" s-cpy
p q s-eq cr .
p q s-eq-i cr .

: extract-field ( from to fld delim-- )
    +regs s4 s3 s2 1+ s1
    r2 s-trunc
    r3 1 < if -regs exit then
    r3 1 > if 
        r3 1 do r1 r4 s-findc s1 
            r1 if i1 else r3 +i then
        loop
    then
    r1 0= if -regs exit then
    0 s8 begin 
        r1 c@ s5 r5 0= r5 r4 = or
        if 1 s8 else r2 r5 s-catc i1 then
    r8 until -regs ;

vhere 100 + s1 cr
s" aaa" r1 1 ',' extract-field r1 count type ." ,"
s" 111,222,333,444" r1 1 ',' extract-field r1 count type ." ,"
s" 111,222,333,444" r1 2 ',' extract-field r1 count type ." ,"
s" 111,222,333,444" r1 3 ',' extract-field r1 count type ." ,"
s" 111,222,333,444" r1 4 ',' extract-field r1 count type ." ,"
s" 111,222,333,444" r1 5 ',' extract-field r1 count type ." ,"
s" 111,222,333,444" r1 9 ',' extract-field r1 count type ." ,"

load-abort

\ Some trigonometry stuff ...
var xf  (var) (xf)  : >xf  (xf)  ! ;
var xt  (var) (xt)  : >xt  (xt)  ! ;
var yf  (var) (yf)  : >yf  (yf)  ! ;
var yt  (var) (yt)  : >yt  (yt)  ! ;
var adj (var) (adj) : >adj (adj) ! ;
var opp (var) (opp) : >opp (opp) ! ;
var hyp (var) (hyp) : >hyp (hyp) ! ;

: sqr dup * ; inline

\ The Babylon square root algorithm
: T0 ( n--sqrt ) dup 4 / begin >r dup r@ / r@ + 2 / dup r> - 0= until nip ;
: sqrt ( n--0|sqrt ) dup 0 > if T0 else drop 0 then ;

: .00 ( n-- ) 100 /mod (.) '.' emit abs <# # # #> #P ;
: .adj adj .00 ; : .opp opp .00 ; : .hyp hyp .00 ;
: .point '(' emit swap (.) ',' emit (.) ')' emit ;
: .from xf xt .point ; : .to yf yt .point ;
: .from-to ." from ".from ."  to " .to ;
: sin opp 100 * hyp / ;
: .sin sin .00 ;
: calc >yt >xt >yf >xf 
    xt xf - 100 * >adj
    yt yf - 100 * >opp 
    adj sqr opp sqr + sqrt >hyp ;
