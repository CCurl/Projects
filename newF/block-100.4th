// Screen stuff

: csi 27 emit '[' emit ; inline
: set-color csi (.) ';' emit (.) 'm' emit ;
: reset-color 0 dup set-color ;
: goto-xy csi (.) ';' emit (.) 'H' emit ;
: cls csi ." 2J" 1 dup goto-xy ;
: cursor-off csi ." ?25l" ;
: cursor-on  csi ." ?25h" ;

: color? dup i set-color ."  (" dup (.) ." ," i (.) ." ) " ;

: colors? 
	30 37 for i  40  47 for color? next drop reset-color cr next 
	// 90 97 for i 100 107 for color? next drop reset-color cr next 
;

: green  32 40 set-color ;
: yellow 33 40 set-color ;
: blue   34 40 set-color ;
: purple 35 40 set-color ;
: cyan   36 40 set-color ;
