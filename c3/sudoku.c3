\ SUDOKU solver

\ Some useful constants

3 constant box-size
box-size dup * constant puzzle-size

variable puzzle puzzle-size dup * allot

\ Words to map row/col to puzzle addresses
\ NOTE: row and col numbers go from zero to (puzzle-size-1)
\ row 0, col 0 is the puzzle start address
\ row 1, col 0 is the puzzle start address + puzzle-size

: offset>puzzle   ( offset--a )         puzzle + ;
: row>offset      ( row--n )            puzzle-size * ;
: r/c>offset      ( row col--n )        swap row>offset +  ;
: r/c>puzzle      ( row col--a )        r/c>offset offset>puzzle ; 
: next-row        ( a1--a2 )            puzzle-size + ;
: num-at          ( row col--num )      r/c>puzzle c@ ;
: num-to          ( num row col-- )     r/c>puzzle c! ;

\ in-row? ( num row -- f ) - f=1 means that num is in the row
: in-row? +regs s2 s1 0 s8
    r2 0 r/c>puzzle s2
    puzzle-size 0 do
        r2+ c@ r1 = if i8 99 (i) ! then
    loop r8 -regs ;

\ in-column? ( num cow -- f ) - f=1 means that num is in the column
: in-column?  +regs s2 s1 0 s8
    0 r2 r/c>puzzle s2
    puzzle-size 0 do
        r2 c@ r1 = 
        if i8 99 (i) !
        else r2 next-row s2
        then
    loop r8 -regs ;

\ in-box? ( num row col -- f ) - f=1 means num num is in the box starting at (row column)
: in-box?  +regs s3 s2 s1 0 s8
    r2 r3 r/c>puzzle s4
    box-size 0 do
        r4 s5
        box-size 0 do 
            r5+ c@ r1 = if 99 (i) ! i8 then 
        loop
        r8 if 99 (i) ! 
        else r4 puzzle-size + s4
        then
    loop r8 -regs ;

\ can-assign? ( row col num -- f ) - f=1 means the num can be assigned to (row,col)
: can-assign?  +regs s3 s2 s1
    r3 r1    in-row?     if -regs 0 then
    r3 r2    in-column?  if -regs 0 then
    r3 r1 r2 in-box?     if -regs 0 then
    -regs 1 ;

\ Function: solve?
\ ----------------
\ Takes a partially filled-in puzzle and attempts to assign values to all
\ unassigned locations in such a way to meet the requirements for sudoku
\ solution (non-duplication across rows, columns, and boxes). The function
\ operates via recursive backtracking: it finds an unassigned location with
\ the puzzle and then considers all digits from 1 to "puzzle-size" in a loop. If a digit
\ is found that has no existing conflicts, tentatively assign it and recur
\ to attempt to fill in rest of puzzle. If this was successful, the puzzle is
\ solved. If not, unmake that decision and try again. If all digits have
\ been examined and none worked out, return false to backtrack to previous
\ decision point.

\ : solve? ( n -- f )
\ 
\    DEPTH MAXDepth max is MAXDepth  \ ADDED code JL
\    TimesAround 1+ is TimesAround 	 \ ADDED code JL
\ 
\    dup cell-count = 
\    if drop -1 exit 
\    then            \ success!
\ 
\    dup puzzle@ 
\    if 1+ recurse exit 
\    then                 \ if it's already assigned, skip
\ 
\    10 1 
\    ?do                  \ consider all digits
\      i over available? 
\      if                    	\ if looks promising
\        i over puzzle!     \ make tentative assignment
\        dup 1+ recurse 
\        if unloop drop -1 exit \ recur, if success, yay!
\        then
\      then 
\    loop
\    unassigned over puzzle!  \ failure, unmake & try again
\    drop 0                       \ this triggers backtracking
\ ;
\ 
\ \ puzzle display
\ 
\ : .puzzle-element ( n -- )
\    ."  "
\    puzzle@ dup unassigned? if drop ." - " else .  then
\    ."  " ;
\ 
\ : .box-break-vertical ( -- )
\    ." |" ;
\ 
\ : .box-break-horizontal ( -- )
\    ." ------------+------------+-----------" ;
\ 
\ : .puzzle ( -- )
\    cell-count 0 
\   do
\        i move>x unassigned? 
\        if   i move>y box-size mod 0= 
\            if  cr cr
\               .box-break-horizontal
\            then  cr cr
\      else
\          i move>x box-size mod 0= 
\          if   .box-break-vertical
\          then
\      then
\      i .puzzle-element
\    loop
\    cr cr .box-break-horizontal cr 
\ ;
\ 
\ : game ( -- )
\ 
\   MS-TICKS 	\ ADDED code JL
\   0 is TimesAround  \ ADDED code JL
\   0 is MAXDepth	\ ADDED code JL
\ 
\   0 solve? if
\     .puzzle
\   else
\     cr cr ." No solution exists"
\   then
\ 
\   MS-TICKS swap - \ ADDED code JL
\   cr ." Time used in milliseconds: " .  ."  Max DEPTH: " MAXDepth .  
\   ."  xAround: " TimesAround . cr \ ADDED code JL
\ ;
\ 
\ \ game
\ 
\ \ EOF		"Test-area":
\ 
\ decimal

: .puzzle
    puzzle-size 0 do i s1
        puzzle-size 0 do r1 i num-at .
        loop cr
    loop ;

: >puzzle-row  ( n1 .. n9 row -- )
    1- 0 r/c>puzzle s1
    9 0 do >r loop
    9 0 do r> r1+ c! loop ;

decimal

\ The "World's hardest SUDOKU" ...
\ http://www.telegraph.co.uk/news/science/science-news/9359579/Worlds-hardest-sudoku-can-you-crack-it.html

8 0 0 0 0 0 0 0 0   1  >puzzle-row
0 0 3 6 0 0 0 0 0   2  >puzzle-row
0 7 0 0 9 0 2 0 0   3  >puzzle-row
0 5 0 0 0 7 0 0 0   4  >puzzle-row
0 0 0 0 4 5 7 0 0   5  >puzzle-row
0 0 0 1 0 0 0 3 0   6  >puzzle-row
0 0 1 0 0 0 0 6 8   7  >puzzle-row
0 0 8 5 0 0 0 1 0   8  >puzzle-row
0 9 0 0 0 0 4 0 0   9  >puzzle-row

\ : reload forget s" sudoku.c3" (load) ;
