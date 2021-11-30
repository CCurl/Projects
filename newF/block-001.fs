: here (here) @ ;
: last (last) @ ;

: immediate 2 last addr + c! ;
: inline    1 last addr + c! ;

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

: =  [ '=' c, ] ; inline
: <  [ '<' c, ] ; inline
: >  [ '>' c, ] ; inline

: 1-  1 - ; inline
: 1+  1 + ; inline

: emit [ ',' c, ] ; inline
: bl 32 ; inline
: space bl emit ; inline
: (.) [ '.' c, ] ; inline
: . (.) space ; inline

: for  [ '[' c, ] ; inline
: next [ ']' c, ] ; inline
: i    [ 'I' c, ] ; inline

: if   [ '(' c, ] ; inline
: then [ ')' c, ] ; inline

: begin [ '{' c, ] ; inline
: while [ '}' c, ] ; inline

: bye [ '`' c, 'q' c, ] ;

: count dup 1+ swap c@ ;
: type 1 for dup c@ emit 1+ next drop ;

: print-ch dup 0= if drop '.' then emit ;
: dd user here 1- for i @ c@ . next ;
: df user here 1- for i @ c@ print-ch next ;

: num-words user-end 1+ last - dentry-sz / ;
: .w dup cell + 1+ count type space ;
: words last num-words 1 for .w dentry-sz + next drop ;
