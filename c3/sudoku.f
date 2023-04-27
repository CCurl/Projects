\ SUDOKU     Worlds most difficult to solve - Test FORTH speed :-)
\ From: https://see.stanford.edu/materials/icspacs106b/H19-RecBacktrackExamples.pdf
\
\ A straightforward port from the original C++ to Forth
\ OBS: I can not find the FORTH author of this program. Jan Langevad (JL)

\ Some useful constants

0 constant unassigned
3 constant box-size
9 constant board-size
board-size dup * constant cell-count
board-size 1+ constant puzzle-base

board-size cells constant column-jump
board-size box-size - 1+ cells constant box-jump

0 value MAXDepth 		\ ADDED code JL
0 value TimesAround 	\ ADDED code JL

: puzzle-base!
   puzzle-base base ! ;

\ puzzle-base!   \ Do not store in a constant! ????


\ http://www.telegraph.co.uk/news/science/science-news/9359579/Worlds-hardest-sudoku-can-you-crack-it.html

create puzzle
8 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
0 , 0 , 3 , 6 , 0 , 0 , 0 , 0 , 0 ,
0 , 7 , 0 , 0 , 9 , 0 , 2 , 0 , 0 ,
0 , 5 , 0 , 0 , 0 , 7 , 0 , 0 , 0 ,
0 , 0 , 0 , 0 , 4 , 5 , 7 , 0 , 0 ,
0 , 0 , 0 , 1 , 0 , 0 , 0 , 3 , 0 ,
0 , 0 , 1 , 0 , 0 , 0 , 0 , 6 , 8 ,
0 , 0 , 8 , 5 , 0 , 0 , 0 , 1 , 0 ,
0 , 9 , 0 , 0 , 0 , 0 , 4 , 0 , 0 ,


\ Rather than compare against zero, make the code more meaningful to readers.

: unassigned? ( -- )
    0=  ; 


\ Puzzle access methods

: grid>addr ( n - addr )
    cells puzzle +  ; 

: grid@ ( n -- number )
    grid>addr @  ; 

: grid! ( number n -- )
    grid>addr !  ; 


\ Move and grid position translation methods

: move>xy ( n -- x y )
    board-size /mod  ; 

: move>x ( n -- x )
    board-size mod  ; 

: move>y ( n -- y )
    board-size /  ; 

: xy>move ( x y -- n )
    board-size * +  ; 


\ Row, column and box start positions

: move>row-start ( n -- n )
    move>y board-size *  ; 

: move>column-start ( n -- n )
    move>x  ; 

: box-side-start ( n -- n )
    dup box-size mod -  ; 

: move>box-start ( n -- n )
   move>xy
   box-side-start swap
   box-side-start swap
   xy>move  ; 


\ Used macro helper

: cell-check
	 2dup @ = if drop r> drop exit then  ;


\ Function: used-in-row?
\ ----------------------
\ Returns a boolean which indicates whether any assigned entry
\ in the specified row matches the given number.

\ simplified in Forth by row cells being contiguious in the grid.

: used-in-row? ( number n -- f )
   move>row-start grid>addr
         cell-check
   cell+ cell-check
   cell+ cell-check
   cell+ cell-check
   cell+ cell-check
   cell+ cell-check
   cell+ cell-check
   cell+ cell-check
   cell+ cell-check
   2drop 0 ;


\ Function: used-in-column?
\ -------------------------
\ Returns a boolean which indicates whether any assigned entry
\ in the specified column matches the given number.

\ Very similar to used-in-row?, with the offset incrementing by the board-size

: used-in-column? ( number n -- f )
   move>column-start grid>addr
                 cell-check
   column-jump + cell-check
   column-jump + cell-check
   column-jump + cell-check
   column-jump + cell-check
   column-jump + cell-check
   column-jump + cell-check
   column-jump + cell-check
   column-jump + cell-check
   2drop 0 ;


\ Function: used-in-box?
\ ----------------------
\ Returns a boolean which indicates whether any assigned entry
\ within the specified 3x3 box matches the given number.

