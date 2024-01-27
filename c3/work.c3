\ Words for work

' words-n loaded?

marker 

load screen.c3
load string.c3
load file.c3

variable p #200 allot

: ?cr r1+ #9 > if cr 0 s1 else tab then ;
: words-n 0 s1 last s2 for r2 .word ?cr r2 word-sz + s2 next ;
: edit " nvim \c3\work.c3" system ;
: reload forget " work.c3" (load) ;
: pull " git pull" system ;
: pw " pw" system ;
: pwd " cd" system ;
: ls " dir" system ;
: li " chrome https://fwut-proc-a.mmm.com:1443/netaccess/loginuser.html" system ;
: yahoo " chrome http://mail.yahoo.com" system ;
: dev " ccc dev" system ;
: gz   ( n-- )  p " gz-360 "    s-cpy   p s-scatn   p system ;
: bw   ( n-- )  p " gz-bw "     s-cpy   p s-scatn   p system ;
: app  ( n-- )  p " gz-server " s-cpy   p s-scatn   p "  a" s-cat   p system ;
: web  ( n-- )  p " gz-server " s-cpy   p s-scatn   p "  w" s-cat   p system ;
: db  " start E:\Code\360DB\DBProject\360ePlatformCore.sln"  system ;
: c3  " start E:\Code\mine\c3\c3.sln"  system ;
: bws " cd E:\Code\\BWServiceDefinitions\ && nvim ."  system ;
: aed " cd E:\Code\SIA\SPA\AuditExpertDashboard\ && nvim ."  system ;
: aew " cd E:\Code\SIA\SPA\AuditExpertWorkflow\ && nvim ."  system ;
: prebill " cd E:\Code\SIA\SPA\PrebillDashboard\ && nvim ."  system ;
: bwp " explorer E:\Code\BWPluginModule"  system ;
: help edit ;
 cr ." words added: " 21 words-n
