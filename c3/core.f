: here (here) @ ;
: c, here c! (here) ++ ;
: , here ! here cell + (here) ! ;

: vhere  (vhere) @ ;
: allot  vhere + (vhere) ! ;
: vc, vhere c! (vhere) ++ ;
: v,  vhere ! cell allot ;

: last (last) @ ;
: immediate 1 last cell + c! ;
: inline 2 last cell + c! ;

: \ 0 >in @ ! ; immediate
: [ 0 state ! ; immediate
: ] 1 state ! ;
: bye 999 state ! ;
: cells cell * ; inline

: const  create (lit4) c, , (exit) c, ;
: variable  vhere const cell allot ;
: val  vhere const ;
: (val)  here 1- cell - const ;

: does>  r> last ! ;
: :noname  here 1 state ! ;
: exec  >r ;

: if    (jmpz) c, here 0 , ; immediate
: else  (jmp) c, here swap 0 , here swap ! ; immediate
: then  here swap ! ; immediate
: exit  (exit) c,   ; immediate

: begin  here         ; immediate
: while  (jmpnz) c, , ; immediate
: until  (jmpz)  c, , ; immediate
: again  (jmp)   c, , ; immediate

: tuck  swap over ; inline
: nip   swap drop ; inline
: 2dup  over over ; inline
: 2drop drop drop ; inline
: ?dup  dup if dup then ;

: +!  tuck @ + swap ! ; inline
: c++ dup @ 1+ swap ! ;
: c-- dup @ 1- swap ! ;
: 2*  dup + ; inline
: 2+  1+ 1+ ; inline
: <=  > 0= ; inline
: >=  < 0= ; inline
: <>  = 0= ; inline

: rdrop r> drop ; inline
: rot   >r swap r> swap ;
: -rot  swap >r swap r> ;

: ( begin 
        >in @ c@ dup 0= if drop exit then
        >in ++ ')' = if exit then
    again ; immediate

: bl  #32 ; inline
: tab  #9 emit ; inline
: cr  #13 emit #10 emit ; inline
: space bl emit ; inline

: negate  com 1+ ; inline
: abs  dup 0 < if negate then ;
: min  over over > if swap then drop ;
: max  over over < if swap then drop ;

: i  (i) @ ;
: +i (i) +! ;
: unloop (lsp) @ 3 - (lsp) ! ;

: /   /mod nip  ; inline
: mod /mod drop ; inline

variable (neg)
: #digit '0' + dup '9' > if 7 + then ;
: <# 0 swap dup 0 < (neg) ! abs ;    \ ( n1 -- 0 n2 )
: # base @ /mod swap #digit swap ;   \ ( u1 -- c u2 )
: #S begin # dup while ;             \ ( u1 -- u2 )
: #> drop (neg) @ if '-' then ;
: #P begin emit dup while drop ;     \ ( 0 ... n 0 -- )
: (.) <# #S #> #P ;
: . (.) space ;

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth ?dup if
        0 do (stk) i 1+ cells + @ . loop 
    then ')' emit ;

: count ( str--a n ) dup 1+ swap c@ ; inline
: type  ( a n-- ) ?dup if 0 do dup c@ emit 1+ loop then drop ;
: typez ( a-- ) dup c@ dup 0= if 2drop exit then emit 1+ typez ;

: T0 r8 c! i8 ;
: T1 r9 c@ 1+ r9 c! ;
: T2 ( --str end ) +regs vhere dup s8 s9 0 T0
    begin >in @ c@ s1 r1 if >in ++ then
        r1 0= r1 '"' = or
        if 0 T0 r9 r8 -regs exit then
        r1 T0 T1
    again ;

: s" ( --str ) T2 state @ 0= if drop exit then (vhere) ! (lit4) c, , ; immediate

: ." ( -- ) T2 state @ 0= if drop count type exit then
    (vhere) ! (lit4) c, ,
    (call) c, [ (lit4) c, ' count drop drop , ] ,
    (call) c, [ (lit4) c, ' type  drop drop , ] , ;  immediate

: .word dup cell + 1+ count type ;
: words r1 0 s1 last begin
        dup mem-end < if 
            i1 r1 #11 mod 0= if cr then
            .word tab word-sz +
        else ." (" r1 . ." words)" s1 exit
        then
    again ;

: binary  %10 base ! ;
: decimal #10 base ! ;
: hex     $10 base ! ;
: ? @ . ;

: rshift ( n1 s--n2 ) 0 do 2 / loop ;
: lshift ( n1 s--n2 ) 0 do 2* loop ;

: load next-word drop 1- (load) ;
: load-abort 99 state ! ;
: loaded? if 2drop load-abort then ;
: fopen-rt s" rt" fopen ;
: fopen-wt s" wt" fopen ;
: fopen-rw s" r+b" fopen ;
: ->stdout 0 (output_fp) ! ;

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

: work forget s" work.f" (load) ;
: benches forget s" benches.f" (load) ;
: sb forget s" sandbox.f" (load) ;
marker
