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

: num-lines (num-lines) @ ;
: scr-top   (scr-top) @ ;
: scr-rows  (scr-rows) @ ;
: scr-cols  (scr-cols) @ ;
: file-sz   (file-sz) @ ;
: file-end  src file-sz + ;

0 (scr-top) !
25 (scr-rows) !
80 (scr-cols) !

: init-blk src src-sz 0 DO 0 OVER C! 1+ LOOP drop ;

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

: to-lines src s0 lines s2 0 s3
    r0 r2 ! c2 i3
    file-sz 0 DO r0 C@ $7F AND s1
        r1 $D = IF $8D r0 C! THEN // CR
        r1 $A = IF $8A r0 C! THEN // LF
        r1 $A = IF r0 r2 ! c2 i3 THEN // next-line
        r0 C@ 32 < IF $20 r0 C! THEN // SPC
        i0
    LOOP
    r3 (num-lines) ! ;

: scr-upd 1 1 ->XY +TMPS
    src scr-top + s0
    scr-rows 0 DO
        scr-cols 0 DO
            r0 c@ s1
            r1 0 = IF 17 s1 THEN
            r1 $8D = IF $AE s1 THEN
            r1 $8A = IF $B3 s1 THEN
            r1 EMIT
            r0 file-end < IF i0 THEN
        LOOP CR
    LOOP
    -TMPS ;

: do-copy ." -copy-" ;
: do-paste ." -paste-" ;
: do-delch ." -delch-" ;
: do-insch ." -insch:" KEY EMIT '-' EMIT ;
: do-repch ." -repch:" KEY EMIT '-' EMIT ;

: move-up ." -up-" ;
: move-lf ." -lf-" ;
: move-rt ." -rt-" ;
: move-dn ." -dn-" ;
: move-pgup  scr-top scr-cols - 
    dup 0 < IF DROP 0 THEN 
    (scr-top) ! scr-upd ;
: move-pgdn  scr-top scr-cols + (scr-top) ! scr-upd ;
: move-home ." -home-" ;
: move-end  ." -end-" ;

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
    r1 'w' = IF move-up   EXIT THEN
    r1 'd' = IF move-rt   EXIT THEN
    r1 'a' = IF move-lf   EXIT THEN
    r1 's' = IF move-dn   EXIT THEN
    r1 'q' = IF move-home EXIT THEN
    r1 'e' = IF move-end  EXIT THEN
    r1 'r' = IF move-pgup EXIT THEN
    r1 'f' = IF move-pgdn EXIT THEN
    r1 'c' = IF do-copy   EXIT THEN
    r1 'v' = IF do-paste  EXIT THEN
    r1 'p' = IF do-repch  EXIT THEN
    r1 'x' = IF do-delch  EXIT THEN
    r1 'i' = IF do-insch  EXIT THEN
    r1  13 = IF move-home move-dn  EXIT THEN
    r1  32 < IF r1 .      EXIT THEN
    r1 126 > IF r1 .      EXIT THEN
    r1 EMIT ;

: get-key ( --n ) key s1 r1 224 = IF win-key THEN r1 ;
: done? st @ 999 = ;
: edit 0 st ! load-blk to-lines CLS scr-upd
    BEGIN
        get-key process-key done?
    UNTIL ;

: reload 456 load ;

456 edit
