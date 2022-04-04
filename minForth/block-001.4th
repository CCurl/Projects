: x1 3 s1 begin r1 dup . 1+ s1 r1 50 < while ." -out1%n" ;
: x2 3 s1 begin r1 dup . 1+ s1 r1 100 = until ." -out2%n" ;
: x3 3 s1 begin r1 22 = if break then r1 dup . 1+ s1 again ." -out3%n" ;
