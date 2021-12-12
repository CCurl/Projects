: here  (here)  @ ;
: vhere (vhere) @ ;
: last  (last)  @ ;

: immediate 2 last addr + c! ;
: inline    1 last addr + c! ;

: [ 0 state ! ; immediate
: ] 1 state ! ;

: +! swap over @ + swap ! ;
: c, here c! 1 (here) +! ;
: , here ! cell (here) +! ;

: bye [ '`' c, 'q' c, ] ;

: dup  [ '#' c, ] ; inline
: drop [ '\' c, ] ; inline
: over [ '%' c, ] ; inline

: nip   swap drop ; inline
: tuck  swap over ; inline
: 2dup  over over ; inline
: 2drop drop drop ; inline

: and [ '&' c, ] ; inline
: or  [ '|' c, ] ; inline
: xor [ '^' c, ] ; inline
: 0=  [ '_' c, ] ; inline
: <<  [ 'L' c, ] ; inline
: >>  [ 'R' c, ] ; inline

: -     [ '-' c, ] ; inline
: *     [ '*' c, ] ; inline
: /     [ '/' c, ] ; inline
: /mod  [ 'S' c, ] ; inline
: mod   [ 'D' c, ] ; inline

: =  [ '=' c, ] ; inline
: <  [ '<' c, ] ; inline
: >  [ '>' c, ] ; inline

: abs    [ 'U' c, ] ; inline
: negate [ 'N' c, ] ; inline
: rand   [ 'r' c, ] ; inline

: 1-  [ 'M' c, ] ; inline
: 1+  [ 'P' c, ] ; inline
: 2+ 1+ 1+ ;       inline
: 2* 1 << ;        inline

: allot (vhere) +! ;
: cells cell * ; inline

: emit [ ',' c, ] ; inline
: (.) [ '.' c, ] ;  inline
: bl $20 ;          inline
: space bl emit ;   inline
: . (.) space ;     inline
: ? @ . ;           inline
: cr #13 emit #10 emit ;

: for  [ '[' c, ] ; inline
: next [ ']' c, ] ; inline
: (i)  [ 'I' c, ] ; inline
: i    (i) @      ; inline

: if    [ '(' c, ] ; inline
: then  [ ')' c, ] ; inline

: begin  [ '{' c, ] ; inline
: while  [ '}' c, ] ; inline
: until 0= while    ; inline
: again 1  while    ; inline
: break  [ 'X' c, ] ; inline

: execute [ '`' c, 'J' c, ] ; inline
: timer   [ '`' c, 'T' c, ] ; inline

: count dup 1+ swap c@ ;      inline
: type 1 for dup c@ emit 1+ next drop ;
: ztype begin dup c@ emit 1+ dup c@ while drop ;

: dump for i c@ . next ;
: print-ch dup 0= if drop '.' then emit ;
: dump-ch for i c@ dup print-ch emit next ;

: num-words user-end 1+ last - dentry-sz / ;
: .w dup cell + 1+ count type space ;
: words last num-words 1 for .w dentry-sz + next drop ;

: hex     $10 base ! ; inline
: decimal #10 base ! ; inline
: binary  %10 base ! ; inline

: load [ '`' c, 'B' c, 'L' c, ] ; inline

variable fg-sv 2 cells allot
: marker fg-sv here over ! cell + last over ! cell + vhere swap ! ;
: forget fg-sv dup @ (here) ! cell + dup @ (last) ! cell + @ (vhere) ! ;
: help 999 load ;

marker 
help
