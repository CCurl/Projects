\ Words for work

' words-n loaded?

load screen.c3
load string.c3

variable p 100 allot

: ?cr r1+ 9 > if cr 0 s1 else tab then ;
: words-n 0 s1 last s2 for r2 .word ?cr r2 word-sz + s2 next ;
: edit s" nvim work.c3" system ;      
: pull s" git pull" system ;      
: pw s" pw" system ;
: li s" chrome https://fwut-proc-a.mmm.com:1443/netaccess/loginuser.html" system ;
: yahoo s" chrome http://mail.yahoo.com" system ;
: dev s" gz-server ccc" system ;
: gz   ( n-- )  p s" gz-sys "    s-cpy   p s-scatn   p system ;
: gzbw ( n-- )  p s" gz-bw "     s-cpy   p s-scatn   p system ;
: app  ( n-- )  p s" gz-server " s-cpy   p s-scatn   p s"  a" s-cat   p system ;
: web  ( n-- )  p s" gz-server " s-cpy   p s-scatn   p s"  w" s-cat   p system ;
: db  s" start E:\Code\GitRepos\360DB\DBProject\360ePlatformCore.sln"  system ;
: bws s" cd E:\Code\GitRepos\BWServiceDefinitions\ && nvim ."  system ;
: prebill s" cd E:\Code\GitRepos\SIA\SPA\prebilldashboard\ && nvim ."  system ;
: bwp s" explorer E:\Code\GitRepos\BWPluginModules"  system ;
cr ." words added: " 15 words-n
