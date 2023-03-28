next-word core.f drop 1- (load)

load screen.f

17 constant floor-w
 9 constant #floors
 1 constant home
 1 constant up
-1 constant down

variable (floor)
: floor   (floor) @ ;
: >floor  (floor) ! ;

: at-floor? ( n--f ) floor = ;

: dash '-' emit ; inline
: star '*' emit ; inline
: stars 0 do star loop ;
: spaces 0 do space loop ;

: .fb ( -- )   cr floor-w stars ;
: .elev ( -- )   ." [___]";
: .elev? ( fl-- )   at-floor? if .elev else 5 spaces then ;
: .floor ( fl-- )
    cr star
    1 spaces   dash dup (.) dash
    3 spaces   .elev?
    3 spaces   star ;
: .floors ( n-- )
    .fb
    1+ 1 do #floors 1+ i - .floor .fb loop
    cr cr ;
: .sim ( -- )
    cur-off   1 1 ->xy
    ." elevator sim ..." cr
    #floors .floors
    cur-on ;

: init ( -- )
    home >floor ;

: sim ( -- ) .sim ;

cls   init   sim
bye
