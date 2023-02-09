: last (last) @ ;
: here (here) @ ;
: vhere (vhere) @ ;
: count dup 1+ swap c@ ; inline
: type 0 do dup c@ emit 1+ loop drop ;
: mem-end (mem) mem-sz + ;

: words 9999 0 do 
        i word-sz * last + dup
        mem-end < 0= 
        if drop unloop exit then
        1+ count type 9 emit
    loop ;

: mil 1000 dup * * ;
: elapsed timer swap - . ;
: bm1 timer swap begin 1- dup while drop elapsed ;
: bm2 timer swap 0 do loop elapsed ;
