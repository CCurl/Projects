\ File handling words

' fopen-rt loaded?

: fopen-rt ( fn--fh ) s" rt" fopen ;
: fopen-rb ( fn--fh ) s" rb" fopen ;
: fopen-wt ( fn--fh ) s" wt" fopen ;
: fopen-wb ( fn--fh ) s" wb" fopen ;
: fopen-rw ( fn--fh ) s" r+b" fopen ;

variable T0
: sv-output (output_fp) @ T0 ! ;
: rs-output T0 @ (output_fp) ! ;
: ch-output sv-output (output_fp) ! ;
: ->stdout 0 (output_fp) ! ;

variable T0
: fgetc ( fh--c f )  T0 swap 1 swap fread T0 c@ swap ;

: fgets ( a fh--len eof )
    +regs   s2 s1   0 s4
    begin 
        r2 fgetc   swap s3
        0= if 0 r1 c!   r4 dup 0= -regs exit then
        r3 10 = if  0 r1 c!  r4 0 -regs exit then
        r3 13 <> if r3 r1+ c!   i4 then
    again ;

: fputc   ( c fh-- )     ch-output emit rs-output ;
: fputs   ( str fh-- )   ch-output count type rs-output ;
: fputs-z ( strz fh-- )  ch-output count typez rs-output ;


: test s" file.c3" fopen-rt s1
	begin vhere r1 fgets nip
		if r1 fclose drop exit else vhere typez then
	again ;