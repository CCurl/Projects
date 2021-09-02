forget 
all definitions
: life-words 19 ;
life-words definitions

 45       constant rows  inline
120       constant cols  inline
rows cols * constant grid-sz

variable grid-1      inline grid-sz allot
variable grid-2      inline grid-sz allot

variable (grid-f)   inline
variable (grid-t)   inline

: grid-f  (grid-f) @ ; inline
: grid-f! (grid-f) ! ; inline
: grid-t  (grid-t) @ ; inline
: grid-t! (grid-t) ! ; inline

: init-grids grid-1 grid-f! grid-2 grid-t! ;

: switch-grids grid-f grid-1 = 
	if    grid-2 grid-f! grid-1 grid-t! 
	else  grid-1 grid-f! grid-2 grid-t!  
	then ;

variable num inline
: alive? ( a -- a ) dup c@ '*' = if num @ 1+ num ! then ;
: count-neighbors ( a -- n ) 
	>r 0 num !
	r@ cols - 1- alive? 1+ alive? 1+ alive? drop
	r@        1- alive? 1+        1+ alive? drop
	r> cols + 1- alive? 1+ alive? 1+ alive? drop
	num @ ;
	
: cell-status ( x n -- c )
	dup 2 = if drop          ret then
	dup 3 = if drop drop '*' ret then
	drop drop bl ;
	
: process-grid
    cols 1+ grid-sz cols - 1- 1- for 
        i grid-f + 
        dup c@ swap 
        count-neighbors cell-status
        i grid-t + c!
    next ;

variable wait-time 0 wait-time !

: .grid 0 0 gotoXY 1 rows 1- for dup cols type cr cols + next drop ;
: gen process-grid grid-t .grid ;
: gens 1 for 
        gen space '(' emit i . ')' emit 
        wait-time @ ms 
        switch-grids
    next ;

: clear-grid dup grid-sz + 1- for bl i c! next ;
: clear-grids grid-1 clear-grid grid-2 clear-grid ;

: reset-grid dup grid-sz + 1- for '*' 9 rand cell-status i c! next ;
: reset-grids grid-1 reset-grid grid-2 reset-grid ;

: offset ( grid c r -- o ) cols * + + ;
: star-cell ( o -- o ) dup '*' swap c! ;
: blank-cell ( o -- o ) dup bl swap c! ;
: glider-ur ( grid c r -- )
    offset
    cols - star-cell
    cols + star-cell 1+ star-cell
    cols + star-cell 1- 1- star-cell
    drop ;
  
: glider-ul ( grid c r -- )
    offset
    cols - star-cell
    cols + star-cell 1- star-cell
    cols + star-cell 1+ 1+ star-cell
    drop ;

init-grids clear-grids grid-f .grid
reset-grids
500 gens
