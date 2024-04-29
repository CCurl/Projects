\ Block 111 - Words for work

' WORK loaded?

marker

2 LOAD \ files
3 LOAD \ strings
5 LOAD \ screen

: WORK    6 lexicon ;   WORK
: c3-lex  C3 ;
: rl  forget 6 LOAD ;
: ed  40 (scr-h) ! 6 edit ;
: p pad1 ;
: a 'a' ; INLINE
: e 'e' ; INLINE
: r 'r' ; INLINE
: s 's' ; INLINE
: w 'w' ; INLINE
: pw   " pw"  SYSTEM ;
: pwd  " cd"  SYSTEM ;
: ls   " dir" SYSTEM ;
: nvim " nvim ." SYSTEM ;
: yahoo " chrome http://mail.yahoo.com" SYSTEM ;
: dev  " ccc dev" SYSTEM ;
: lg   " lazygit" SYSTEM ;
: gz     ( N-- )  p " gz-360 "    S-CPY   p S-SCATN   p SYSTEM ;
: gz-bw  ( N-- )  p " gz-bw "     S-CPY   p S-SCATN   p SYSTEM ;
: T0     ( N-- )  p " gz-server " S-CPY   p S-SCATN   p bl S-CATC ;
: gz-server ( N CH-- )  SWAP   T0   p S-SCATC  p SYSTEM ;
: gz-app  ( N-- )  a gz-server ;
: gz-web  ( N-- )  w gz-server ;
: db-vs   " start \code\360DB\DBProject\360ePlatformCore.sln"  SYSTEM ;
: sia-vs  " start \code\SIA\360E.sln"    SYSTEM ;
: c3-vs   " start \code\mine\c3\c3.sln"  SYSTEM ;
: T1    ( S-- )  p " cd \code\" S-CPY  p S-SCAT  p " \ " S-CAT ;
: pull  ( S-- )  DUP green QTYPE white ."  ... "
                   T1 p " && git pull"   S-CAT  p SYSTEM ;
: code  ( S-- )    T1 p " && code ."     S-CAT  p SYSTEM ;
: explorer ( S-- ) T1 p " && explorer ." S-CAT  p SYSTEM ;
: bia  " bia-dw" ;
: db   " 360DB" ;
: bws  " BWServiceDefinitions" ;
: bwp  " BWPluginModules" ;
: ngc  " NgComponents" ;
: sia  " SIA" ;
: ssr  " ssr-db" ;
: svc  " Service" ;
: aed  " SIA\SPA\AuditExpertDashboard" ;
: aew  " SIA\SPA\AuditExpertWorkflow"  ;
: pb   " SIA\SPA\PrebillDashboard" ;
: c3   " mine\c3" ;
: pull-all  0 db sia bws bwp ngc ssr bia c3  BEGIN ?DUP WHILE pull REPEAT ;
 CR ." words added: %n%n" words
