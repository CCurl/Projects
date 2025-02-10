( Block 5 - Screen )

find colors loaded?

cur-on   (--)     ." %e[?25h" ;
cur-off  (--)     ." %e[?25l" ;
cur-block(--)     ." %e[2 q" ;
cur-bar  (--)     ." %e[5 q" ;
->cr     (r c--)  ." %e[%d;%dH" ;
->rc     (c r--)  swap ->cr ;
cls      (--)     ." %e[2J" 1 dup ->rc ;
clr-eol  (--)     ." %e[0K" ;
fg       (fg--)   ." %e[38;5;%dm" ;
bg       (bg--)   ." %e[48;5;%dm" ;
color    (bg fg--)fg bg ;

white 255 fg ;    red   203 fg ;
green  40 fg ;    yellow226 fg ;
blue   63 fg ;    purple201 fg ;
cyan  117 fg ;    grey  250 fg ;

colors(f t--)over - 1+ for
    dup i + dup fg ." (color-%d)" i 10 modif0crthen
 next drop white ;








