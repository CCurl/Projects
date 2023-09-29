\ A simple source code editor

' edit loaded?

: rl forget " editor.c3" (load) ;

load file.c3
load string.c3
load screen.c3
load memory.c3

    0 constant md-normal
  111 constant md-command
  112 constant md-replace
  113 constant md-insert
50000 constant buf-sz
 1000 constant max-lines

variable buf buf-sz allot
: buf-end buf buf-sz + ;

\ lines is an array of addresses ... where each line starts
variable lines max-lines allot
: last-line max-lines 1- ;
: >lines (lines) ! ;

variable done 0 done !
variable fn 32 allot
variable cmd 32 allot
variable msg 32 allot
variable clip-board 128 allot

variable (scr-top)   : scr-top (scr-top) @ ;   : >scr-top (scr-top) ! ;
variable (scr-h)     : scr-h   (scr-h)   @ ;   : >scr-h   (scr-h) ! ;
variable (row)       : row     (row)     @ ;   : >row     (row)     ! ;
variable (col)       : col     (col)     @ ;   : >col     (col)     ! ;   : col++ (col) ++ ;
variable (pos)       : pos     (pos)     @ ;   : >pos     (pos)     ! ;
variable (mode)      : mode    (mode)    @ ;   : >mode    (mode)    ! ;
variable (#lines)    : #lines  (#lines)  @ ;   : >#lines  (#lines)  ! ;

variable (rfr)
: refresh? (rfr) @ ;
: refresh   1 (rfr) ! ;
: refreshed 0 (rfr) ! ;

: clr-buf   ( -- )    buf buf-sz 0 fill ;
: line-addr ( n--a )  lines + @ ;
: row-addr  ( --a )   scr-top row + line-addr ;
: rc->pos   ( -- )    row-addr   col +   >pos ;
: ->cur     ( -- )    col 1+   row 1+   ->yx ;
: reset     ( -- )    clr-buf 0 >row 0 >col 0 >scr-top ;
: >msg      ( s-- )   msg swap s-cpy ;
: clr-msg   ( -- )    >msg s-trunc ;
: clr-line  ( n--)    line-addr line-len 0 fill ;

: find-eol ( line--a )
    +regs line-addr dup s1 s2
    line-len 0 do 
		r1 c@ BL > if r1 s2 then i1
	loop
    r2 -regs ;

: insert-line ( line-- ) >r
	+regs r@ line-addr s1  \ src
	r@ 1+ line-addr s2     \ dst
	buf-end r2 - s3        \ sz
	r1 r2 r3 -regs cmove> r> 1+ clr-line ;

: delete-line ( line-- )
    +regs dup line-addr s2 \ dst
    1- line-addr s1        \ src
	buf-end r1 - s3        \ sz
    .s clip-board r2 line-len cmove
	r1 r2 r3 -regs .s cmove ;

: load-file ( fh-- )
    >r buf buf-sz r> fread drop
	buf-sz buf + buf do i c@ 32 < if 32 i c! then loop
    clr-msg ;

: read-file ( fn-- )
    clr-buf   fopen-rb   ?dup
	0= if " -not found-" >msg exit then
    dup load-file fclose ;

: write-file ( fn-- )
    fopen-wb ?dup 0= if " -write fail-" >msg exit then
    buf qtype
    r1 fclose   ->stdout
    " -saved-" >msg ;

: show-footer ( -- )
    cr cr ." cb: [" clip-board 6 type ." ...], "
    fn qtype space msg qtype clr-eol ;

: ?refresh ( -- )
    refresh? 0= if exit then
	cur-off 1 1 ->xy
	scr-h 0 do
        i str-top + line-addr qtype clr-eol cr
	loop
	show-footer
	cur-on ->cur
	refreshed ;

: left   col 1- 0          max >col   ->cur ;
: right  col 1+ line-len   min >col   ->cur ;
: up     row 1- 0          max >row   ->cur ;
: down   row 1+ last-line  min >row   ->cur ;
: home   0 >col                       ->cur ;
: end    row find-eol row-addr - >col ->cur ;

: esc?        27 = ;
: exit?      done @ ;
: up?        'k' = ; INLINE
: down?      'j' = ; INLINE
: left?      'h' = ; INLINE
: right?     'l' = ; INLINE
: home?      '_' = ; INLINE
: end?       '$' = ; INLINE
: del-ch?    'x' = ; INLINE
: del-ln?    'D' = ; INLINE
: ins-ln?    'o' = ; INLINE
: put-ln?    'P' = ; INLINE
: repl-md?   'R' = ; INLINE
: ins-md?    'i' = ; INLINE
: cmd-md?    ':' = ; INLINE
: yank?      'Y' = ; INLINE


: delete-1 ( -- ) 
    +regs pos s1 buf-end s2
    begin
		r1 r2 < while
		r1 1+ c@ r1+ c!
    repeat BL r1 c! -regs ;

: insert-1 ( -- )
    +regs pos s1 buf-end s2
    begin
		r2 r1 > while
		r2 1- c@ r2- c!
    repeat BL r2 c! -regs ;

: repl-md   md-replace >mode ;
: ins-md    md-insert  >mode ;
: cmd-md    md-command >mode  lines 2+ 1 ->xy ':' emit ;
: cmd-clr   1 lines 2+ ->xy   clr-eol   ->cur ;
: norm-md   md-normal >mode   cmd-clr ;

: replace-ch ( ch-- )  pos c!   col++   refresh ;
: insert-ch  ( ch-- )  insert-1   replace-ch ;
: delete-ch  ( -- )    delete-1   refresh ;
: ins-line ( -- ) row insert-line refresh ;
: del-line ( -- ) row delete-line refresh ;
: yank ( -- ) row-addr clip-board line-len cmove refresh ;
: put-line ( -- )
    row 1+ insert-line row 1+ >row
    clip-board row-addr line-len cmove refresh ;

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
    r1 ins-ln?    if ins-line      exit then
    r1 put-ln?    if put-line      exit then
    r1 del-ln?    if del-line      exit then
    r1 repl-md?   if repl-md       exit then
    r1 ins-md?    if ins-md        exit then
    r1 cmd-md?    if cmd-md        exit then
    r1 yank?      if yank          exit then
    r1 . ;

: edit-loop ( -- )
    begin
        exit? if exit then
        ?refresh   rc->pos
        key handle-ch
    again ;

: init ( -- )
    md-normal >mode
    clip-board s-trunc
    0 dup dup >row >col done !
    refresh ;

: edit ( -- )
	init
    fn next-word drop s-from-sz
    fn read-file
    cls edit-loop ;

\ edit editor.c3
