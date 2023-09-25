\ Words for work

' words-n loaded?

load screen.c3
load string.c3
load file.c3

variable p #200 allot

: ?cr r1+ #9 > if cr 0 s1 else tab then ;
: words-n 0 s1 last s2 for r2 .word ?cr r2 word-sz + s2 next ;
: edit " nvim work.c3" system ;      
: pull " git pull" system ;      
: pw " pw" system ;
: pwd " cd" system ;
: ls " dir" system ;
: li " chrome https://fwut-proc-a.mmm.com:1443/netaccess/loginuser.html" system ;
: yahoo " chrome http://mail.yahoo.com" system ;
: dev " gz-server ccc" system ;
: gz   ( n-- )  p " gz-sys "    s-cpy   p s-scatn   p system ;
: gzbw ( n-- )  p " gz-bw "     s-cpy   p s-scatn   p system ;
: app  ( n-- )  p " gz-server " s-cpy   p s-scatn   p "  a" s-cat   p system ;
: web  ( n-- )  p " gz-server " s-cpy   p s-scatn   p "  w" s-cat   p system ;
: db  " start E:\Code\GitRepos\360DB\DBProject\360ePlatformCore.sln"  system ;
: bws " cd E:\Code\GitRepos\BWServiceDefinitions\ && nvim ."  system ;
: cadb " cd E:\Code\GitRepos\SIA\SPA\AuditExpertDashboard\ && nvim ."  system ;
: cawf " cd E:\Code\GitRepos\SIA\SPA\AuditExpertWorkflow\ && nvim ."  system ;
: prebill " cd E:\Code\GitRepos\SIA\SPA\PrebillDashboard\ && nvim ."  system ;
: bwp " explorer E:\Code\GitRepos\BWPluginModule"  system ;
cr ." words added: " 20 words-n
