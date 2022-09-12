1 load
2 load

10000 CONSTANT f-sz
500 CONSTANT #lines

variable lines #lines CELLS ALLOT
variable src f-sz ALLOT
variable pad 128 ALLOT

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

: to-lines ;

: edit-loop ;

: edit load-blk to-lines edit-loop ;

: reload 789 load ;

0 edit
