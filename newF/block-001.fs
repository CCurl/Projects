: here (here) @ ;
: last (last) @ ;

: immediate 2 last addr + c! ;
: inline 1 last addr + c! ;

: [ 0 state ! ; immediate
: ] 1 state ! ;

: +! swap over @ + swap ! ;
: c, here c! 1 (here) +! ;

: dup  [ '#' c, ] ; inline
: drop [ '\' c, ] ; inline
: over [ '%' c, ] ; inline
: nip swap drop ; inline
: tuck swap over ; inline

: and [ '&' c, ] ; inline
: or  [ '|' c, ] ; inline
: xor [ '^' c, ] ; inline
: 0=  [ '_' c, ] ; inline

: -  [ '-' c, ] ; inline
: *  [ '*' c, ] ; inline
: /  [ '/' c, ] ; inline
: /mod  [ '`' c, '/' c, ] ; inline

: 1-  1 - ; inline
: 1+  1 + ; inline

: emit [ ',' c, ] ; inline
: space 32 emit ; inline
: . space [ '.' c, ] ; inline

: for  [ '[' c, ] ; inline
: next [ ']' c, ] ; inline
: i [ 'I' c, ] ; inline

: if   '(' c, ; immediate
: then ')' c, ; immediate

: bye [ '`' c, 'q' c, ] ;


: type 1 for dup i @ + c@ emit next drop ;

: dd here 1- user for i @ c@ . next ;
: df user here 1- user - type ;

