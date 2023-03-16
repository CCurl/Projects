\ A sandbox for messing around
forget

load strings.f

: extract-field ( from to fld delim-- )
    +regs s4 s3 s2 count drop s1
    r3 1 > if 
        r3 1 do r1 r4 str-findc s1 
            r1 if i1 else r3 +i then
        loop
    then
    r2 str-trunc drop
    r1 0= if exit then
    0 s8 begin 
        r1 c@ s5 r5 0= r5 r4 = or s8
        r8 0= if r5 r2 str-catc drop i1 then
    r8 until -regs ;

vhere 100 + s1 s" 111,222,333,444" r1 1 ',' extract-field r1 count type
vhere 100 + s1 s" 111,222,333,444" r1 2 ',' extract-field r1 count type
vhere 100 + s1 s" 111,222,333,444" r1 3 ',' extract-field r1 count type
vhere 100 + s1 s" 111,222,333,444" r1 4 ',' extract-field r1 count type
vhere 100 + s1 s" 111,222,333,444" r1 5 ',' extract-field r1 count type

load-abort


    0( skip to nth fld ... ) 
    r3 1 [r1 r4 sf 1+ s1]
    0( collect num into r2/to-str .. )
    r2 s5 0( r5: temp char ptr )
    {r1 c@ r5 c! i1 i5 r1 c@ r4 = ~} 0 r5 c! 
    r2 xE sC rC " -extracted integer %d- " 
    rC rM >( " new peak " rC sM )


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
: T0 ( n--sqrt ) dup 4 / begin >r dup r@ / r@ + 2 / dup r> - while nip ;
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
