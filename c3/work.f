load strings.f

var pad 64 allot
\ : reload " work" SLOAD ;
: edit s" nvim ." system ;
: ?? dup count type cr ;
: pw s" pw" system ;
: li s" chrome https://fwut-proc-a.mmm.com:1443/netaccess/loginuser.html" system ;
: yahoo s" chrome http://mail.yahoo.com" system ;
: dev s" gz-server ccc" ?? system ;
: gz-360 ( n-- ) s" gz-sys "    pad str-cpy str-catn ?? system ;
: gz-bw  ( n-- ) s" gz-bw "     pad str-cpy str-catn ?? system ;
: gz-app ( n-- ) s" gz-server " pad str-cpy str-catn s"  a" str-scat ?? system ;
: gz-web ( n-- ) s" gz-server " pad str-cpy str-catn s"  w" str-scat ?? system ;
." words added."
