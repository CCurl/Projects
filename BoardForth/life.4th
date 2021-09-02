all definitions
: life-words 19 ;
life-words definitions

 50       constant rows
120       constant cols
rows cols * constant grid-sz

variable grid-1      inline grid-sz allot
variable grid-2      inline grid-sz allot
variable (grid-f)   inline
variable (grid-t)   inline

: grid-f  (grid-f) @ ; inline
: grid-f! (grid-f) ! ; inline
: grid-t  (grid-t) @ ; inline
: grid-t! (grid-t) ! ; inline

: switch-grids grid-f grid-1 = 
	if    grid-2 grid-f! grid-1 grid-t! 
	else  grid-1 grid-f! grid-2 grid-t!  
	then ;

variable num inline
: alive? ( a -- ) c@ '*' = if num @ 1+ num ! then ;
: count-neighbors ( a -- n  ) 
	>r 0 num !
	r@ cols - dup alive? dup 1- alive? 1+ alive?
	r@ dup 1- alive? 1+ alive?
	r> cols + dup alive? dup 1- alive? 1+ alive?
	num @ ;
	
: cell-status ( x n -- c )
	dup 2 = if drop          ret then
	dup 3 = if drop drop '*' ret then
	drop drop bl ;
	
: process-grid cols 1+ grid-sz cols - 1- 1-
	for 
		i grid-f + 
    dup c@ swap 
    count-neighbors cell-status
		i grid-t + c!
	next ;

variable wait-time 0 wait-time !
: .grid 0 0 gotoXY 1 rows 1- for dup cols type cr cols + next drop ;
: 1gen switch-grids process-grid grid-t .grid ;
: gens 1 for 1gen space '(' emit i . ')' emit wait-time @ ms next ;

: reset-grid dup grid-sz + 1- for '*' 9 rand cell-status i c! next drop ;
: reset-grids grid-1 reset-grid grid-2 reset-grid switch-grids ;
reset-grids
