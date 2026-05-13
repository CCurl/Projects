( Words for git )

: .sys ( S-- ) dup .f" %Y%s%W%n" system ;

128 var cmd
: prefix windows if z" \code\mine\" then z" /home/chris/code/" ;
: pl   ( a-- ) z"  && git pull -p" s-cat .sys ;
: cdp  ( a-- ) cmd z" cd " s-cpy prefix s-cat swap s-cat pl ;
: proj ( -- ) z" Projects" cdp ;
: fwc  ( -- ) z" fwc" cdp ;
: bin  ( -- ) windows if z" \bin" pl then ;
: pull-all ( -- ) cls proj fwc bin green ." done." white ;
