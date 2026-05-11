: elapsed  ( --n ) timer x@ - 1000 / ;
: .elapsed ( -- )  5 5 ->cr ." --> " elapsed . ." <-- " ;
: looper   ( -- )  250 ms .elapsed key? 0= if looper exit then key drop ;
: go       ( -- )  timer x! cls cursor-off looper cursor-on cr cr ;
