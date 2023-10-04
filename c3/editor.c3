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

variable done 0 done !
variable fn 32 allot
variable cmd 32 allot
variable msg 32 allot
variable clip-board line-len allot

variable (scr-top)   : scr-top (scr-top) @ ;   : >scr-top (scr-top) ! ;    0 >scr-top
variable (scr-h)     : scr-h   (scr-h)   @ ;   : >scr-h   (scr-h)   ! ;   45 >scr-h
variable (scr-w)     : scr-w   (scr-w)   @ ;   : >scr-w   (scr-w)   ! ;  100 >scr-w
variable (row)       : row     (row)     @ ;   : >row     (row)     ! ;
variable (col)       : col     (col)     @ ;   : >col     (col)     ! ;   : col++ (col) ++ ;
variable (mode)      : mode    (mode)    @ ;   : >mode    (mode)    ! ;
variable (#lines)    : #lines  (#lines)  @ ;   : >#lines  (#lines)  ! ;
variable (#bytes)    : #bytes  (#bytes)  @ ;   : >#bytes  (#bytes)  ! ;

: line-addr  ( n--a ) 0 max max-lines min line-len * buf + ;

: laptop 32 >scr-h ;
: pc     47 >scr-h ;

variable (rfr)
: refresh? (rfr) @ ;
: refresh   1 (rfr) ! ;
: refreshed 0 (rfr) ! ;

: clr-buf    ( -- )     buf buf-sz 0 fill 0 >#lines ;
: clr-line   ( a-- )    line-len 0 fill ;
: clr-clip   ( -- )     clip-board clr-line ;
: cur-line   ( --a )    scr-top row + line-addr ;
: pos        ( --a )    cur-line col + ;
: ->cur      ( -- )     col 1+   row 1+   ->xy ;
: ->cmd      ( -- )     1 scr-h 1+ ->xy ;
: reset      ( -- )     clr-buf 0 >row 0 >col 0 >scr-top ;
: >msg       ( s-- )    msg swap s-cpy ;
: clr-msg    ( -- )     msg S-TRUNC ;
: .mode      ( -- )
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
	show-footer .s cur-on ->cur refreshed ;

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

: insert-line ( -- ) +REGS
    cur-line      s1    \ src
    r1 line-len + s2    \ dst
    buf-end r2 -  s3    \ sz
    r1 r2 r3 cmove>  -REGS
    cur-line clr-line ;

: delete-line ( -- ) +REGS
	clr-clip clip-board cur-line S-CPY
    cur-line       s2    \ dst
    r2 line-len +  s1    \ src
    buf-end r1 -   s3    \ sz
    r1 r2 r3 cmove  -REGS ;

: left   col 1- 0          max >col   ->cur ;
: right  col 1+ scr-w      min >col   ->cur ;
: up     row 1- 0          max >row   ->cur ;
: down   row 1+ scr-h      min >row   ->cur ;
: home   0 >col                       ->cur ;
: end    row find-eol >col ->cur ;

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
: repl-md   md-replace >mode cmd-clr  .mode ->cur ;
: ins-md    md-insert  >mode cmd-clr  .mode ->cur ;
: cmd-md    md-command >mode cmd-clr  ':' emit ;
: norm-md   md-normal  >mode cmd-clr  ->cur ;

: replace-ch ( ch-- )  pos c!   col++   refresh ;
: insert-ch  ( ch-- )  insert-1   replace-ch ;
: delete-ch  ( -- )    delete-1   refresh ;
: ins-line-a ( -- ) insert-line refresh ;
: ins-line-b ( -- ) row 1+ >row insert-line refresh ;
: del-line ( -- ) delete-line refresh ;
: yank ( -- ) clr-clip clip-board cur-line S-CPY refresh ;
: put-line ( -- )
    cur-line clr-line
    cur-line clip-board S-CPY refresh ;

: command-ch ( -- )    r1 emit md-normal >mode
    'L' r1 = IF fn read-file  EXIT THEN
    'W' r1 = IF fn write-file EXIT THEN
    'Q' r1 = IF 1 done !      EXIT THEN
    ;

: replace-mode? ( --f )   mode md-replace = ;
: insert-mode?  ( --f )   mode md-insert  = ;
: command-mode? ( --f )   mode md-command = ;
: normal-mode?  ( --f )   mode md-normal  = ;

: handle-ch ( ch-- )  s1
    r1 27 =       IF norm-md       EXIT THEN
    insert-mode?  IF r1 insert-ch  EXIT THEN
    replace-mode? IF r1 replace-ch EXIT THEN
    command-mode? IF command-ch    EXIT THEN
    r1 'k' =      IF up            EXIT THEN
    r1 'j' =      IF down          EXIT THEN
    r1 'h' =      IF left          EXIT THEN
    r1 'l' =      IF right         EXIT THEN
    r1 '_' =      IF home          EXIT THEN
    r1 '$' =      IF end           EXIT THEN
    r1 'x' =      IF delete-ch     EXIT THEN
    r1 'o' =      IF ins-line-b    EXIT THEN
    r1 'O' =      IF ins-line-a    EXIT THEN
    r1 'p' =      IF put-line      EXIT THEN
    r1 'D' =      IF del-line      EXIT THEN
    r1 'R' =      IF repl-md       EXIT THEN
    r1 'i' =      IF ins-md        EXIT THEN
    r1 'Y' =      IF yank          EXIT THEN
    r1 ':' =      IF cmd-md        EXIT THEN
    ->cmd r1 . ->cur ;

: edit-loop ( -- )
    BEGIN
        done @ IF CR CR EXIT THEN
        ?refresh
        key handle-ch
    AGAIN ;

: init ( -- )
    md-normal >mode
    clip-board s-trunc
    0 DUP >row DUP >col done !
    refresh ;

: edit ( -- )
	init cls
    next-word DROP read-file
    edit-loop ;

laptop edit editor.c3
