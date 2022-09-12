1 load
2 load

10000 CONSTANT f-sz
500 CONSTANT #lines

variable lines #lines CELLS ALLOT
variable src f-sz ALLOT
variable pad 128 ALLOT
variable st 0 st !

: init-blk src f-sz 0 do 0 OVER C! 1+ loop drop ;

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
    init-blk make-fn pad 0 FOPEN s1
    r1 IF
        src s2
        BEGIN r1 FGETC SWAP r2 C! i2 WHILE
        d2 d2 0 r2 C!
        r1 FCLOSE
    THEN 
    CR src qtype
    -TMPS ;

: to-lines src qtype ;

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
    r1 23 = IF move-up   EXIT THEN
    r1  4 = IF move-rt   EXIT THEN
    r1  1 = IF move-lf   EXIT THEN
    r1 19 = IF move-dn   EXIT THEN
    r1 18 = IF move-pgup EXIT THEN
    r1  6 = IF move-pgdn EXIT THEN
    r1 17 = IF move-home EXIT THEN
    r1  5 = IF move-end  EXIT THEN
    r1  3 = IF do-copy   EXIT THEN
    r1 22 = IF do-paste  EXIT THEN
    r1 .
    ;

: done? st @ 999 = ;
: edit 0 st ! load-blk to-lines begin key process-key done? until ;

: reload 456 load ;

0 edit
