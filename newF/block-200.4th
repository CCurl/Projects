// Genetic evolution

forget

125 constant world.x
 50 constant world.y
variable world world.x world.y * cells allot

100 constant #critters
  8 constant #conns
#conns 1+ cells constant critter-sz

variable critters
#critters critter-sz * allot
value critter

// Nuuron stuff
: n.in c@ ;     // (a--b)
: n.out 1+ c@ ; // (a--b)
: n.wt  2+ w@ ; // (a--w)
: n.id/t dup $7F and swap 7 >> ;                                // (n--id type)
: n.dump.n  n.id/t ." { t:" . ." , id:" . ."  }" ;               // (n--)
: n.dump ."   { in: "  dup n.in  n.dump.n                       // (a--)
         ." , out: "   dup n.out n.dump.n
         ." , wt:"    n.wt .
         ."  }" cr ;
: n.dump.all 1 #conns for dup n.dump cell + next drop ;         // (a--)

// Critter stuff
: cr.set 1- critter-sz * critters + (critter) ! ;               // (I--)
: cr.xy critter dup c@ swap 1+ c@ ;                             // (--x y)
: cr.set.xy critter 1+ c! critter c! ;                          // (x y--)
: cr.xy? cr.xy world.y mod swap world.x mod swap cr.set.xy ;
: cr.conns critter cell + ;                                     // (--a)
: cr.n.in  cells critter + n.in ;                               // (I--n)
: cr.n.out cells critter + n.out ;                              // (I--n)
: cr.n.wt  cells critter + n.wt ;                               // (I--n)
: cr.rand critter 0 #conns for rand over ! cell + next drop ;
: cr.dump cr cr.xy swap ." { x:" . ." , y: " . ." , neurons: [" cr
    cr.conns n.dump.all
    ." ]}" cr ;                                                 // (--)

// Critter collection stuff
: cr.all.xy?  0 #critters 1- for i cr.set cr.xy?     next ;
: cr.all.rand 0 #critters 1- for i cr.set cr.rand    next ;
: cr.all.dump 0 #critters 1- for i cr.set cr.dump cr next ;

: go cr.all.rand cr.all.xy? cr.all.dump ;

: rl 200 load ;
