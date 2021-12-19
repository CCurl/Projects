// Genetic evolution

forget

100 load

130 constant w.x
 40 constant w.y
w.x w.y * cells constant w.sz 
variable world w.sz allot
: w.init world dup w.sz + 1- for 0 i c! next ;
: w.randXY rand w.x mod rand w.y mod ;           // (--x y)
: w.xy>o w.y * + cells ;                         // (x y--o)
: w.xy>a w.xy>o world + ;                        // (x y--a)
: w.xy@ w.xy>a @ ;                               // (x y--n)
: w.xy! w.xy>a ! ;                               // (n x y--)
: w.xy.free? w.xy@ 0= ;                          // (--x y)
: w.free.xy w.randXY 2dup w.xy@ 0= 
    if leave then 
    2drop w.free.xy ;

." initializing world ... " w.init cr

100 constant #critters
  8 constant #conns
#conns 1+ cells constant critter-sz

variable critters
#critters critter-sz * allot
value critter

// Neuron stuff
: n.in c@ ;     // (a--b)
: n.out 1+ c@ ; // (a--b)
: n.wt  2+ w@ ; // (a--w)
: n.id/t dup $7F and swap 7 >> ;                                // (n--id type)
: n.dump.n  n.id/t ." { t:" . ." , id:" . ."  }" ;              // (n--)
: n.dump ."   { in: "  dup n.in  n.dump.n                       // (a--)
         ." , out: "   dup n.out n.dump.n
         ." , wt:"    n.wt .
         ."  }" cr ;
: n.dump.all 1 #conns for dup n.dump cell + next drop ;        // (a--)

// Critter stuff
: cr.set 1- critter-sz * critters + (critter) ! ;              // (I--)
: cr.xy@ critter dup c@ swap 1+ c@ ;                           // (--x y)
: cr.xy! critter tuck 1+ c! c! ;                               // (x y--)
: cr->w  critter cr.xy@ w.xy! ;                                // (--)
: cr.xy.rand w.free.xy cr.xy! cr->w ;                          // (--)
: cr.conns critter cell + ;                                    // (--a)
: cr.conns.rand 1 #conns for rand i cells critter + ! next ;   // (--)
: cr.n.in  cells critter + n.in ;                              // (I--n)
: cr.n.out cells critter + n.out ;                             // (I--n)
: cr.n.wt  cells critter + n.wt ;                              // (I--n)
: cr.rand cr.xy.rand cr.conns.rand ;                           // (--)
: cr.dump cr cr.xy@ swap ." { x:" . ." , y: " . ." , neurons: [" cr
    cr.conns n.dump.all
    ." ]}" cr ;                                                // (--)
: cr.color cr.conns @ 7 and 1+ 30 + 40 set-color ;
: cr.show cr.color cr.xy@ goto-xy '*' emit ;

// Critter collection stuff
: cr.all.rand 1 #critters for i cr.set cr.rand    next ;
: cr.all.dump 1 #critters for i cr.set cr.dump cr next ;
: cr.all.show 1 #critters for i cr.set cr.show    next ;

: go w.init cr.all.rand cr.all.dump ;

: rl 200 load ;
