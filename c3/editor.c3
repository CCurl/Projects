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
  100 constant line-len
 1000 constant max-lines
max-lines 1+ line-len * constant buf-sz

variable buf buf-sz allot
: buf-end buf buf-sz + ;

\ lines is an array of addresses ... where each line starts
variable lines max-lines CELLS allot

variable done 0 done !
variable fn 32 allot
variable cmd 32 allot
variable msg 32 allot
variable clip-board 128 allot

variable (scr-top)   : scr-top (scr-top) @ ;   : >scr-top (scr-top) ! ;    0 >scr-top
variable (scr-h)     : scr-h   (scr-h)   @ ;   : >scr-h   (scr-h)   ! ;   45 >scr-h
variable (scr-w)     : scr-w   (scr-w)   @ ;   : >scr-w   (scr-w)   ! ;  100 >scr-w
variable (row)       : row     (row)     @ ;   : >row     (row)     ! ;
variable (col)       : col     (col)     @ ;   : >col     (col)     ! ;   : col++ (col) ++ ;
variable (mode)      : mode    (mode)    @ ;   : >mode    (mode)    ! ;
variable (#lines)    : #lines  (#lines)  @ ;   : >#lines  (#lines)  ! ;
variable (#bytes)    : #bytes  (#bytes)  @ ;   : >#bytes  (#bytes)  ! ;

: last-line max-lines 1- ;
: T0 ( n--a ) 0 max max-lines min line-len * buf + ;
: line-addr  ( n--a )   T0 ;

: laptop 33 >scr-h ;
: pc     45 >scr-h ;

variable (rfr)
: refresh? (rfr) @ ;
: refresh   1 (rfr) ! ;
: refreshed 0 (rfr) ! ;

: clr-buf    ( -- )     buf buf-sz 0 fill 0 >#lines ;
: cur-line   ( --a )    scr-top row + line-addr ;
: pos        ( --a )    cur-line col + ;
: ->cur      ( -- )     col 1+   row 1+   ->xy ;
: ->cmd      ( -- )     1 scr-h 1+ ->xy ;
: reset      ( -- )     clr-buf 0 >row 0 >col 0 >scr-top ;
: >msg       ( s-- )    msg swap s-cpy ;
: clr-msg    ( -- )     msg S-TRUNC ;
: .mode mode
	mode md-command = IF ':' EMIT      THEN 
	mode md-insert  = IF ." -INSERT-"  THEN 
	mode md-replace = IF ." -REPLACE-" THEN ;

: show-footer ( -- )
    ->cmd CR ." cb: [" clip-board 6 type ." ...], "
    fn QTYPE space msg QTYPE clr-eol ;

: ?refresh ( -- )
    refresh? NOT IF EXIT THEN
	cur-off 1 1 ->xy
	scr-h 0 DO
        scr-top I + line-addr ?DUP IF QTYPE THEN clr-eol CR
	LOOP
	show-footer cur-on ->cur refreshed ;

: read-file ( fn-- )
	fn s-scpy   0 >#lines   0 >#bytes
    clr-buf     clr-msg     refresh
    fn fopen-rt >R R@ NOT IF
        R> DROP " -file not found-" >msg EXIT
	THEN
	BEGIN
		#lines line-addr R@ fgets 0= WHILE
		#lines line-addr S-LEN 1+ #bytes + >#bytes
		#lines 1+ >#lines
	REPEAT
	R> FCLOSE
	msg #bytes s-catn  msg "  bytes." S-CAT ;

: write-file ( fn-- )
    fopen-wb ?DUP NOT IF " -write fail-" >msg EXIT THEN
    DUP ->file
	#lines 0 DO I line-addr ?DUP IF QTYPE 10 EMIT THEN LOOP
	FCLOSE ->stdout
    " -saved-" >msg ;

: find-eol ( line#--n )
    +REGS  line-addr s1 0 s2
    BEGIN
		r1 r2 + C@ 0= IF r2 -REGS EXIT THEN i2
	AGAIN ;

: line-end ( a1--a2 ) BEGIN DUP C@ 0= IF EXIT THEN 1+ AGAIN ;

: insert-line ( line-- ) >R
    +REGS R@ line-addr s1  \ src
    R@ 1+ line-addr s2     \ dst
    buf-end r2 - s3        \ sz
    r1 r2 r3 -REGS cmove> R> ; \ 1+ clr-line ;

: delete-line ( -- )
	clip-board cur-line S-CPY
    +REGS dup line-addr s2 \ dst
    1- line-addr s1        \ src
    buf-end r1 - s3        \ sz
    \ .s clip-board r2 line-len cmove
    r1 r2 r3 -REGS .s cmove ;

: left   col 1- 0          max >col   ->cur ;
: right  col 1+ scr-w      min >col   ->cur ;
: up     row 1- 0          max >row   ->cur ;
: down   row 1+ scr-h      min >row   ->cur ;
: home   0 >col                       ->cur ;
: end    row find-eol >col ->cur ;

: esc?       27 = ;
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
    +REGS pos s1 cur-line line-end s2
    BEGIN
		r1 r2 < WHILE
		r1 1+ C@ r1+ C!
    REPEAT
	0 r1 C! -REGS ;

: insert-1 ( -- )
    +REGS pos s1 cur-line line-end s2
    begin
		r2 r1 > while
		r2 1- C@ r2- C!
    repeat
	BL r2 C! -REGS ;

: cmd-clr   ->cmd clr-eol ;
: repl-md   md-replace >mode cmd-clr .mode ->cur ;
: ins-md    md-insert  >mode cmd-clr .mode ->cur ;
: cmd-md    md-command >mode cmd-clr  ':' emit ;
: norm-md   md-normal >mode  cmd-clr ->cur ;

: replace-ch ( ch-- )  pos c!   col++   refresh ;
: insert-ch  ( ch-- )  insert-1   replace-ch ;
: delete-ch  ( -- )    delete-1   refresh ;
: ins-line ( -- ) row insert-line refresh ;
: del-line ( -- ) delete-line refresh ;
: yank ( -- ) ; \ cur-line clip-board line-len cmove refresh ;
: put-line ( -- )
    row 1+ insert-line row 1+ >row
    clip-board cur-line line-len cmove refresh ;

: command-ch ( -- )    r1 emit
    'L' r1 = if fn read-file  exit then
    'W' r1 = if fn write-file exit then
    'Q' r1 = if 1 done !      exit then ;

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
    BEGIN
        exit? IF CR CR EXIT THEN
        ?refresh
        key handle-ch
    AGAIN ;

: init ( -- )
    md-normal >mode
    clip-board s-trunc
    0 dup >row dup >col done !
    refresh ;

: edit ( -- )
	init cls
    next-word DROP read-file
    edit-loop ;

\ edit editor.c3
