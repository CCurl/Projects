// Genetic evolution

forget

100 load

130 constant w.x
 40 constant w.y
w.x w.y * cells constant w.sz 
variable world w.sz allot
: w.init world dup w.sz + 1- for 0 i c! next ;
: w.randXY rand w.x mod rand w.y mod ;               // (--x y)
: w.xy>o w.y * + cells ;                             // (x y--o)
: w.xy>a w.xy>o world + ;                            // (x y--a)
: w.xy@ w.xy>a @ ;                                   // (x y--n)
: w.xy! w.xy>a ! ;                                   // (n x y--)
: w.xy.free? w.xy@ 0= ;                              // (--x y)
: w.norm.x 0 max w.x min ;                           // (x1--x2)
: w.norm.y 0 max w.y min ;                           // (y1--y2)
: w.norm.xy swap w.norm.x swap w.norm.y ;            // (x1 y1--x2 y2)
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
: cr->w  critter cr.xy@ w.xy! ;                                // (--)
: cr.xy! critter tuck 1+ c! c! cr->w ;                         // (x y--)
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
: cr.unshow 0 cr.xy@ 2dup goto-xy space w.xy! ;
: cr.move 2dup w.xy@ if 2drop leave then                       // (x y--)
    cr.unshow cr.xy! cr.show ;
: cr.mv.n cr.xy@ swap 1- swap cr.move ;
: cr.mv.s cr.xy@ swap 1+ swap cr.move ;
: cr.mv.w cr.xy@ swap 1- swap cr.move ;
: cr.mv.e cr.xy@ swap 1+ swap cr.move ;

: +-1 rand 3 mod 1- ;
: cr.move.rnd cr.xy@ +-1 + swap +-1 + swap w.norm.xy cr.move ;

// Critter collection stuff
: cr.all.rand     1 #critters for i cr.set cr.rand    next ;
: cr.all.dump     1 #critters for i cr.set cr.dump cr next ;
: cr.all.show cls 1 #critters for i cr.set cr.show    next ;
: cr.all.xy       1 #critters for i cr.set cr.xy.rand next ;

: cr.all.mv 1 #critters for i cr.set cr.move.rnd next ;
: cr.mv.n cursor-off 1 for cr.all.mv next cursor-on reset-color ;

: init w.init cr.all.rand cr.all.show 1 w.y goto-xy reset-color ;

: rl 200 load ;
