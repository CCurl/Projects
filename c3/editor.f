\ A simple source code editor

' editor loaded?

: rl forget s" editor.f" (load) ;

load screen.f
load memory.f

500   const max-lines
128   const max-width
 30   const scr-rows
max-lines max-width * const buf-sz

variable buf buf-sz allot
val row   (val) (row)   : >row (row) ! ;
val col   (val) (col)   : >col (col) ! ;  : col++ (col) ++ ;
val pos   (val) (pos)   : >pos (pos) ! ;
val refresh?   (val) (rfr)
: refresh 1 (rfr) ! ;
: refreshed 0 (rfr) ! ;

val top     (val) (top) 
val bottom  (val) (bottom)
: >bottom (bottom) ! ;
: >top (top) ! top scr-rows + >bottom ;

: clear-buf ( -- )    buf buf-sz 0 fill ;
: line-addr ( n--a )  max-width * buf + ;
: row->addr ( --a )   row top +   line-addr ;
: rc->pos   ( -- )    row->addr   col +   >pos ;
: ->cur     ( -- )    col 1+   row 1+   ->yx ;

: do-refresh ( -- )
    refresh? if
        cur-off 1 1 ->yx
        bottom top do
            i line-addr typez clr-eol cr
        loop
        cur-on
        refreshed
    then ;

: exit?      ( k--f )   27 = ;
: scr-up?    ( k--f )  'W' = ;
: scr-down?  ( k--f )  'S' = ;
: up?        ( k--f )  'w' = ;
: down?      ( k--f )  's' = ;
: left?      ( k--f )  'a' = ;
: right?     ( k--f )  'd' = ;
: del-ch?    ( k--f )   24 = ;
: rep-ch?    ( k--f )  'r' = ;

: left  ( -- )  col 1- 0         max >col ;
: right ( -- )  col 1+ max-width min >col ;
: up    ( -- )  row 1- 0         max >row ;
: down  ( -- )  row 1+ scr-rows  min >row ;

: scroll-up    top 1- 0 max >top  down  refresh ;
: scroll-down  top 1+ >top        up    refresh ;

: delete-num ( num-- )  drop ;
: insert-num ( num-- )  drop ;
: insert-ch  ( ch-- )   1 insert-num  pos c!   col++   refresh ;
: replace-ch ( -- )     key   pos c!   col++   refresh ;
: delete-ch  ( -- )     1 delete-num   refresh ;

: handle-ch ( -- )
    r1 up?    if up    exit then
    r1 down?  if down  exit then
    r1 scr-up?    if scroll-up    exit then
    r1 scr-down?  if scroll-down  exit then
    r1 left?  if left  exit then
    r1 right? if right exit then
    r1 del-ch? if delete-ch  exit then
    r1 rep-ch? if replace-ch exit then
    31 r1 < r1 127 < and if r1 insert-ch exit then
    r1 . ;

: edit-loop ( -- )
    begin
        do-refresh   ->cur   rc->pos   key s1
        r1 exit? if exit then
        handle-ch
    again ;

: init   0 >top   0 dup >row >col   refresh ;

: read-line ( a fh--eof )
    +regs s2 s1 0 s4
    begin 
        r1 1 r2 fread 0= if 1 -regs exit then
        r1 c@ s3
        r3 10 = if  0 r1 c! 0 -regs exit then
        r3  9 = if 32 r1 c! then
        r3 13 <> if i1 i4 then
    again ;

: load-file ( fh-- )
    +regs s1 0 s2
    begin
        r2 i2 line-addr r1 read-line
    until
    r1 fclose ;

: read-file ( sz-- )
    clear-buf   fopen-rt   ?dup
    if load-file else ." -file not found-" then ;

: edit ( -- )
    init next-word drop 1- read-file 
    cls edit-loop ;
