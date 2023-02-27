: t 27 emit '[' emit ; inline
: t2 t (.) ';' emit (.) ;
: ->yx ( y x-- ) t2 'H' emit ;
: cls t ." 2J" 1 dup ->yx ;
: cur-on  t ." ?25h" ;
: cur-off t ." ?25l" ;
: color ( bg fg-- ) t2 'm' emit ;
: fg 40 swap color ;
