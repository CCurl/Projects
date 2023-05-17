: here (here) @ ;
: c, here c! 1 (here) +! ;
: , here ! here cell + (here) ! ;

: vhere  (vhere) @ ;
: allot  vhere + (vhere) ! ;
: vc, vhere c! 1 (vhere) +! ;
: v,  vhere ! cell allot ;

: last (last) @ ;
: immediate 1 last cell + c! ;
: inline 2 last cell + c! ;

: \ 0 >in @ ! ; immediate
: [ 0 state ! ; immediate
: ] 1 state ! ;
: bye 999 state ! ;
: cells cell * ; inline

: constant  create (lit4) c, , (exit) c, ;
: variable  vhere constant cell allot ;
: val  vhere constant ;
: (val)  here 1- cell - constant ;

: does>  r> last ! ;
: :noname  here 1 state ! ;
: exec  >r ;

: (    (jmpz) c, here 0 , ; immediate
: |  (jmp) c, here swap 0 , here swap ! ; immediate
: )  here swap ! ; immediate
: exit  (exit) c,   ; immediate

: begin  here         ; immediate
: while  (jmpnz) c, , ; immediate
: until  (jmpz)  c, , ; immediate
: again  (jmp)   c, , ; immediate

: tuck  swap over ; inline
: nip   swap drop ; inline
: 2dup  over over ; inline
: 2drop drop drop ; inline
: ?dup  dup ( dup ) ;

: ++  dup @  1+ swap ! ; inline
: c++ dup c@ 1+ swap c! ; inline
: 2*  dup + ; inline
: 2+  1+ 1+ ; inline
: <=  > 0= ; inline
: >=  < 0= ; inline
: <>  = 0= ; inline

: rdrop r> drop ; inline
: rot   >r swap r> swap ;
: -rot  swap >r swap r> ;

: bl  #32 ; inline
: tab  #9 emit ; inline
: cr  #13 emit #10 emit ; inline
: space bl emit ; inline

: negate  com 1+ ; inline
: abs  dup 0 < ( negate ) ;
: min  over over > ( swap ) drop ;
: max  over over < ( swap ) drop ;

: i  (i) @ ;
: j  (i) 3 cells - @ ;
: +i (i) +! ;
: unloop (lsp) @ 3 - 0 max (lsp) ! ;

: /   /mod nip  ; inline
: mod /mod drop ; inline

variable (neg)
: #digit '0' + dup '9' > ( 7 + ) ;
: <# 0 swap dup 0 < (neg) ! abs ;    \ (( n1 -- 0 n2 )
: # base @ /mod swap #digit swap ;   \ (( u1 -- c u2 )
: #S begin # dup while ;             \ (( u1 -- u2 )
: #> drop (neg) @ ( '-' ) ;
: #P begin emit dup while drop ;     \ (( 0 ... n 0 -- )
: (.) <# #S #> #P ;
: . (.) space ;

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth ?dup (
        0 do (stk) i 1+ cells + @ . loop 
    ) ')' emit ;

: count  dup 1+ swap c@ ; inline

\ T2: ( --cStr end )
: T8    r8 c! i8 ;
: T2    +regs
    vhere dup s8 s9   0 T8
    begin >in @ c@ s1
        r1 ( >in ++ )
        r1 0= r1 '"' = or
        ( 0 T8   r9 r8 -regs   exit )
        r1 T8   r9 c++
    again ;

: s" T2 state @ 0= ( drop exit ) (vhere) ! (lit4) c, , ; immediate 

: ."   T2 state @ 0= ( drop count type exit ) 
    (vhere) ! (lit4) c, ,
    (call) c, [ (lit4) c, ' count drop drop , ] ,
    (call) c, [ (lit4) c, ' type  drop drop , ] , ;  immediate

: .word cell + 1+ count type ;
: T0 i1 r1 #11 mod 0= ( cr ) ;
: T1 T0 r2 .word tab r2 word-sz + s2 ;
: T2 ." (" r1 . ." words)" ;
: words +regs 0 s1 last s2 begin
        r2 mem-end < 
        ( T1 | T2 -regs exit )
    again ;

: binary  %10 base ! ;
: decimal #10 base ! ;
: hex     $10 base ! ;
: ? @ . ;

: rshift  0 do 2 / loop ;   \ (( n1 s--n2 )
: lshift  0 do 2*  loop ;   \ (( n1 s--n2 )

: load next-word drop 1- (load) ;
: load-abort 99 state ! ;
: loaded? ( 2drop load-abort ) ;

variable (fg) 2 cells allot
: fg cells (fg) + ;
: marker here 0 fg ! vhere 1 fg ! last 2 fg ! ;
: forget 0 fg @ (here) ! 1 fg @ (vhere) ! 2 fg @ (last) ! ;
: forget-1 last @ (here) ! last word-sz + (last) ! ;
marker
: .ver version 10 /mod (.) '.' emit . ;

." c3 - v" .ver ." - Chris Curl" cr
here mem -   . ." code bytes used, " last here - . ." bytes free." cr
vhere vars - . ." variable bytes used, " vars-end vhere - . ." bytes free."
forget

: benches forget s" benches.c3" (load) ;
: sb forget s" sandbox.c3" (load) ;
: work forget s" work.c3" (load) ;
: ed forget s" editor.c3" (load) ;
marker
