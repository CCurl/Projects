\ A simple source code editor

' editor loaded?

: rl forget s" editor.f" (load) ;

load screen.f
load memory.f

500   const max-lines
100   const max-width
 30   const scr-rows
max-lines max-width * const buf-sz

variable lines 500 cells allot
variable buf buf-sz allot

val buf-end (val) (buf-end)
: >buf-end (buf-end) ! ;

val pos (val) (pos)
: >pos (pos) ! ;

val file-sz ( --n )   (val) (file-sz)
: >file-sz ( n-- )    (file-sz) !   buf file-sz + >buf-end ;

val bot (val) (bot)
: >bot (bot) ! ;

val top (val) (top) 
: >top (top) ! top scr-rows + >bot ;

: clear-buf ( -- )  buf buf-sz 0 fill ;
: clear-lines ( -- )  lines max-lines cells 0 fill ;

: get-line ( n--a )  cells lines + @ ;
: set-line ( a-- )   r2 cells lines + ! i2 ;

: buf->lines ( -- )
    +regs buf s1
    clear-lines 0 s2 buf set-line
    buf-sz 0 do
        r1 c@ s3 i1
        r3 0= if i >file-sz buf-sz (i) ! then
        r3 10 = if r1 set-line then
    loop -regs ;

: type-line ( a-- ) dup c@ dup 32 < if 2drop exit then emit 1+ type-line ;

: show-screen ( -- )
    1 1 ->xy
    bot top do
        i get-line ?dup if type-line then clr-eol cr
    loop ;

: init clear-buf clear-lines 0 >top buf >pos ;

: read-file  ( fn--sz )
    fopen-r s1 
    r1 if 
        buf buf-sz r1 fread
        r1 fclose
    else
        ." file not found." 0
    then ;

: delete-num ( num-- )
    +regs s9 pos s2 r2 r9 + s1
    begin
        r1 c@ r2 c! i1 i2 r1 buf-end <
    while 
    buf-end r9 0 fill
    buf-end r9 + >buf-end 
    -regs ;

: insert-num ( num-- )
    +regs s9
    buf-end r9 + >buf-end   buf-end s2   r2 r9 - s1
    begin
        r1 c@ r2 c!   d1 d2   r2 pos >=
    while 
    -regs ;

: insert-ch ( ch-- )  1 insert-num  pos c!  buf->lines ;
: delete-ch ( -- )    1 delete-num          buf->lines ;

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
    31 r1 < r1 127 < and if r1 insert-ch then  ;

: edit-loop ( -- )
    begin
        show-screen
        key s1 r1 exit? if exit then
        handle-ch
    again ;

: edit ( -- )
    init next-word drop 1- read-file >file-sz
    file-sz 0= if exit then
    buf->lines cls edit-loop ;
