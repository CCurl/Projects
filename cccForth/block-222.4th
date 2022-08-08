// A genetic algorithm

1 load
// 223 load

// Register assignments
// r6: current critter

200 constant maxC
 55 constant maxR
maxC 1+ maxR 1+ * constant world-sz
variable world world-sz allot

500 constant #crits
  1 constant #conns
 10 constant #life-years

: rand-neu rand 255 and ;

: worldClr 0 world world-sz fill-n ;
: T0 ( c r--a ) maxC * + world + ;
: w-set ( n c r--) T0 c! ;
: w-get ( c r--n ) T0 c@ ;
: w-paintR ( r-- ) 1 swap T0
	1 maxC for dup c@ dup if FG '*' else drop bl then emit 1+ next
	drop cr ;
: w-paint ( -- ) 1 1 ->XY 1 maxR for i w-paintR next ;

// connection
// [from:8][to:8][weight:16]
// from: [type:1][id:7] - type: 0=>input, 1=>hidden
// to:   [type:1][id:7] - type: 0=>output, 1=>hidden
// weight: normalized to -400 to 400
4 constant conn-sz

// critter:
// [c:1][r:1][color:1][alive:1][connections:#conns]
#conns conn-sz * 4 +     constant critter-sz
#crits 1+ critter-sz *   constant critters-sz
variable critters #crits 1+ critter-sz * allot

// r6: the current critter
: ->crit ( n--a ) critter-sz * critters + ;
: set-crit ( n--r6 ) ->crit s6 ;
: next-crit	( a--b ) critter-sz + ;
: critters-end critters critters-sz + ;

: normX ( x--x1 ) maxC min 1 max ;
: normY ( y--y1 ) maxR min 1 max ;

: X@ ( crit--n )     c@ ;
: X! ( n crit-- )    >R normX R> c! ;
: Y@ ( crit--n )     1+ c@ ;
: Y! ( n crit-- )    >R normY R> 1+ c! ;
: XY@ ( crit--x y )  DUP >R X@ R> Y@ ;
: CLR@ ( crit--n )   2+ c@ ;
: CLR! ( n crit-- )  2+ c! ;
: Age@ ( crit--n )   3 + c@ ; // 0 == not alive
: Age! ( n crit-- )  3 + c! ;
: Age+ ( crit-- )    dup Age@ dup .if 1+ .then swap Age! ;
: Kill! ( crit-- )   0 swap Age! ;
: Alive? ( crit--f ) Age@ ;
: Dead? ( crit--f )  Age@ 0= ;
: ->conns   ( crit--a )  4 + ;
: conn@     ( a--b )     @ ;
: conn!     ( n a-- )    ! ;
: next-conn ( a--b )     4 + ;

: rand-mod+ ( a b--c )  rand swap mod + ;
: rand-XY   ( -- )      1 maxC rand-mod+ r6 X! 1 maxR rand-mod+ r6 Y! ;
: rand-CLR  ( -- )      31 7 rand-mod+ r6 CLR! ;
: rand-crit ( -- )      rand-XY rand-CLR 1 r6 Age! 
	r6 ->conns 0 #conns for rand-neu over conn! next-conn next drop ;
: rand-crits 0 #crits for i set-crit rand-crit next ;

: unpaint-crit ( -- )  r6 CLR@ FG r6 XY@ ->XY space ;
: paint-crit   ( -- )  r6 CLR@ FG r6 XY@ ->XY '*' emit ;
: paint-crits  ( -- )  1 #crits for i set-crit paint-crit next ;

: up    r6 Y@ 1- r6 Y! ;
: down  r6 Y@ 1+ r6 Y! ;
: left  r6 X@ 1- r6 X! ;
: right r6 X@ 1+ r6 X! ;

: up?    dup 0 = if up    then ;
: down?  dup 1 = if down  then ;
: left?  dup 2 = if left  then ;
: right?     3 = if right then ;

: move rand 3 and up? down? left? right? ;
: work-conn  ( a-- )  ( TODO ) drop move ;
: crit-live  ( a-- )  r6 ->conns 0 #conns for dup work-conn next-conn next drop ;
: crit-life  ( -- )   r6 Dead? .if exit .then 
	unpaint-crit r6 Age+ crit-live paint-crit ;

: dump-crit  ( -- )  r6 CLR@ r6 XY@ swap i ." %d: (%d,%d) %d, " 
	r6 Alive? if ." (alive)" else ." (dead)" then cr ;
: dump-crits ( --)   0 #crits for i set-crit dump-crit next ;
: dump-alive ( --)   0 #crits for i set-crit r6 Alive? if dump-crit then next ;

: next-alive ( -- ) begin 
		r7 next-crit s7
		r7 critters-end > if critters s7 then
		r7 Alive? .if unloop-w exit .then
	again ;
: copy-byte ( n1--n2 ) rand 1000 mod 8 < .if 1 rand 7 AND LSHIFT XOR .then ;
: copy-critter ( -- ) r6 ->conns s8 r7 ->conns s9
	0 critter-sz for 
		r9 c@ copy-byte r8 c!
		i9 i8
	next 
	rand-XY ;
: spawn ( -- ) r6 Dead? if next-alive copy-critter then rand-XY 1 r6 Age! ;
: regen ( -- ) 0 ->crit s7 0 #crits for i set-crit spawn next ;
: kill? ( crit--f ) X@ maxC swap - 10 > ;
: cull       ( -- )   0 #crits for i set-crit r6 kill? if r6 Kill! unpaint-crit then next ;
: one-year   ( -- )   0 #crits for i set-crit crit-life next ;
: one-life   ( -- )   0 #life-years for one-year next ;
: go rand-crits CURSOR-OFF 
	begin CLS one-life cull regen key? until key drop
	0 FG 1 maxR ->XY CURSOR-ON ;
: test CLS CURSOR-OFF rand-crits
	begin one-year key? until key drop 
	0 FG 1 maxR ->XY CURSOR-ON ;
: reload 222 load ;
: ed " notepad block-222.4th" system ;