\ Convert the loop into a box xy, then calculate an offset to obtain cell value.

: used-in-box? ( number n - f )
   move>box-start grid>addr
              cell-check
        cell+ cell-check
        cell+ cell-check
   box-jump + cell-check
        cell+ cell-check
        cell+ cell-check
   box-jump + cell-check
        cell+ cell-check
        cell+ cell-check
   2drop 0 ;


\ Function: available?
\ --------------------
\ Returns a boolean which indicates whether it will be legal to assign
\ number to the given row,column location. As assignment is legal if it that
\ number is not already used in the row, column, or box.

\ Because Forth doesn't seem to shortcut logical operations, we must explicitly leave early
\ if possible.

: available? ( number n -- f )
   2dup used-in-row?    if 2drop 0 exit then
   2dup used-in-column? if 2drop 0 exit then
        used-in-box? 0= ;


\ Function: solve?
\ ----------------
\ Takes a partially filled-in grid and attempts to assign values to all
\ unassigned locations in such a way to meet the requirements for sudoku
\ solution (non-duplication across rows, columns, and boxes). The function
\ operates via recursive backtracking: it finds an unassigned location with
\ the grid and then considers all digits from 1 to "board-size" in a loop. If a digit
\ is found that has no existing conflicts, tentatively assign it and recur
\ to attempt to fill in rest of grid. If this was successful, the puzzle is
\ solved. If not, unmake that decision and try again. If all digits have
\ been examined and none worked out, return false to backtrack to previous
\ decision point.

: solve? ( n -- f )

   DEPTH MAXDepth max is MAXDepth  \ ADDED code JL
   TimesAround 1+ is TimesAround 	 \ ADDED code JL

   dup cell-count = 
   if drop -1 exit 
   then            \ success!

   dup grid@ 
   if 1+ recurse exit 
   then                 \ if it's already assigned, skip

   10 1 
   ?do                  \ consider all digits
     i over available? 
     if                    	\ if looks promising
       i over grid!     \ make tentative assignment
       dup 1+ recurse 
       if unloop drop -1 exit \ recur, if success, yay!
       then
     then 
   loop
   unassigned over grid!  \ failure, unmake & try again
   drop 0                       \ this triggers backtracking
;

\ Board display

: .board-element ( n -- )
   ."  "
   grid@ dup unassigned? if drop ." - " else .  then
   ."  " ;

: .box-break-vertical ( -- )
   ." |" ;

: .box-break-horizontal ( -- )
   ." ------------+------------+-----------" ;

: .board ( -- )
   cell-count 0 
  do
       i move>x unassigned? 
       if   i move>y box-size mod 0= 
           if  cr cr
              .box-break-horizontal
           then  cr cr
     else
         i move>x box-size mod 0= 
         if   .box-break-vertical
         then
     then
     i .board-element
   loop
   cr cr .box-break-horizontal cr 
;

: game ( -- )

  MS-TICKS 	\ ADDED code JL
  0 is TimesAround  \ ADDED code JL
  0 is MAXDepth	\ ADDED code JL

  0 solve? if
    .board
  else
    cr cr ." No solution exists"
  then

  MS-TICKS swap - \ ADDED code JL
  cr ." Time used in milliseconds: " .  ."  Max DEPTH: " MAXDepth .  
  ."  xAround: " TimesAround . cr \ ADDED code JL
;

\ game

\ EOF		"Test-area":

decimal

: 81>board	( n...81......n --- )
	81 0
	do 80 I -  \ 80.......0
	    cells 	\  n --- 4n
	    puzzle  + !
	loop
;

decimal

\ "World's hardest SUDOKU":
8 0 0 0 0 0 0 0 0
0 0 3 6 0 0 0 0 0
0 7 0 0 9 0 2 0 0
0 5 0 0 0 7 0 0 0
0 0 0 0 4 5 7 0 0
0 0 0 1 0 0 0 3 0
0 0 1 0 0 0 0 6 8
0 0 8 5 0 0 0 1 0
0 9 0 0 0 0 4 0 0

81>board
