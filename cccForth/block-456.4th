1 load
2 load

10000 CONSTANT src-sz
500 CONSTANT max-lines

  0 CONSTANT ST_INS
  1 CONSTANT ST_REPL
999 CONSTANT ST_EXIT

2000 CONSTANT K-up
2001 CONSTANT K-dn
2002 CONSTANT K-lf
2003 CONSTANT K-rt
2004 CONSTANT K-home
2005 CONSTANT K-end
2006 CONSTANT K-pgup
2007 CONSTANT K-pgdn
2008 CONSTANT K-copy
2010 CONSTANT K-paste
2011 CONSTANT K-del
2012 CONSTANT K-cr

variable lines max-lines CELLS ALLOT
variable (num-lines)
variable (top-line)
variable (scr-rows)
variable (scr-cols)
variable (file-sz)
variable (file-end)
variable pad 128 ALLOT
variable st 0 st !
variable src src-sz ALLOT
variable (row)
variable (col)

: num-lines (num-lines) @ ;
: top-line  (top-line) @ ;
: scr-top   top-line CELLS lines + @ ;
: scr-rows  (scr-rows) @ ;
: scr-cols  (scr-cols) @ ;
: file-end  (file-end) @ ;
: file-sz   file-end src - ;
: mark-end 26 file-end C! ;
: row  (row) @ ;
: row! (row) ! ;
: col  (col) @ ;
: col! (col) ! ;

25 (scr-rows) !
80 (scr-cols) !

: init-blk 0 row! 0 col! 0 (top-line) !
    src-sz src + src DO 0 I C! LOOP ;

: make-fn ( n-- )
    " block-" pad str-cpy
    3 pad str-catnw
    " .4th" pad str-cat ;

: save-blk ( n-- ) +TMPS
    make-fn pad 1 FOPEN s1
    r1 IF 
        src s2 
        BEGIN r2 C@ DUP IF r1 FPUTC 1 i2 THEN WHILE
        r1 FCLOSE
    THEN
    -TMPS ;

: to-lines +TMPS 
    src s0 lines s9 
    src r9 ! c9
    file-end src DO
        I C@ 10 = IF I 1+ r9 ! c9 THEN
    LOOP
    0 r9 ! -TMPS ;

: rd-file BEGIN
        r1 FGETC 0= IF DROP UNLOOP EXIT THEN
        r2 C! i2 i8
    AGAIN ;

: load-blk ( n-- ) +TMPS
    init-blk src s2 
    make-fn pad 0 FOPEN s1
    r1 IF rd-file r1 FCLOSE THEN
    r2 (file-end) ! mark-end
    \ file-sz pad ." %n(%s: %d bytes)%n"
    to-lines
    -TMPS ;

: T0 scr-top s0 0 s2 
    BEGIN
        r0 c@ s1
        r1 26 = IF UNLOOP EXIT THEN
        r1  9 = IF 32 s1 THEN
        r1 13 = IF 0 s1 THEN
        r1 10 = IF CLR-EOL 13 EMIT i2 THEN
        r1 IF r1 EMIT THEN
        i0 r2 scr-rows <
    WHILE ;

: norm-RC
    row 1 < IF 0 row! THEN
    col 1 < IF 0 col! THEN
    row scr-rows >= IF scr-rows row! THEN
    col scr-cols >= IF scr-cols col! THEN ;

: ->RC norm-RC col 1+ row 1+ ->XY ;

: scr-upd CURSOR-OFF 1 1 ->XY 
    +TMPS T0 -TMPS
    CURSOR-ON ;

: rc->off lines row CELLS + @ col + ;

: do-copy ." -copy-" ;
: do-paste ." -paste-" ;
: do-delch file-end rc->off
    DO I 1+ C@ I C! LOOP 
    -1 (file-end) +! mark-end to-lines
    scr-upd ;
: do-rep-char ( c-- ) rc->off C!  ;
: do-ins-char ( c-- ) s9 
     1 (file-end) +!
    file-end s8 rc->off s7
    BEGIN r8 1- C@ r8 C! d8 r8 r7 > WHILE
    r9 r7 C!
    mark-end  to-lines
    scr-upd ;

: scroll-up ( n-- ) NEGATE (top-line) +!
    top-line 0 < IF 0 (top-line) ! THEN
    scr-upd ;
: scroll-dn (top-line) +! scr-upd ;

: do-pgup  3 scroll-up row 3 + row! ;
: do-pgdn  3 scroll-dn row 3 - row! ;

: do-home 0 col! ;
: do-end  col 4 + col! ;

: do-up  row 1- row! row 0 <         IF 1 scroll-up 0 row! THEN ;
: do-dn  row 1+ row! row scr-rows >= IF 1 scroll-dn scr-rows 1- row! THEN ;
: do-lf  col 1- col! col 0 <         IF do-up do-end  THEN ;
: do-rt  col 1+ col! col scr-cols >= IF do-dn do-home THEN ;

: tty-key ( -- ) key s1
    r1 91 != IF EXIT THEN
    key s2
    r2 65 = IF K-up   s1 THEN
    r2 66 = IF K-dn   s1 THEN
    r2 68 = IF K-lf   s1 THEN
    r2 67 = IF K-rt   s1 THEN
    r2 99 = IF K-home s1 THEN
    r2 99 = IF K-end  s1 THEN
    r2 99 = IF K-pgup s1 THEN
    r2 99 = IF K-pgdn s1 THEN
    r2 99 = IF K-del  s1 THEN ;

: win-key ( -- ) key s2
    r2 72 = IF K-up   s1 THEN
    r2 80 = IF K-dn   s1 THEN
    r2 75 = IF K-lf   s1 THEN
    r2 77 = IF K-rt   s1 THEN
    r2 71 = IF K-home s1 THEN
    r2 79 = IF K-end  s1 THEN
    r2 73 = IF K-pgup s1 THEN
    r2 81 = IF K-pgdn s1 THEN
    r2 83 = IF K-del  s1 THEN ;

: process-key ( k-- ) s1 
    r1 'Q' = IF ST_EXIT st !  EXIT THEN
    r1 K-up    = IF do-up     EXIT THEN
    r1 K-dn    = IF do-dn     EXIT THEN
    r1 K-lf    = IF do-lf     EXIT THEN
    r1 K-rt    = IF do-rt     EXIT THEN
    r1 K-home  = IF do-home   EXIT THEN
    r1 K-end   = IF do-end    EXIT THEN
    r1 K-pgup  = IF do-pgup   EXIT THEN
    r1 K-pgdn  = IF do-pgdn   EXIT THEN
    r1 K-copy  = IF do-copy   EXIT THEN
    r1 K-paste = IF do-paste  EXIT THEN
    r1 K-del   = IF do-delch  EXIT THEN
    r1 K-cr    = IF do-home do-dn  EXIT THEN
    r1  32 < IF r1 .      EXIT THEN
    r1 126 > IF r1 .      EXIT THEN
    st @ ST_INS = IF r1 do-ins-char do-rt EXIT THEN
    r1 do-rep-char ;

: get-key ( --n ) key s1 
    r1 224 = IF win-key THEN 
    r1  27 = IF tty-key THEN 
    r1 ;
: done? st @ ST_EXIT = ;
: edit 0 st ! load-blk CLS scr-upd ->RC
    BEGIN
        get-key process-key ->RC done?
    UNTIL ;

: reload 456 load ;

456 edit
