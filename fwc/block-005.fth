\ Block 5 - Calorie counter

50 cells var cals
val ci   (val) (ci)

: ci! ( n-- ) (ci) ! ;
: >ci? ( n--n' f ) cell+ dup ci > ;
: +ci ( --n ) ci cell+ dup ci! ;
: !ci ( n-- ) ci ! ;
: +cals ( n-- ) +ci ! ;
: .cals ( --n ) cals begin dup @ . >ci? until drop ;
: +x ( n-- ) dup . x@ + x! ;
: tot-cals ( --n ) 0 +L1 cals cell+ begin dup @ +x >ci? until drop x@ -L ;
: .tot ( -- ) tot-cals ." = " . ." calories" ;
: 0cals ( -- ) cals ci!  0 ci ! ;
: rl5 forget 5 load ;

0cals

: +chicken    200 + ;
: +mixed-vegs  25 + ;
: +stock       15 + ;
: +olive-oil  250 + ;
: +rice       100 + ;
: +vinegar      5 + ;

: coffee  100 +cals ;
: oatmeal 400 +cals ;
: nuts    200 +cals ;
: dinner 0 +chicken +mixed-vegs +rice +stock +olive-oil +vinegar +cals ;
: apple   100 +cals ;
: orange   75 +cals ;
: banana  125 +cals ;

: daily 0cals coffee oatmeal banana coffee nuts dinner apple coffee .tot ;
