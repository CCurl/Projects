( comments are free )
( add-word ] 1 state ! 1 state ! exit [ )
( add-word : ] add-word ] ; )
( add-word ; immediate ] 0 , 0 state ! exit [ )

: immediate $80 (l) @ 5 + c! ;

: cell  4 ;
: cell+ cell + ;
: cells cell * ;
: wc-sz 4 ;

: (exit)    0 ; 
: (lit)     1 ;
: (jmpz)    2 ;
: (jmp)     3 ;
: (=)      21 ;
: (ztype)  24 ;

: bye 999 state ! ;
: last (l) @ ;
: here (h) @ ;
: ->code wc-sz * code + ;
: vhere (vh) @ ;
: dict-end vars vars-sz + ;

: comp? (( --n )) state @ 1 = ;
: if (jmpz) , here 0 , ; immediate
: if0 $70000000 , (=) , (jmpz) , here 0 , ; immediate
: then here swap ->code !  ; immediate
: begin here ; immediate
: again (jmp) , , ; immediate
: while $70000000 , (=) , (jmpz) , , ; immediate
: until (jmpz) , , ; immediate

: hex     $10 base ! ;
: decimal #10 base ! ;

: const add-word (lit) , , (exit) , ;
: var vhere const  ;
: allot vhere + (vh) ! ;
: vc, vhere c! 1 allot ;
: v,  vhere ! cell allot ;

: rot >r swap r> swap ;
: -rot rot rot ;
: over >r dup r> swap ;
: tuck swap over ;
: nip  swap drop ;
: ?dup dup if dup then ;
: 0= 0 = ;
: 0< 0 < ;
: 1+ 1 + ;
: cell+ cell + ;
: 1- 1 - ;
: 2* 2 * ;
: +! dup >r @ + r> ! ;
: ++  1 swap +! ;
: -- -1 swap +! ;

var (a) cell allot
: a!  (a) ! ;
: a   (a) @ ;
: a+  (a) @ dup 1+ a! ;
: a+c (a) @ dup cell+ a! ;
: @a   a c@ ;
: @a+  a+ c@ ;
: @ac  a @ ;
: @a+c a+c @ ;
: !a+  a+ c! ;
: !a   a  c! ;

var (b) cell allot
: b! (b) ! ;
: b  (b) @ ;
: b+ (b) @ dup 1+ b! ;

var (t) cell allot
: t! (t) ! ;
: t  (t) @ ;
: t+ (t) @ dup 1+ t! ;


: bl 32 ;
: space bl emit ;
: tab 9 emit ;
: cr 13 emit 10 emit ;

: negate 0 swap - ;
: abs dup 0< if negate then ;

: /   /mod nip  ;
: mod /mod drop ;

var (neg) 1 allot
var buf 65 allot
var (buf) cell allot
: ?neg 0 (neg) c!  dup 0< if negate 1 (neg) c! then ;
: #c (buf) -- (buf) @ c! ;
: #digit '0' + dup '9' > if 7 + then #c ;
: # base @ /mod swap #digit ;
: #S # dup if #S exit then drop ;
: <# ?neg buf 65 + (buf) ! 0 #c ;
: #> (neg) @ if '-' #c then (buf) @ ;

: (.) <# #S #> ztype ;
: . (.) space ;
: ? @ . ;

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth if
        (stk) cell+ a! depth for a+c @ . next 
    then ')' emit ;

: (") vhere dup a! >in ++
    begin >in @ c@ >r >in ++
        r@ 0= r@ '"' = or
        if  r> drop 0 !a+
            comp? if (lit) , , a (vh) ! then exit
        then
        r> !a+
    again ;

: z" (") ; immediate
: ." (") comp? if (ztype) , exit then ztype ;  immediate

: words last a! 0 b! 0 t! begin
        a dict-end < if0 '(' emit t . ." words)" exit then
        a 7 + ztype 9 emit
        (t) ++ b+ 9 > if cr 0 b! then
        a wc-sz + c@ a + a!
    again ;

( temp for testing )
: elapsed timer swap - . ." usec" cr ;
: bm1 timer swap begin 1- dup while drop elapsed ;
: bm2 timer swap for next elapsed ;
: mil #1000 dup * * ;
: bm 250 mil bm1 1000 mil bm2 ;
: bb 1000 mil bm2 ;
 ." dwc v0.0.1" cr ." Hello."
