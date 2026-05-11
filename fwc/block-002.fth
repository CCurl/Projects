\ see <x>
: @x ( --n ) x@ @ ;
: @z ( --n ) z@ @ ;
: cell+ cell + ;
: .hex/dec ( n-- ) dup '$' emit .hex ." /#" .dec ;
: .prim? ( xt--f ) dup 46 < if ." primitive " .hex/dec 1 exit then drop 0 ;
: t0 ( n-- ) ." lit " $3fffffff and .hex/dec ;
: .lit? ( b--f ) y@ $3fffffff > if y@ t0 1 exit then 0 ;
: find-xt ( xt--de 1 | 0 ) x@ >r last x!
    begin
        x@ dict-end < if0 r> x! drop 0 exit then
        @x over = if drop x@ 1 r> x! exit then
        x@ de-sz + x!
    again ;
: next-xt ( de--xt ) de-sz - dup last < if drop last then @ ;
: .lit-jmp? ( y@-- ) y@ (lit) (jmpnz) btwi if space x@+ code@ .hex/dec then ;
: t2 ( x@-- ) cr x@ .hex ." : " x@+ code@ dup .hex y!
    space .lit? if exit then
    y@ find-xt if 4 spaces .word then .lit-jmp? ;
: see-range ( f t-- ) 2dup .hex .hex z! x! begin x@ z@ >= if exit then t2 again ;
: see find -if0 drop ." -not found-" exit then
    x!  @x  .prim? if exit then
    x@ .hex ':' emit space x@ .word
    @x  x@ next-xt see-range ;
