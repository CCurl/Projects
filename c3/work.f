marker

s" strings.f" load
var pad 64 allot
: reload forget s" work.f" load ;
: words-n last swap 1+ 1 do .word tab i 10 mod 0= if cr then word-sz + loop drop ;
: edit s" nvim work.f" system ;
: pw s" pw" system ;
: li s" chrome https://fwut-proc-a.mmm.com:1443/netaccess/loginuser.html" system ;
: yahoo s" chrome http://mail.yahoo.com" system ;
: dev s" gz-server ccc" system ;
: gz-360 ( n-- ) s" gz-sys "    pad str-cpy str-catn system ;
: gz-bw  ( n-- ) s" gz-bw "     pad str-cpy str-catn system ;
: gz-app ( n-- ) s" gz-server " pad str-cpy str-catn s"  a" str-scat system ;
: gz-web ( n-- ) s" gz-server " pad str-cpy str-catn s"  w" str-scat system ;
." words added: " 10 words-n
