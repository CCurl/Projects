// A genetic algorithm

1 load

// Register assignments
// r6: current critter

200 constant maxC
 55 constant maxR
maxC 1+ maxR 1+ * constant world-sz
variable world world-sz allot

// holders for the neuron input values
variable inputs  32 CELLS allot
: >input  ( n--a )   31 AND CELLS inputs + ;
: input@  ( n--x )   >input @ ;
: input!  ( x n-- )  >input ! ;

variable hiddens 32 CELLS allot
: >hidden ( n--a )   31 AND CELLS hiddens + ;
: hidden@ ( n--x )   >hidden @ ;
: hidden! ( x n-- )  >hidden ! ;

variable outputs 32 CELLS allot
: >output ( n--a )   31 AND CELLS outputs + ;
: output@ ( n--x )   >output @ ;
: output! ( x n-- )  >output ! ;

// neurons
// [type:1][unused:2][id:5]
// type: 0=>input/output,  1=>hidden
: rand-neu  ( --neu )       RAND %10011111 AND ;
: n-id-type ( n--id type )  DUP $1F AND SWAP 7 RSHIFT 1 AND ;
: n-id      ( n--id )       n-id-type DROP ;
: n-type    ( n--t )        n-id-type NIP ;
: isHidden? ( n--f)         n-type ;
: n-dump    ( n-- )         n-id-type ." (%d %d)" ;

// connection
// [from:8][to:8][weight:16]
// from, to: [neuron]
// weight: normalized to -400 to 400
  4 constant conn-sz

// neural connections
: rand-conn ( --conn )  rand-neu 24 LSHIFT rand-neu 16 LSHIFT OR RAND $FFFF AND OR ;
: c-input   ( c--neu )  #24 RSHIFT $FF AND ;
: c-output  ( c--neu )  #16 RSHIFT $FF AND ;
: c-weight  ( c--wgt )  $FFFF AND DUP #400 MOD SWAP $8000 AND IF NEGATE THEN ;
: c-dump ( c-- ) dup c-input ." in: " n-dump
	dup c-output ." , out: " n-dump
	c-weight ." , wt: %d" ;

500 constant #crits
  4 constant #conns

variable (years) 10 (years) !
 : #life-years (years) @ ;

// critter:
// [c:1][r:1][color:1][age:1][connections:#conns]
#conns conn-sz * 4 +     constant critter-sz
#crits critter-sz *   constant critters-sz
variable critters #crits critter-sz * allot

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
	r6 ->conns 0 #conns FOR rand-conn over conn! next-conn NEXT DROP ;
: rand-crits 0 #crits FOR I set-crit rand-crit NEXT ;

: unpaint-crit ( -- )  0 FG r6 XY@ ->XY space ;
: paint-crit   ( -- )  r6 CLR@ FG r6 XY@ ->XY '*' emit ;
: paint-crits  ( -- )  1 #crits FOR I set-crit paint-crit NEXT ;

: dump-crit ( -- )   r6 CLR@ r6 XY@ SWAP I ." %d: (%d,%d) %d, " 
	r6 Alive? IF ." (alive)" else ." (dead)" THEN cr 
	r6 ->conns 0 #conns FOR DUP conn@ c-dump next-conn cr NEXT DROP ;
: dump-crits ( --)   0 #crits FOR I set-crit dump-crit NEXT ;
: dump-alive ( --)   0 #crits FOR I set-crit r6 Alive? IF dump-crit THEN NEXT ;

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
//
// critter "live" algorithm: 
// clear all input values (hidden and output neurons)
// foreach connection (process inputs)
// - connection input isHidden not if
// - - derive output value (*weight)
// - - add output value to output node (might be a hidden node)
// foreach connection (process hiddens)
// - connection input isHidden if
// - - derive output value (*weight)
// - - add output value to output node (might be hidden node)
// foreach connection (process outputs)
// - connection input isHidden not if
// - - perform output based on value

: crit-live ( -- )    ( TODO! ) RAND 6 MOD up? down? left? right? ;
: crit-die? ( --f )   RAND 1000 MOD 998 > ;
: crit-wakeUp ( -- )  
	r6 Alive? IF
		unpaint-crit
		r6 Age+
		crit-die? IF r6 Kill! EXIT THEN
		crit-live
		paint-crit
	THEN ;

: first-dead ( --crit ) 
	0 #crits FOR
		I ->crit Dead? IF I ->crit UNLOOP-F EXIT THEN
	NEXT
	0 ->crit ;
: copy-byte ( n1--n2 ) RAND 125 MOD .IF EXIT .THEN 1 RAND 7 AND LSHIFT XOR ;
: reproduce ( -- ) first-dead s9
	r6 ->conns s7  r9 ->conns s8
	0 critter-sz FOR
		r7 C@ copy-byte r8 C!
		i7 i8
	NEXT 
	1 r9 Age! ;
: births  ( -- )  0 #crits FOR I set-crit r6 Alive? IF reproduce THEN NEXT ;
: zombies ( -- )  0 #crits FOR I set-crit r6 Dead?  IF rand-crit THEN NEXT ;
: all-new  0 #crits FOR I set-crit rand-CLR rand-XY 1 r6 Age! NEXT ;
: regen ( -- )  births zombies all-new ;
: kill? ( crit--f )   X@ maxC SWAP - 10 > ;
: cull       ( -- )   0 #crits FOR I set-crit r6 kill? IF r6 Kill! unpaint-crit THEN NEXT ;
: one-year   ( -- )   0 #crits FOR I set-crit crit-wakeUp NEXT ;
: one-life   ( -- )   0 #life-years FOR one-year NEXT ;
: go rand-crits CURSOR-OFF 
	BEGIN CLS one-life cull regen key? UNTIL key DROP
	0 FG 1 maxR ->XY CURSOR-ON ;
: reload 222 load ;
: ed " notepad block-222.4th" system ;
