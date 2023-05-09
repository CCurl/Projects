\ Words for work

' gz-web loaded?

load screen.f
load string.f

variable p 100 allot

: words-n last swap 1+ 1 do dup .word tab i 10 mod 0= if cr then word-sz + loop drop ;
: edit s" nvim ." system ;      
: pull s" git pull" system ;      
: pw s" pw" system ;
: li s" chrome https://fwut-proc-a.mmm.com:1443/netaccess/loginuser.html" system ;
: yahoo s" chrome http://mail.yahoo.com" system ;
: dev s" gz-server ccc" system ;
: gz ( n-- )  p s" gz-sys "    s-cpy p s-scatn p system ;
: bw  ( n-- ) p s" gz-bw "     s-cpy p s-scatn p system ;
: app ( n-- ) p s" gz-server " s-cpy p s-scatn p s"  a" p s-cat p system ;
: web ( n-- ) p s" gz-server " s-cpy p s-scatn p s"  w" p s-cat p system ;
." words added: " 12 words-n
