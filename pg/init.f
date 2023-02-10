: \ 0 >in @ ! ;

: (exit)    1 ; 
: (jmp)     2 ;
: (jmpz)    3 ;
: (jmpnz)   4 ;
: (bitop)  34 ;
: (retop)  35 ;
: (fileop) 36 ;

: last (last) @ ;
: here (here) @ ;
: vhere (vhere) @ ;
: mem-end (mem) mem-sz + ;

: inline 2 last c! ;
: immediate 1 last c! ;
: [ 0 state ! ; immediate
: ] 1 state ! ;
: bye 999 state ! ;
: cells cell * ; inline

: c, here c! here 1+     (here) ! ;
: ,  here !  here cell + (here) ! ;

: allot vhere + (vhere) ! ;
: vc, vhere c! 1 allot ;
: v,  vhere ! cell allot ;

: count dup 1+ swap c@ ; inline
: type 0 do dup c@ emit 1+ loop drop ;

: if (jmpz) c, here 0 , ; immediate
: then here swap !      ; immediate
: exit (exit) c,        ; immediate

: tuck swap over ; inline
: nip swap drop ; inline

: begin here         ; immediate
: while (jmpnz) c, , ; immediate
: until (jmpz)  c, , ; immediate
: again (jmp)   c, , ; immediate

: and [ (bitop) c, 11 c, ] ; inline
: or  [ (bitop) c, 12 c, ] ; inline
: xor [ (bitop) c, 13 c, ] ; inline
: com [ (bitop) c, 14 c, ] ; inline

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

: words last begin
        dup mem-end < 0=  if drop exit then
        dup 1+ count type tab word-sz +
    again ;

: mil 1000 dup * * ;
: elapsed timer swap - . ;
: bm1 timer swap begin 1- dup while drop elapsed ;
: bm2 timer swap 0 do loop elapsed ;
500 mil bm1 
\ bye
