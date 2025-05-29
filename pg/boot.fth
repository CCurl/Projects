add-word ] 1 state ! 1 state ! exit [
add-word immediate ] $80 (l) @ 3 + c! exit [
add-word ; immediate ] 0 , 0 state ! exit [
add-word : ] add-word ] ;

: cell 8 ;
: wc-sz 4 ;

: \ 0 >in @ ! ;

: (exit)    0 ; 
: (lit)     1 ;
: (jmpz)    2 ;
: (,)       3 ;
: (dup)     4 ;
: (drop)    5 ;
: (swap)    6 ;
: (!)       7 ;

: last (l) @ ;
: here (h) @ ;
: vhere (vh) @ ;
: dict-end dict dict-sz + ;

: cells cell * ; inline

: const add-word (lit) , , (exit) , ;
: var vhere const  ;
: allot vhere + (vh) ! ;
: vc, vhere c! 1 allot ;
: v,  vhere ! cell allot ;

: if (jmpz) c, here 0 , ; immediate
: then here swap !      ; immediate
: exit (exit) c,        ; immediate

: tuck swap over ; inline
: nip  swap drop ; inline
: ?dup dup if dup then ;

: begin here         ; immediate
: while (jmpnz) c, , ; immediate
: until (jmpz)  c, , ; immediate
: again (jmp)   c, , ; immediate

: ( begin 
        >in @ c@ >in @ 1+ >in !
        dup  0= if drop exit then
        ')' = if drop exit then
    again ; immediate

: and [ (bitop) c, 11 c, ] ; inline
: or  [ (bitop) c, 12 c, ] ; inline
: xor [ (bitop) c, 13 c, ] ; inline
: com [ (bitop) c, 14 c, ] ; inline

: >r [ (retop) c, 11 c, ] ; inline
: r@ [ (retop) c, 12 c, ] ; inline
: r> [ (retop) c, 13 c, ] ; inline
: rdrop r> drop ; inline
: rot >r swap r> swap ;
: -rot rot rot ;

: (i) (lsp) @ cells (lstk) + ;
: i (i) @ ;
: +i (i) tuck @ + swap ! ;
: unloop (lsp) @ 3 - (lsp) ! ;

: bl 32 ; inline
: space bl emit ; inline
: tab 9 emit ; inline
: cr 13 emit 10 emit ; inline

: negate com 1+ ; inline
: abs dup 0 < if negate then ;
: +!  dup @   + swap !  ; inline
: ++  dup @  1+ swap !  ; inline
: c++ dup c@ 1+ swap c! ; inline

: /   /mod nip  ; inline
: mod /mod swap ; inline

var (neg) cell allot
var (len) cell allot
: len (len) @ ;
: #digit '0' + dup '9' > if 7 + then ;
: <# 0 (neg) c! 0 (len) ! dup 0 < 
    if negate 1 (neg) ! then 0 swap ;         \ ( n1 -- 0 n2 )
: # base @ /mod swap #digit swap (len) ++ ;   \ ( u1 -- c u2 )
: #S begin # dup while ;                      \ ( u1 -- u2 )
: #> ;
: #- drop (neg) @ if '-' emit then ;
: #P #- begin emit dup while drop ;           \ ( 0 ... n 0 -- )
: (.) <# #S #> #P ;
: . (.) space ;

: count dup 1+ swap c@ ; inline
: type 0 do dup c@ emit 1+ loop drop ;

: S" (lit4) c, vhere ,
    vhere >r 0 vc,
    begin >in @ c@ >in ++
        dup 0= over '"' = or
        if drop 0 vc, rdrop exit then
        vc, r@ c++
    again ; immediate

: ." [ (call) c, ' S" drop drop , ]
    (call) c, [ (lit4) c, ' count drop drop , ] ,
    (call) c, [ (lit4) c, ' type  drop drop , ] , ;  immediate

: hex     #16 base ! ;
: decimal #10 base ! ;
: ? @ . ;

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth ?dup if
        0 do (stk) i 1+ cells + @ . loop 
    then ')' emit ;

: words last begin
        dup mem-end < 0=  if drop exit then
        dup 1+ count type tab word-sz +
    again ;

\ temp for testing
: ms (.) ."  usec " ;
: elapsed timer swap - ms ;
: bm1 timer swap begin 1- dup while drop elapsed ;
: bm2 timer swap 0 do loop elapsed ;
: mil #1000 dup * * ;
: sz #500 mil ;

sz bm1 sz bm2
\ bye
