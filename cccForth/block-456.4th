1 load
2 load

10000 CONSTANT src-sz
  500 CONSTANT max-lines
   30 CONSTANT scr-rows

scr-rows 2/ CONSTANT page-sz

  0 CONSTANT ST_INS
  1 CONSTANT ST_REPL
999 CONSTANT ST_EXIT

   8 CONSTANT K-bs1
 127 CONSTANT K-bs2
  13 CONSTANT K-cr
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
2013 CONSTANT K-esc

variable lines max-lines CELLS ALLOT
variable (num-lines)
variable (top-line)
variable (file-sz)
variable (file-end)
variable pad 128 ALLOT
variable st ST_INS st !
variable src src-sz ALLOT
variable (row)
variable (col)
variable (block-num)

: num-lines (num-lines) @ ;
: top-line  (top-line) @ ;
: scr-top   top-line CELLS lines + @ ;
: file-end  (file-end) @ ;
: file-sz   file-end src - ;
: mark-end 26 file-end C! ;
: row  (row) @ ;
: row! (row) ! ;
: col  (col) @ ;
: col! (col) ! ;

: init-blk 0 row! 0 col! 0 (top-line) !
    src-sz src + src DO 0 I C! LOOP ;

: make-fn ( -- )
    (block-num) @
    " block-" pad str-cpy
    3 pad str-catnw
    " .4th" pad str-cat ;

: save-blk ( -- )
    make-fn pad ." Saving %s..."
    pad 1 FOPEN s1
    r1 0= IF ." failed!" EXIT THEN
    file-end 1- src DO I C@ r1 FPUTC LOOP
    r1 FCLOSE
    ." saved." ;

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

: load-blk ( -- ) +TMPS
    init-blk src s2 
    make-fn pad 0 FOPEN s1
    r1 IF rd-file r1 FCLOSE THEN
    r2 (file-end) ! mark-end
    \ file-sz pad ." %n(%s: %d bytes)%n"
    to-lines
    -TMPS ;

: norm-RC
    row 1 < IF 0 row! THEN
    col 1 < IF 0 col! THEN
    row scr-rows >= IF scr-rows row! THEN ;

: ->RC norm-RC col 1+ row 1+ ->XY ;
: ->cmd-row 1 scr-rows 1+ ->XY ;

: show-eof 26 emit CLR-EOS ;

: T0 scr-top s0 0 s2 
    BEGIN
        r0 c@ s1
        r1 26 = IF UNLOOP r0 (file-end) ! show-eof EXIT THEN
        r1  9 = IF 32 s1 THEN
        r1 13 = IF 0 s1 THEN
        r1 10 = IF CLR-EOL 13 EMIT i2 THEN
        r1 IF r1 EMIT THEN
        i0 r2 scr-rows <
    WHILE ;

: scr-upd CURSOR-OFF 1 1 ->XY 
    +TMPS T0 -TMPS
    CURSOR-ON ;

: rc->off lines row top-line + CELLS + @ col + ;

: do-copy ." -copy-" ;
: do-paste ." -paste-" ;
: do-delch file-end rc->off
    DO I 1+ C@ I C! LOOP 
    -1 (file-end) +! mark-end to-lines
    scr-upd ;
: scroll-up ( n-- ) NEGATE (top-line) +!
    top-line 0 < IF 0 (top-line) ! THEN
    scr-upd ;
: scroll-dn ( n-- ) (top-line) +! scr-upd ;

: do-pgup  page-sz scroll-up row page-sz + row! ;
: do-pgdn  page-sz scroll-dn row page-sz - row! ;

: do-home 0 col! ;
: do-end  col 4 + col! ;

: do-up  row 1- row! row 0 <         IF 1 scroll-up 0 row! THEN ;
: do-dn  row 1+ row! row scr-rows >= IF 1 scroll-dn scr-rows 1- row! THEN ;
: do-lf  col 1- col! col 0 <         IF do-up do-end  THEN ;
: do-rt  col 1+ col! ;

: do-rep-char ( c-- ) rc->off C! do-rt ;
: do-ins-char ( c-- ) s9 
    1 (file-end) +!
    file-end s8 rc->off s7
    BEGIN r8 1- C@ r8 C! d8 r8 r7 > WHILE
    r9 r7 C! mark-end to-lines
    scr-upd ;
: do-bs do-lf do-delch ;
: do-cr st @ ST_INS = IF 
        10 do-ins-char 13 do-ins-char do-rt
    THEN
    do-dn do-home ;

: do-esc ->cmd-row ." :" CLR-EOL key s1
    r1 'W' = IF save-blk EXIT THEN
    r1 'Q' = IF save-blk 'q' s1 THEN
    r1 'q' = IF ST_EXIT st ! EXIT THEN
    r1 ." -%c?-" ;

: tty-key ( -- ) key s1
    r1 27 = IF K-esc s1 EXIT THEN
    r1 91 != IF EXIT THEN
    key s2
    r2 65 = IF K-up   s1 THEN
    r2 66 = IF K-dn   s1 THEN
    r2 68 = IF K-lf   s1 THEN
    r2 67 = IF K-rt   s1 THEN
    r2 72 = IF K-home s1 THEN
    r2 70 = IF K-end  s1 THEN
    r2 53 = IF key DROP K-pgup s1 THEN
    r2 54 = IF key DROP K-pgdn s1 THEN
    r2 51 = IF key DROP K-del  s1 THEN ;

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
    r1 K-esc   = IF do-esc    EXIT THEN
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
    r1 K-bs1   = IF do-bs     EXIT THEN
    r1 K-bs2   = IF do-bs     EXIT THEN
    r1 K-cr    = IF do-cr     EXIT THEN
    r1   13    < IF r1 .      EXIT THEN
    r1   32    < IF r1 .      EXIT THEN
    r1  126    > IF r1 .      EXIT THEN
    st @ ST_REPL = IF r1 do-rep-char EXIT THEN
    r1 do-ins-char do-rt ;

: get-key ( --n ) key s1 
    r1 224 = IF win-key THEN 
    r1  27 = IF tty-key THEN 
    r1 ;
: done? st @ ST_EXIT = ;
: edit (block-num) ! load-blk CLS scr-upd ->RC 0 st !
    BEGIN
        get-key process-key ->RC done?
   UNTIL
   ->cmd-row ;

: reload 456 load ;
