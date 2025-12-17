(( code: 65536 cells, then vars ))
65536 cell * memory + (vha) ! (ha) @ (la) @
: here  (ha)  @ ;
: vhere (vha) @ ;
: last  (la)  @ ;
: ->code ( n--a ) cell * memory + ;
: , ( n-- ) here ->code ! 1 (ha) +! ;
: allot ( n-- ) (vha) +! ;
: const ( n-- ) addword lit, (exit) , ;
: var   ( n-- ) vhere const allot ;

(( these are used by "rb" ))
const -la-    const -ha-    vhere const -vha-

: immediate 1 last cell + c! ; immediate
: inline    2 last cell + c! ; immediate

: begin here ; immediate
: while  (jmpnz)  , , ; immediate
: -while (njmpnz) , , ; immediate
:  until (jmpz)   , , ; immediate
: -until (njmpz)  , , ; immediate
: again  (jmp)    , , ; immediate

: if   (jmpz)   , here 0 , ; immediate
: -if  (njmpz)  , here 0 , ; immediate
: if0  (jmpnz)  , here 0 , ; immediate
: -if0 (njmpnz) , here 0 , ; immediate
: then  here swap ->code ! ; immediate

: c@a   a@  c@    ; inline
: c@a+  a@+ c@    ; inline
: c!b+  b@+ c!    ; inline
: c!b-  b@- c!    ; inline
: bdrop b> drop   ; inline

( quote subroutine )
: compiling? ( --f ) state @ 2 = ;
: t4 ( --a ) vhere dup >b  >in @ 1+ >a
   begin
      c@a '"' = if
         0 c!b+  a> 1+ >in !
         compiling? if0 bdrop exit then
         b> (vha) ! lit, exit
      then c@a+ c!b+
   again ;

: z"  t4 ; immediate
: ."  t4 compiling? if (ztype) , exit then ztype ; immediate

( number format / print )
: negate com 1+ ;
: #neg 0 >a dup 0 < if negate 1 a! then ;
: <#   ( n--n' ) #neg last 32 - >b 0 c!b- ;
: hold ( c--n )  c!b- ;
: #n   ( n-- )   '0' + dup '9' > if 7 + then hold ;
: #.   ( -- )    '.' hold ;
: #    ( n--n' ) base @ /mod swap #n ;
: #s   ( n-- )   begin # -while drop ;
: #>   ( --a )   a> if '-' hold then b> 1+ ;
: (.) ( n-- ) <# #s #> ztype ;
: .   ( n-- ) (.) ( no ret here )
: space 32 emit ;
: spaces ( n-- ) for 32 emit next ;
: cr 13 emit 10 emit ;
: abs ( n--m )   dup 0 < if negate then ;
: mod ( a b--m ) /mod drop ;

( ANSI escape codes )
: csi   ( -- )    27 emit '[' emit ;
: ->cr  ( c r-- ) csi (.) ';' emit (.) 'H' emit ;
: cls   ( -- )    csi ." 2J" 1 dup ->cr ;
: fg    ( n-- )   csi ." 38;5;" (.) 'm' emit ;

(( Random number generator ))
cell var seed
: random ( --n )
    seed @ -if0 drop timer then
    dup 8192 * xor
    dup 131072 / xor
    dup 32 * xor
    dup seed ! ;

: rand-max ( max--n ) random abs swap mod ;

( Tree )
: star '*' emit ;
: stars for '*' emit next ;

5 var colors
: color ( n--c ) colors + c@ ;
colors b!
  0 c!b+ (( black ))
 40 c!b+ (( green ))
203 c!b+ (( red ))
226 c!b+ (( yellow ))
255 c!b+ (( white ))

: t0 a@ i - spaces star ;
: t1 i if i 2 * 1- spaces star then ;
: t2 a@ b@ - spaces b@ 1- stars 3 spaces b@ 1- stars cr ;
: t3 a@ 2 - spaces star 3 spaces star cr ;
: t4 a@ 2 - spaces 5 stars cr ;
: body a! b! b@ for t0 t1 cr next ;
: stump t2 t3 t3 t4 ;
: twinkle ( TODO ) ;
: tree cls body stump twinkle ;

25 35 tree
