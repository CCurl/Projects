// Screen stuff

: csi 27 emit '[' emit ; inline
: set-color csi (.) ';' emit (.) 'm' emit ;
: reset-color 0 dup set-color ;
: goto-xy swap csi (.) ';' emit (.) 'H' emit ;
: cls csi ." 2J" 1 dup goto-xy ;
