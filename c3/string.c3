\ string.c3 - some string words

' count loaded?

\ Just some temp buffers for convenience
: x1 vhere $100 + ;
: x2 vhere $200 + ;

\ Words for NULL-Terminated strings
: s-catn  ( str n-- ) ITOA S-CAT ;
: s-scatn ( n str-- ) SWAP s-catn ;
: s-scpy  ( s d-- )   SWAP S-CPY ;
: s-scat  ( s d-- )   SWAP S-CAT ;


\ Compare the beginning N chars of the strings
: s-eq-n ( S D N--F ) +REGS s3 s2 s1
        r3 0 DO
            r1+ C@ r2+ C@ <> IF UNLOOP 0 -EXIT THEN
        LOOP
    -REGS 1 ;

\ s-findc looks for char CH in string at STR
\ Returns addr of CH if found, else 0
: s-findc ( CH STR -- addr|0 ) 
    +REGS s2 s1
	BEGIN r2 c@ s4
		r4 0= IF 0 -EXIT THEN
		r4 r1 = IF r2 -EXIT THEN
		i2
	AGAIN ;

\ s-find looks for string LF in string at STR
\ Returns addr of LF if found, else 0
: s-find ( LF STR -- addr|0 ) 
    +regs s2 s1  r1 c@ s6
    r1 S-LEN s3
    BEGIN
        r6 r2 s-findc s2  r2 0= IF 0 -EXIT THEN
        r1 r2 r3 s-eq-n IF r2 -EXIT THEN
        i2
     AGAIN ;

\ Words for Counted strings
: sc-len ( str--len ) C@ ; INLINE

: sz->sc ( sz sc-- ) +REGS s2 s1
    r1 S-LEN r2+ C!
    r2 r1 S-CPY
    -REGS ;

: sc->sz ( sc sz-- ) SWAP 1+ S-CPY ; INLINE

: count ( sc--a n ) 1+ DUP 1- C@ ;
