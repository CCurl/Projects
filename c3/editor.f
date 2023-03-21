\ A simple source code editor

' editor loaded?

: rl forget s" editor.f" (load) ;

load screen.f
load memory.f

32 1024 * const buf-sz
500       const max-lines
 30       const num-rows

variable buf buf-sz allot
variable lines 500 cells allot
val top (val) (top) 
: >top (top) ! ;

: clear-buf ( -- )  buf buf-sz 0 fill ;
: clear-lines ( -- )  lines max-lines cells 0 fill ;

: get-line ( n--a )  cells lines + @ ;
: set-line ( a n-- ) cells lines + ! ;

: buf->lines ( -- )
    +regs buf s1 0 s2 clear-lines
    buf r2 set-line
    buf-sz 0 do
        r1 c@ s3 
        r3 0= if buf-sz (i) ! then
        r3 10 = if i2 r1 1+ r2 set-line then
        i1
    loop -regs ;

: type-line ( a-- ) dup c@ dup 32 < if 2drop exit then emit 1+ type-line ;

: show-screen ( -- )
    1 1 ->xy
    num-rows 0 do
        i top + get-line type-line clr-eol cr
    loop ;

: init clear-buf clear-lines 0 >top ;
: read-file  fopen-r s1 
    r1 if 
        buf buf-sz r1 fread . ." chars read."
        r1 fclose
    else
        ." file not found."
    then ;

init
s" editor.f" read-file
buf->lines
cls show-screen
