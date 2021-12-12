// Screen stuff

: csi 27 emit '[' emit ; inline
: set-color csi (.) ';' emit (.) 'm' emit ;
: reset-color 0 dup set-color ;
: goto-xy swap csi (.) ';' emit (.) 'H' emit ;
: cls csi ." 2J" 1 dup goto-xy ;

: color? dup i set-color ."  (" dup (.) ." ," i (.) ." ) " ;

: colors? 
	30 37 for i  40  47 for color? next drop reset-color cr next 
	// 90 97 for i 100 107 for color? next drop reset-color cr next 
;