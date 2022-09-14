1 load
2 load

10000 CONSTANT src-sz
500 CONSTANT max-lines

variable lines max-lines CELLS ALLOT
variable (num-lines)
variable (scr-top)
variable (scr-rows)
variable (scr-cols)
variable (file-sz)
variable pad 128 ALLOT
variable st 0 st !
variable src src-sz ALLOT
variable (row)
variable (col)

: num-lines (num-lines) @ ;
: scr-top   (scr-top) @ ;
: scr-rows  (scr-rows) @ ;
: scr-cols  (scr-cols) @ ;
: file-sz   (file-sz) @ ;
: file-end  src file-sz + ;
: row  (row) @ ;
: row! (row) ! ;
: col  (col) @ ;
: col! (col) ! ;

0 (scr-top) !
25 (scr-rows) !
80 (scr-cols) !

: init-blk src src-sz 0 DO 0 OVER C! 1+ LOOP drop
    1 row! 1 col! ;

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

: load-blk ( n-- ) +TMPS
    init-blk 0 s9
    make-fn pad 0 FOPEN s1
    r1 IF
        src s2
        BEGIN r1 FGETC SWAP r2 C! i2 i9 WHILE
        d2 d2 0 r2 C!
        d9 d9
        r1 FCLOSE
    THEN 
    CLS src qtype
    r9 (file-sz) ! r9 pad ." %n(%s: %d bytes)%n"
    -TMPS ;

: scr-upd CURSOR-OFF 1 1 ->XY +TMPS
    src scr-top + s0
    scr-rows 0 DO 
        scr-cols 0 DO
            r0 c@ s1
            r1  32 < IF 32 s1 THEN
            r1 127 > IF 32 s1 THEN
            r1 EMIT
            r0 file-end < IF i0 THEN
        LOOP CR
    LOOP
    -TMPS CURSOR-ON ;

: rc->off row 1- scr-cols * col + 1- scr-top + src + ;

: do-copy ." -copy-" ;
: do-paste ." -paste-" ;
: do-delch file-end rc->off 
    DO I 1+ C@ I C! LOOP 
    0 file-end c! 
    -1 (file-sz) +! scr-upd ;
: do-insch ." -insch:" KEY ." %c-" ;
: do-repch ." -repch:" KEY ." %c-" ;

: norm-RC
    row 1 < IF 1 row! THEN
    col 1 < IF 1 col! THEN
    row scr-rows >= IF scr-rows row! THEN
    col scr-cols >= IF scr-cols col! THEN ;
: ->RC norm-RC col row ->XY ;

: do-pgup scr-top scr-cols - (scr-top) !
    scr-top 0 < IF 0 (scr-top) ! THEN 
    scr-upd ;
: do-pgdn  scr-top scr-cols + (scr-top) ! scr-upd ;

: do-home 1 col! ;
: do-end  scr-cols col! ;

: do-up  row 1- row! row 1 <         IF do-pgup row 1+ row! THEN ;
: do-dn  row 1+ row! row scr-rows >= IF do-pgdn row 1- row! THEN ;
: do-lf  col 1- col! col 1 <         IF do-up do-end  THEN ;
: do-rt  col 1+ col! col scr-cols >= IF do-dn do-home THEN ;

: win-key ( -- ) key s2
    r2 72 = IF 'w' s1 THEN
    r2 80 = IF 's' s1 THEN
    r2 75 = IF 'a' s1 THEN
    r2 77 = IF 'd' s1 THEN
    r2 73 = IF 'r' s1 THEN
    r2 81 = IF 'f' s1 THEN
    r2 71 = IF 'q' s1 THEN
    r2 79 = IF 'e' s1 THEN
    r2 83 = IF 'x' s1 THEN ;

: process-key ( k-- ) s1 
    r1 'Q' = IF 999 st !  EXIT THEN
    r1 'w' = IF do-up   EXIT THEN
    r1 'd' = IF do-rt   EXIT THEN
    r1 'a' = IF do-lf   EXIT THEN
    r1 's' = IF do-dn   EXIT THEN
    r1 'q' = IF do-home EXIT THEN
    r1 'e' = IF do-end  EXIT THEN
    r1 'r' = IF do-pgup EXIT THEN
    r1 'f' = IF do-pgdn EXIT THEN
    r1 'c' = IF do-copy   EXIT THEN
    r1 'v' = IF do-paste  EXIT THEN
    r1 'p' = IF do-repch  EXIT THEN
    r1 'x' = IF do-delch  EXIT THEN
    r1 'i' = IF do-insch  EXIT THEN
    r1  13 = IF do-home do-dn  EXIT THEN
    r1  32 < IF r1 .      EXIT THEN
    r1 126 > IF r1 .      EXIT THEN
    r1 EMIT ;

: get-key ( --n ) key s1 r1 224 = IF win-key THEN r1 ;
: done? st @ 999 = ;
: edit 0 st ! load-blk CLS scr-upd ->RC
    BEGIN
        get-key process-key ->RC done?
    UNTIL ;

: reload 456 load ;

456 edit
