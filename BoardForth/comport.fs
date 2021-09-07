all definitions
: com-port 3 ; com-port definitions
variable (comport)   CELL allot 0 (comport) !
variable (comhandle) CELL allot 0 (comhandle) !
: comhandle (comhandle) @ ;
: comport (comport) @ ;
: comopen ( n -- ) dup (comport) ! com-open (comhandle) ! ;
: comclose ( -- ) comhandle com-close 0 (comhandle) ! ;
: comread ( -- c ) comhandle com-read ;
: comwrite ( c -- ) comhandle com-write 0= if ." -err-" then ;
: comall ( -- ) begin comread dup if emit 1 then while ;
: comstring ( a n -- ) over + for i c@ comwrite next ;
: comcr ( -- ) 13 comwrite ;
: comline ( cs -- )   count comstring comcr comall ;
: comlinez ( zs -- ) zcount comstring comcr comall ;
