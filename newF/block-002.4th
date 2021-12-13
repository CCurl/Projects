// Sandbox

forget

: w ." vhere:" here . ." , vhere:" vhere . ." , last:" last . ;

: rand-mod rand abs swap mod  ; inline
: rand-and rand and       ; inline
: rand-8   rand $007f and ; inline
: rand-1k  rand $03ff and ; inline
: rand-16  rand $ffff and ; inline

// A neuron-id is 8 bits: [type:1][num:7]
// - type: 1: input/output, 0: hidden
: n.type #7 >>          ; inline
: n.num  $7f and        ; inline
: n.dump dup ." {type:" n.type . ." , num:" n.num . ." }" ;

// An input neuron is just a number [0..999] whose value is determined by [num]
// A hidden neuron is just a sum, output is [-999..999]
// An output neuron is just a sum and behavior determined by [sum] and [num]

// The brain is just a collection of neurons
// When activated, it is given the address of the critter it is working with

// A connection is 32 bits
// [from:8][to:8][weight:16]
: c.from #24 >> $ff and ;
: c.to   #16 >> $ff and ;
: c.wt   $ffff and      ; inline
: c.dump dup dup ." {from:" c.from n.dump ." , to:" c.to n.dump ." , wt:" c.wt . ." }";

// a critter is a location and its brain (collection of neurons)
// [x:2][y:2][connections]

8 constant #conns									// The number of connections

// (n a sz--)
: fill over + 1- for dup i c! next drop ;

// World words
130 constant w.szX
40  constant w.szY
w.szX w.szY * constant w.sz
variable theWorld w.sz allot
: (w.at) w.szX * + theWorld + ;							// (x y--a)
: ?world (w.at) c@ ;									// (x y--n)
: >world (w.at) c! ;									// (n x y--)
: w.init 0 theWorld w.sz fill ;
: w.cr bl swap if drop '*' then emit ;
: w.dump 0 w.szY 1- for i 0 w.szX 1- for dup i swap ?world w.cr next drop cr next ;
: w.paint 1 1 goto-xy w.dump 1 1 goto-xy ;

: rand-X w.szX rand-mod ;
: rand-Y w.szY rand-mod ;

// Critter words
#conns 1+ cells constant cr-sz
100 constant #critters
variable critters cr-sz #critters * allot
value critter									// the current critter
value cr-id										// the current critter ID

: cr-x? critter c@ ;
: cr-y? critter 1+ c@ ;
: cr->x critter c! ;
: cr->y critter 1+ c! ;
: cr-id? critter 2 + c@ ;
: cr->id critter 2 + c! ;
: cr->XY 2dup ?world 0=							// (x y--f)
	if 2dup cr->y cr->x cr-id -rot >world 1 leave 
	then 2drop 0 ;
: cr-conns critter cell + ;
: cr-set dup (cr-id) ! 1- cr-sz * critters + (critter) ! ;			// (n--)
: cr-randXY rand-X rand-Y cr->XY if leave then cr-randXY ;
: cr.dump ." { id:" cr-id? . ." , x:" cr-x? . ." , y:" cr-y? . ."  }" ;
: cr.init 1 #critters for i cr-set i cr->id cr-randXY cr.dump cr next ;

w.init cr.init

: rl 2 load ;
