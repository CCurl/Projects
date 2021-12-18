// Genetic evolution

forget

100 constant #critters
  8 constant #conns
#conns 1+ cells constant critter-sz
variable critters
#critters critter-sz * allot
value critter

: n.in c@ ;     // (a--b)
: n.out 1+ c@ ; // (a--b)
: n.wt  2+ w@ ; // (a--w)
: n.id/t dup $7F and swap 7 >> ;                // (n--id type)
: n.dump.n  n.id/t ." { t:" . ." , id:" . ." }" ;  // (n--)
: n.dump                                        // (a--)
    ."   { in: "  dup n.in  n.dump.n
    ." , out: " dup n.out n.dump.n
    ." , wt: " n.wt .
    ." }";
: n.dump.all 1 #conns for dup n.dump cell + next drop ; // (a--)
: cr.set 1- critter-sz * critters + (critter) ! ; // (I--)
: cr.rand critter 0 #conns for rand over ! cell + next drop ;
: cr.all.rand 1 #critters for i cr.set cr.rand next ;
: cr.x critter c@ ;                           // (--x)
: cr.y critter 1+ c@ ;                        // (--y)
: cr.conns critter cell + ;                   // (--a)
: cr.n.in  cells critter + n.in ;             // (I--n)
: cr.n.out cells critter + n.out ;            // (I--n)
: cr.n.wt  cells critter + n.wt ;             // (I--n)

: cr.dump cr ." { x:" cr.x . ." , y: " cr.y . ." , neurons: [" cr
    cr.conns 1 #conns for dup n.dump cr cell + next  drop
    ." ]}" cr ; // (--)
: cr.dump.all 0 #critters 1- for i cr.set cr.dump cr next ;
: rl 200 load ;
