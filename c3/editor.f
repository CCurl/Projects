\ A simple source code editor

' edit loaded?

: rl forget s" editor.f" (load) ;

load screen.f
load memory.f

500   const max-lines
128   const max-width
 30   const scr-rows

111 const md-replace
112 const md-insert

max-lines max-width * const buf-sz
variable buf buf-sz allot

val row      (val) (row)   : >row (row) ! ;
val col      (val) (col)   : >col (col) ! ;  : col++ (col) ++ ;
val pos      (val) (pos)   : >pos (pos) ! ;
val mode     (val) (mode)  : >mode (mode) ! ;
val refresh? (val) (rfr)

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
        bottom 1+ top do
            i line-addr typez clr-eol cr
        loop
        cur-on
        refreshed
    then ;

: esc?        27 = ;
: exit?      'Q' = ;
: up?        'w' = ;
: down?      's' = ;
: left?      'a' = ;
: right?     'd' = ;
: home?      '^' = ;
: end?       '$' = ;
: del-ch?    'x' = ;
: repl-md?   'R' = ;
: ins-md?    'I' = ;
: scr-up?    'W' = ;
: scr-down?  'S' = ;
: pg-up?     'E' = ;
: pg-dn?     'C' = ;

: find-end ( --a )
    +regs pos s1
    begin r1 c@ if i1 1 else 0 then while
    begin r1 c@ if 0 else d1 1 then while
    i1 -regs ;

: left   col 1- 0         max >col ;
: right  col 1+ max-width min >col ;
: up     row 1- 0         max >row ;
: down   row 1+ scr-rows  min >row ;
: home   0 >col ;
: end    find-end pos -   col +   >col ;
: repl-md   md-replace >mode ;
: ins-md    md-insert  >mode ;

: scroll-up    top 1- 0 max >top  down  refresh ;
: scroll-down  top 1+ >top        up    refresh ;
: pg-up  top 8 - 0 max >top  refresh ;
: pg-dn  top 8 +       >top  refresh ;

: delete-num ( num-- )
    r1 >r 0 do
        pos s1
        begin r1 1+ c@   r1 c!   r1 c@   i1 while
    loop r> s1 ;
: insert-num ( num-- )
    +regs s1 find-end s2
    0 do 
        r2 s3 begin
            r3 1- c@ r3 c!   d3   r3 pos >
        while i2
    loop -regs ;
: replace-ch ( ch-- )   pos c!   col++   refresh ;
: insert-ch  ( ch-- )   1 insert-num   replace-ch ;
: delete-ch  ( -- )     1 delete-num   refresh ;

: printable?    ( ch--f )  dup >r 31 > r> 127 < and ;
: replace-mode? ( --f )    mode md-replace = ;
: insert-mode? ( --f )     mode md-insert = ;

: handle-ch ( -- )
    r1 printable? if
        replace-mode?  if r1 replace-ch exit then
        insert-mode?   if r1 insert-ch  exit then
    then
    r1 up?        if up            exit then
    r1 down?      if down          exit then
    r1 left?      if left          exit then
    r1 right?     if right         exit then
    r1 home?      if home          exit then
    r1 end?       if end           exit then
    r1 del-ch?    if delete-ch     exit then
    r1 repl-md?   if repl-md       exit then
    r1 ins-md?    if ins-md        exit then
    r1 pg-up?     if pg-up         exit then
    r1 pg-dn?     if pg-dn         exit then
    r1 scr-up?    if scroll-up     exit then
    r1 scr-down?  if scroll-down   exit then
    r1 esc?       if 0 >mode       exit then
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
