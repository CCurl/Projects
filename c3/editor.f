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

val bottom
(val) (bottom)
: >bottom (bottom) ! ;

val top
(val) (top) 
: >top (top) ! top scr-rows + >bottom ;

: clear-buf ( -- )  buf buf-sz 0 fill ;
: line-addr ( n--a )  max-width * buf + ;
: rc->pos ( -- )   row top +   line-addr   col +   >pos ;
: ->cur   ( -- )   col 1+   row 1+   ->yx ;

: show-screen ( -- )
    cur-off 1 1 ->yx
    bottom top do
        i line-addr typez clr-eol cr
    loop
    cur-on   ->cur ;

: delete-num ( num-- ) drop ;
: insert-num ( num-- ) drop ;
: insert-ch  ( ch-- )  1 insert-num  pos c!   col++ ;
: replace-ch ( ch-- )  pos c!   col++ ;
: delete-ch  ( -- )    1 delete-num ;

: exit?   ( k--f ) 27 = ;
: key-up? ( k--f ) 'w' = ;
: key-dn? ( k--f ) 's' = ;
: del-ch? ( k--f ) 24 = ;

: scroll-up top 1- 0 max >top ;
: scroll-dn top 1+ >top ;

: handle-ch ( -- )
    r1 key-up? if scroll-up exit then
    r1 key-dn? if scroll-dn exit then
    r1 del-ch? if delete-ch exit then
    31 r1 < r1 127 < and if r1 insert-ch exit then
    r1 . ;

: edit-loop ( -- )
    begin
        show-screen
        rc->pos
        key s1 r1 exit? if exit then
        handle-ch
    again ;

: init 0 >top   0 dup >row >col ;

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
        r2 line-addr r1 read-line   i2
    until
    r1 fclose ;

: read-file ( sz-- )
    clear-buf fopen-rt ?dup 
    if load-file else ." -file not found-" then ;

: edit ( -- )
    init next-word drop 1- read-file 
    cls edit-loop ;
