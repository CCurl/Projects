// Sandbox

forget

: w ." vhere:" here . ." , vhere:" vhere . ." , last:" last . ;

: rand-mod rand swap mod  ; inline
: rand-and rand and       ; inline
: rand-1k  rand $03ff and ; inline
: rand-16  rand $ffff and ; inline

cr ."  100 mod:" #100 rand-mod .
cr ."  $FF and:" $0fF rand-and .
cr ."  1k     :" rand-1k .

// A neuron-id is 8 bits: [type:1][num:7]
// - type: 1: input/output, 0: hidden
: n.type #7 >>          ; inline
: n.num  $7f and        ; inline
: n.dump dup ." {type:" n.type . ." , num:" n.num . ." }" ;

// An input neuron is just a number [0..999] whose value is determined by [num]
// A hidden neuron is just a sum, output is [-999..999]
// An output neuron is just a sum and behavior determined by [sum] and [num]

// The brain is a collection of neurons
// When activated, it is given the address of the critter it is working with

// A connection is 32 bits
// [from:8][to:8][weight:16]
: c.from #24 >> $ff and ;
: c.to   #16 >> $ff and ;
: c.wt   $ffff and      ; inline
: c.dump dup dup ." {from:" c.from n.dump ." , to:" c.to n.dump ." , wt:" c.wt . ." }";

// a critter is 
// [x:2][y:2][connections:32]
: rl 2 load ;
