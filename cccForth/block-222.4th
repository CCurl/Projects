// A genetic algorithm

1 load

// Register assignments
// r6: current critter

VARIABLE (max-c)
: max-c  (max-c) @ ;
: max-c! (max-c) ! ;
300 max-c!

VARIABLE (max-r)
: max-r  (max-r) @ ;
: max-r! (max-r) ! ;
100 max-r!

max-c 1+ max-r 1+ * constant world-sz
VARIABLE world world-sz allot
150 max-c! 50 max-r!

// holders for the neuron input values
VARIABLE inputs  32 CELLS allot
: >input  ( n--a )   31 AND CELLS inputs + ;
: input@  ( n--x )   >input @ ;
: input!  ( x n-- )  >input ! ;

VARIABLE hiddens 32 CELLS allot
: >hidden ( n--a )   31 AND CELLS hiddens + ;
: hidden@ ( n--x )   >hidden @ ;
: hidden! ( x n-- )  >hidden ! ;

VARIABLE outputs 32 CELLS allot
: >output ( n--a )   31 AND CELLS outputs + ;
: output@ ( n--x )   >output @ ;
: output! ( x n-- )  >output ! ;

// neurons
// [type:1][unused:2][id:5]
// type: 0=>input/output,  1=>hidden
: rand-neu  ( --neu )       RAND %10011111 AND ;
: n-id-type ( n--id type )  DUP $1F AND SWAP $80 AND ;
: n-id      ( n--id )       n-id-type DROP ;
: n-type    ( n--t )        n-id-type NIP ;
: isHidden? ( n--f)         n-type ;
: n-dump    ( n-- )         n-id-type 0= 0= ." (%d %d)" ;

// connection
// [from:8][to:8][weight:16]
// from, to: [neuron]
// weight: normalized to -400 to 400
CELL constant conn-sz

// neural connections
: rand-conn ( --conn )  rand-neu 24 LSHIFT rand-neu 16 LSHIFT OR RAND $FFFF AND OR ;
: c-input   ( c--neu )  #24 RSHIFT $FF AND ;
: c-output  ( c--neu )  #16 RSHIFT $FF AND ;
: c-weight  ( c--wgt )  $FFFF AND DUP #400 MOD SWAP $8000 AND IF NEGATE THEN ;
: c-dump ( c-- ) dup c-input ." in: " n-dump
	dup c-output ." , out: " n-dump
	c-weight ." , wt: %d" ;

VARIABLE (#crits)
: #crits  (#crits) @ ;
: #crits! (#crits) ! ;
500 #crits!

VARIABLE (#conns)
: #conns  (#conns) @ ;
: #conns! (#conns) ! ;
4 #conns!

VARIABLE (years) 20 (years) !
: #years (years) @ ;
: years! (years) ! ;

// critter:
// [c:1][r:1][color:1][age:1][connections:#conns]
#conns conn-sz * CELL +     constant critter-sz
#crits critter-sz *   constant critters-sz
VARIABLE critters #crits critter-sz * allot

// r6: the current critter
: ->crit ( n--a ) critter-sz * critters + ;
: set-crit ( n--r6 ) ->crit s6 ;
: next-crit	( a--b ) critter-sz + ;
: critters-end critters critters-sz + ;

: normX ( x--x1 ) max-c min 1 max ;
: normY ( y--y1 ) max-r min 1 max ;

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
: ->conns   ( crit--a )  CELL + ;
: conn@     ( a--b )     @ ;
: conn!     ( n a-- )    ! ;
: next-conn ( a--b )     CELL + ;

: rand-mod+ ( a b--c )  rand SWAP mod + ;
: rand-XY   ( -- )      1 max-c rand-mod+ r6 X! 1 max-r rand-mod+ r6 Y! ;
: rand-CLR  ( -- )      31 7 rand-mod+ r6 CLR! ;
: rand-crit ( -- )      rand-XY rand-CLR 1 r6 Age! 
	r6 ->conns #conns 0 DO rand-conn over conn! next-conn LOOP DROP ;
: rand-crits   #crits 0 DO I set-crit rand-crit LOOP ;

VARIABLE (paint) TRUE (paint) !
: paint? (paint) @ ;
: paint-on   TRUE  (paint) ! ;
: paint-off  FALSE (paint) ! ;
: unpaint-crit ( -- )  paint? IF 0 FG r6 XY@ ->XY space THEN ;
: paint-crit   ( -- )  paint? IF r6 CLR@ FG r6 XY@ ->XY '*' emit THEN ;
: paint-crits  ( -- )  #crits 0 DO I set-crit paint-crit LOOP ;

: dump-crit ( -- )   r6 CLR@ r6 XY@ SWAP I ." %d: (%d,%d) %d, " 
	r6 Alive? IF ." (alive)" else ." (dead)" THEN cr 
	r6 ->conns #conns 0 DO DUP conn@ c-dump next-conn cr LOOP DROP ;
: dump-crits ( --)   #crits 0 DO I set-crit dump-crit LOOP ;
: dump-alive ( --)   #crits 0 DO I set-crit r6 Alive? IF dump-crit THEN LOOP ;

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
// A crude approximation of tanh(x)
: tanh ( n1--n2 ) dup 82 < if 85 * 100 / else 25 * 100 / 49 + then 99 min -99 max ;
: tanh-graph ( -- )  300 0 DO i tanh 100 swap - i swap ->XY '*' emit LOOP ;

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
	#crits 0 DO I ->crit Dead? IF I ->crit UNLOOP EXIT THEN LOOP
	0 ->crit ;
: copy-byte ( n1--n2 ) RAND 125 MOD .IF EXIT .THEN 1 RAND 7 AND LSHIFT XOR ;
: reproduce ( -- ) first-dead s9
	r6 ->conns s7  r9 ->conns s8
	critter-sz 0 DO r7 C@ copy-byte r8 C! i7 i8 LOOP
	1 r9 Age! ;
: num-dead   ( --n )  0 #crits 0 DO I ->crit Dead?  IF 1+ THEN LOOP ;
: num-alive  ( --n )  0 #crits 0 DO I ->crit Alive? IF 1+ THEN LOOP ;
: babies  ( -- )  #crits 0 DO I set-crit r6 Alive? IF reproduce THEN LOOP ;
: zombies ( -- )  #crits 0 DO I set-crit r6 Dead?  IF rand-crit THEN LOOP ;
: all-new ( -- )  #crits 0 DO I set-crit rand-CLR rand-XY 1 r6 Age!  LOOP ;
: regen ( -- )  babies zombies all-new ;
: kill? ( crit--f )   X@ max-c SWAP - 10 > ;
: cull       ( -- )   #crits 0 DO I set-crit r6 kill? IF r6 Kill! unpaint-crit THEN LOOP ;
: one-year   ( -- )   #crits 0 DO I set-crit crit-wakeUp LOOP ;
: one-life   ( -- )   #years 0 DO one-year LOOP ;
VARIABLE gens 0 gens !
: .stats num-dead num-alive gens @ ." gen %d: alive: %d, dead: %d%n" ;
: go rand-crits 0 gens ! BEGIN
		paint? IF CURSOR-OFF CLS THEN
		1 gens +! one-life cull 
		paint? NOT IF .stats THEN
		regen key? 
	UNTIL key DROP
	paint? IF 0 FG 1 max-r ->XY CURSOR-ON THEN  ;
: reload 222 load ;