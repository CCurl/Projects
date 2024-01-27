\ string.c3 - some string words

' count loaded?

\ Words for NULL-Terminated strings
: s-catn  ( str n-- ) ITOA S-CAT ;
: s-scatn ( n str-- ) SWAP s-catn ;
: s-scpy  ( s d-- )   SWAP S-CPY ;
: s-scat  ( s d-- )   SWAP S-CAT ;

\ Compare the beginning N chars of the strings
: s-eq-n ( S D N--F ) +REGS s3 s2 s1
        r3 0 DO
            r1+ C@ r2+ C@ <> IF -REGS UNLOOP 0 EXIT THEN
        LOOP
    -REGS 1 ;

: find ( lookFor in -- addr|0 ) 
    +regs s2 s1
    r1 S-LEN s3
    begin
        r2 C@ 0= IF -REGS 0 EXIT THEN
        r1 r2 r3 s-eq-n
        IF r2 -REGS EXIT THEN
        i2
     again ;

\ Words for Counted strings
: sc-len ( str--len ) C@ ; INLINE

: sz->sc ( sz sc-- ) +REGS s2 s1
    r1 S-LEN r2+ C!
    r2 r1 S-CPY
    -REGS ;

: sc->sz ( sc sz-- ) SWAP 1+ S-CPY ; INLINE

: count ( sc--a n ) 1+ DUP 1- C@ ;
