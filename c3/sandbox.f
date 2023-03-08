\ A bandbox for messing around
forget

\ Test stop-load
99 state !
." this should not appear!"

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
: (sqrt) ( n--sqrt*100 ) 10000 * dup 4 / begin >r dup r@ / r@ + 2 / dup r> - while nip ;
: sqrt ( n--0|sqrt*100 ) dup 0 > if (sqrt) else drop 0 then ;

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
    adj 100 / sqr opp 100 / sqr + sqrt >hyp ;
