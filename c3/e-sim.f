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

variable (going)
: going (going) @ ;
: >going (going) ! ;

: up?   ( n--f ) up = ;
: down? ( n--f ) down = ;

variable tgts #floors cells allot

: at-floor? ( n--f ) floor = ;

: press ( n-- )    1 swap cells tgts + ! ;
: unpress ( n-- )  0 swap cells tgts + ! ;
: pressed? ( n--f ) cells tgts + @ ;

: dash '-' emit ; inline
: star '*' emit ; inline
: stars 0 do star loop ;
: spaces 0 do space loop ;

: .[n] ( n-- )
    dup pressed? if 31 43 else 0 0 then color
    '[' emit (.) ']' emit 0 0 color
    2 spaces ;

: .buttons ( -- )
    3 30 ->xy   4 1 do i .[n] loop
    5 30 ->xy   7 4 do i .[n] loop
    7 30 ->xy  10 7 do i .[n] loop ;

: .going ( -- )
    11 34 ->xy
    going up?   if ." -up-" then
    going down? if ." -down-" then
    6 spaces ;

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
    cur-off
    ." elevator sim ..." cr
    .buttons .going
    1 1 ->xy   #floors .floors
    cur-on ;

: init ( -- )
    up >going
    3 press 5 press 8 press
    home >floor ;

: sim ( -- ) .sim ;

cls  init   sim
bye
