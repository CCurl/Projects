1 load
2 load

10000 CONSTANT max-sz
500 CONSTANT max-lines

variable lines max-lines CELLS ALLOT
variable num-lines
variable file-sz
variable src max-sz ALLOT
variable pad 128 ALLOT
variable st 0 st !

: init-blk src max-sz 0 DO 0 OVER C! 1+ LOOP drop ;

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
    CR src qtype
    r9 file-sz ! r9 pad ." %n(%s: %d bytes)%n"
    -TMPS ;

: to-lines src s0 lines s2 1 s3
    file-sz @ 0 DO r0 C@ $7F AND s1
        r1 $D = IF $8D r0 C! THEN // CR
        r1 $A = IF $8A r0 C! THEN // LF
        r1 $A = IF r0 r2 ! c2 i3 THEN 
        i0
    LOOP
    r3 num-lines ! ;

: do-copy ." -copy-" ;
: do-paste ." -paste-" ;

: move-up ." -up-" ;
: move-lf ." -lf-" ;
: move-rt ." -rt-" ;
: move-dn ." -dn-" ;
: move-pgup ." -pgup-" ;
: move-pgdn ." -pgdn-" ;
: move-home ." -home-" ;
: move-end  ." -end-" ;

: process-key ( k-- ) s1 
    r1 'Q' = IF 999 st ! EXIT THEN
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
    r1 32  < IF r1 .      EXIT THEN
    r1 126 > IF r1 .      EXIT THEN
    r1 EMIT ;

: done? st @ 999 = ;
: edit 0 st ! load-blk to-lines begin key process-key done? until ;

: reload 456 load ;

0 edit
