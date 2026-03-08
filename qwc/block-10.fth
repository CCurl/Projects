\ ftype - usage: 123 .f" %q%Gthi-%R%d-%Ythere%W%q"

: t8 ( c-- ) x! \ handler for '%'
  x@ 'd' = if (.) exit then
  x@ 'e' = if 27  emit exit then
  x@ 'n' = if 10  emit exit then
  x@ 'q' = if '"' emit exit then
  x@ 'r' = if 13  emit exit then
  x@ 's' = if ztype    exit then
  x@ 'G' = if green    exit then
  x@ 'R' = if red      exit then
  x@ 'W' = if white    exit then
  x@ 'Y' = if yellow   exit then
  x@ emit ;
: t9 ( c-- ) dup '%' = if drop c@z+ t8 exit then emit ;
: ftype ( a-- )  +L z! begin c@z+ -if0 drop -L exit then t9 again ;
last @ const t4
: .f" ( "string"-- ) (") compiling? if t4 , exit then ftype ; immediate
