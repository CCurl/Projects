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

variable (years) 50 (years) !
 : #life-years (years) @ ;

: rand-neu rand 255 and ;

: worldClr 0 world world-sz fill-n ;
: T0 ( c r--a ) maxC * + world + ;
: w-set ( n c r--) T0 c! ;
: w-get ( c r--n ) T0 C@ ;
: w-paintR ( r-- ) 1 SWAP T0
	1 maxC FOR DUP C@ DUP IF FG '*' else DROP bl THEN emit 1+ next
	DROP cr ;
: w-paint ( -- ) 1 1 ->XY 1 maxR FOR I w-paintR NEXT ;

// connection
// [from:8][to:8][weight:16]
// from: [type:1][id:7] - type: 0=>input, 1=>hidden
// to:   [type:1][id:7] - type: 0=>output, 1=>hidden
// weight: normalized to -400 to 400
4 constant conn-sz

// critter:
// [c:1][r:1][color:1][age:1][connections:#conns]
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

: X@ ( crit--n )     C@ ;
: X! ( n crit-- )    SWAP normX SWAP c! ;
: Y@ ( crit--n )     1+ C@ ;
: Y! ( n crit-- )    SWAP normY SWAP 1+ c! ;
: XY@ ( crit--x y )  DUP >R X@ R> Y@ ;
: CLR@ ( crit--n )   2+ C@ ;
: CLR! ( n crit-- )  2+ c! ;
: Age@ ( crit--n )   3 + C@ ;
: Age! ( n crit-- )  3 + c! ;
: Age+ ( crit-- )    DUP Age@ 1+ SWAP Age! ;
: Alive? ( crit--f ) Age@ ;
: Dead? ( crit--f )  Age@ 0= ; // Age 0 => not alive
: Kill! ( crit-- )   0 SWAP Age! ;
: ->conns   ( crit--a )  4 + ;
: conn@     ( a--b )     @ ;
: conn!     ( n a-- )    ! ;
: next-conn ( a--b )     4 + ;

: rand-mod+ ( a b--c )  rand SWAP mod + ;
: rand-XY   ( -- )      1 maxC rand-mod+ r6 X! 1 maxR rand-mod+ r6 Y! ;
: rand-CLR  ( -- )      31 7 rand-mod+ r6 CLR! ;
: rand-crit ( -- )      rand-XY rand-CLR 1 r6 Age! 
	r6 ->conns 0 #conns FOR rand-neu over conn! next-conn NEXT DROP ;
: rand-crits 0 #crits FOR I set-crit rand-crit NEXT ;

: unpaint-crit ( -- )  0 FG r6 XY@ ->XY space ;
: paint-crit   ( -- )  r6 CLR@ FG r6 XY@ ->XY '*' emit ;
: paint-crits  ( -- )  1 #crits FOR I set-crit paint-crit NEXT ;

: up    r6 Y@ 1- r6 Y! ;
: down  r6 Y@ 1+ r6 Y! ;
: left  r6 X@ 1- r6 X! ;
: right r6 X@ 1+ r6 X! ;

: up?    DUP 0 = IF up    THEN ;
: down?  DUP 1 = IF down  THEN ;
: left?  DUP 2 = IF left  THEN ;
: right?     3 > IF right THEN ;

// NOTE: r5 is the current connection
//       r6 is the current critter
: get-input ( --n ) ( TODO! ) RAND 6 MOD ;
: do-output ( n-- ) ( TODO! ) up? down? left? right? ;
: work-conn  ( -- )  get-input do-output ;
: crit-die? ( --f )  RAND 1000 MOD 995 > IF r6 Kill! TRUE ELSE FALSE THEN ;
: crit-live ( -- )   r6 ->conns s5 0 #conns FOR work-conn r5 next-conn s5 NEXT ;
: crit-wakeUp ( -- ) r6 Alive? IF
		unpaint-crit r6 Age+
		crit-die? IF EXIT THEN
		crit-live
		paint-crit
	THEN ;

: dump-crit ( -- )   r6 CLR@ r6 XY@ SWAP I ." %d: (%d,%d) %d, " 
	r6 Alive? IF ." (alive)" else ." (dead)" THEN cr ;
: dump-crits ( --)   0 #crits FOR I set-crit dump-crit NEXT ;
: dump-alive ( --)   0 #crits FOR I set-crit r6 Alive? IF dump-crit THEN NEXT ;

: next-alive ( -- ) #crits s1 BEGIN 
		r7 next-crit s7
		r7 critters-end > IF critters s7 THEN
		r7 Alive? IF UNLOOP-W EXIT THEN
	d1 r1 WHILE ;
: copy-byte ( n1--n2 ) RAND 1000 MOD 8 < IF 1 RAND 7 AND LSHIFT XOR THEN ;
: copy-critter ( -- ) r6 ->conns s8 r7 ->conns s9
	0 critter-sz FOR
		r9 C@ copy-byte r8 C!
		i9 i8
	NEXT
	rand-CLR ;
: spawn ( -- ) r6 Dead? IF next-alive copy-critter THEN rand-XY 1 r6 Age! ;
: regen ( -- ) 0 ->crit s7 0 #crits FOR I set-crit spawn NEXT ;
: kill? ( crit--f ) X@ maxC SWAP - 10 > ;
: cull       ( -- )   0 #crits FOR I set-crit r6 kill? IF r6 Kill! unpaint-crit THEN NEXT ;
: one-year   ( -- )   0 #crits FOR I set-crit crit-wakeUp NEXT ;
: one-life   ( -- )   0 #life-years FOR one-year NEXT ;
: go rand-crits CURSOR-OFF 
	BEGIN CLS one-life cull regen key? UNTIL key DROP
	0 FG 1 maxR ->XY CURSOR-ON ;
: reload 222 load ;
: ed " notepad block-222.4th" system ;
