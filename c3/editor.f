\ A simple source code editor

' edit loaded?

: rl forget s" editor.f" (load) ;

load file.f
load string.f
load screen.f
load memory.f

500   const max-lines
128   const max-width
 30   const scr-rows

  0 const md-normal
111 const md-command
112 const md-replace
113 const md-insert

max-lines max-width * const buf-sz
variable buf buf-sz allot
variable done
variable fn 32 allot
variable cmd 32 allot

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

: find-eof ( --a )
    buf buf-sz +
    begin 1- dup c@ until
    1+ buf max ;

: find-eol ( --a )
    +regs pos s1
    begin r1 c@ if 0 else d1 1 then while
    begin r1 c@ if i1 1 else 0 then while
    r1 -regs ;

: load-file ( fh-- )
    >r
    max-lines 0 do
        i line-addr r@ file-gets
        nip if max-lines (i) ! then
    loop
    r> fclose ;

: read-file ( fn-- )
    clear-buf   fopen-rt   ?dup
    if load-file else ." -file not found-" then ;

: write-file ( fn-- )
    fopen-wt dup 0= if drop ." -fail-" exit then
    +regs s1  find-eof s9  r1 (output_fp) !
    max-lines 0 do
        i line-addr
        dup r9 < if typez #10 emit
        else drop max-lines (i) !
        then
    loop
    r1 fclose   ->stdout
    -regs ;



: do-refresh ( -- )
    refresh? if
        cur-off 1 1 ->yx
        bottom 1+ top do
            i line-addr typez clr-eol cr
        loop
        cur-on ->cur
        refreshed
    then ;

: esc?        27 = ;
: exit?      done @ ;
: up?        'w' = ;
: down?      's' = ;
: left?      'a' = ;
: right?     'd' = ;
: home?      'A' = ;
: end?       'D' = ;
: del-ch?    'x' = ;
: repl-md?   'R' = ;
: ins-md?    'i' = ;
: cmd-md?    ':' = ;
: scr-up?    'W' = ;
: scr-down?  'S' = ;
: pg-up?     'E' = ;
: pg-dn?     'C' = ;

: left   col 1- 0         max >col   ->cur ;
: right  col 1+ max-width min >col   ->cur ;
: up     row 1- 0         max >row   ->cur ;
: down   row 1+ scr-rows  min >row   ->cur ;
: home   0 >col                      ->cur ;
: end    find-eol pos -  col +  >col ->cur ;

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
    +regs   find-eol s2
    0 do 
        r2 s3 begin
            r3 1- c@ r3 c!   d3   r3 pos >
        while i2
    loop -regs ;

: repl-md   md-replace >mode ;
: ins-md    md-insert  >mode ;
: cmd-md    md-command >mode  scr-rows 2+ 1 ->xy ':' emit ;
: cmd-clr   scr-rows 2+ 1 ->xy   clr-eol   ->cur ;
: norm-md   md-normal >mode   cmd-clr ;

: replace-ch ( ch-- )  pos c!   col++   refresh ;
: insert-ch  ( ch-- )  1 insert-num   replace-ch ;
: delete-ch  ( -- )    1 delete-num   refresh ;
: command-ch ( -- )    r1 emit
    'W' r1 = if fn write-file exit then
    'Q' r1 = if 1 done ! exit then ;

: printable?    ( ch--f )  dup >r 31 > r> 127 < and ;

: replace-mode? ( --f )   mode md-replace = ;
: insert-mode?  ( --f )   mode md-insert  = ;
: command-mode? ( --f )   mode md-command = ;
: normal-mode?  ( --f )   mode md-normal  = ;

: handle-ch ( ch-- )  s1
    r1 esc?       if norm-md       exit then
    insert-mode?  if r1 insert-ch  exit then
    replace-mode? if r1 replace-ch exit then
    command-mode? if command-ch    exit then
    r1 up?        if up            exit then
    r1 down?      if down          exit then
    r1 left?      if left          exit then
    r1 right?     if right         exit then
    r1 home?      if home          exit then
    r1 end?       if end           exit then
    r1 del-ch?    if delete-ch     exit then
    r1 repl-md?   if repl-md       exit then
    r1 ins-md?    if ins-md        exit then
    r1 cmd-md?    if cmd-md        exit then
    r1 pg-up?     if pg-up         exit then
    r1 pg-dn?     if pg-dn         exit then
    r1 scr-up?    if scroll-up     exit then
    r1 scr-down?  if scroll-down   exit then
    r1 . ;

: edit-loop ( -- )
    begin
        exit? if exit then
        do-refresh   rc->pos   key
        handle-ch
    again ;

: init ( -- )
    md-normal >mode
    0 dup done !
    dup >top
    dup >row >col
    refresh ;

: edit ( -- )
    init next-word drop
    fn swap s-from-sz
    fn read-file
    cls edit-loop ;

edit xxx.f
