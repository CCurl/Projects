\ File handling words

' fopen-rt loaded?

: fopen-rt s" rt" fopen ;
: fopen-wt s" wt" fopen ;
: fopen-rw s" r+b" fopen ;
: ->stdout 0 (output_fp) ! ;

variable T0
: file-getc ( fh--c f )
    >r T0 1 r> fread T0 c@ swap ;

: file-gets ( a fh--len eof )
    +regs   s2 s1   0 s4
    begin 
        r2 file-getc   swap s3
        0= if 0 r1 c!   r4 1 -regs exit then
        r3 10 = if  0 r1 c!  r4 0 -regs exit then
        r3 13 = if  0 s3 then
        r3  9 = if 32 s3 then
        r3 if r3 r1 c!   i1 i4 then
    again ;

: file-putc ( c fh-- )
    (output_fp) ! emit ->stdout ;

: file-puts ( str fh-- )
    (output_fp) count type ->stdout ;
