add-word ] 1 state ! 1 state ! exit [
add-word immediate ] $80 (l) @ 5 + c! exit [
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

: cells cell * ;

: const add-word (lit) , , 0 , (exit) , ;
: var vhere const  ;
: allot vhere + (vh) ! ;
: vc, vhere c! 1 allot ;
: v,  vhere ! cell allot ;

: over >r dup r> swap ;
: tuck swap over ;
: nip  swap drop ;
: ?dup dup if dup then ;
: +! dup >r @ + r> ! ;
: 0= 0 = ;
: 0< 0 < ;
: 1+ 1 + ;
: 1- 1 - ;

: (   >in @ c@ 1 >in +!
      dup emit
      dup 0= if drop exit then
      ')' = if exit then
    ( ; immediate

: rot >r swap r> swap ;
: -rot rot rot ;

: bl 32 ;
: space bl emit ;
: tab 9 emit ;
: cr 13 emit 10 emit ;

: negate 0 swap - ;
: abs dup 0< if negate then ;
: ++  dup @  1+ swap !  ;
: --  dup @  1- swap !  ;
: c++ dup c@ 1+ swap c! ;

: /   /mod nip  ;
: mod /mod swap ;
: xx emit cr ;

var (neg) 1 allot
var buf 65 allot
var (buf) cell allot
: ?neg 0 (neg) c!  dup 0< if negate 1 (neg) c! then ;
: #c (buf) -- (buf) @ c! ;
: #digit '0' + dup '9' > if 7 + then #c ;
: #digit '0' +  #c ;
: # base @ /mod swap #digit ;
: #S # dup if #S exit then drop ;
: <# ?neg buf 65 + (buf) ! 0 #c ;
: #> (neg) @ if '-' #c then (buf) @ ;

: (.) <# #S #> (buf) @ ztype ;
: . (.) space ;
1234 . -5678 .

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
