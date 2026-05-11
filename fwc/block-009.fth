( Words for git )

: .sys ( S-- ) dup .f" %Y%s%W%n" system ;

128 var cmd
: pl   ( a-- ) z"  && git pull -p" s-cat .sys ;
: cdp  ( a-- ) cmd z" cd \code\" s-cpy swap s-cat pl ;
: proj ( -- ) z" mine\projects" cdp ;
: fwc  ( -- ) z" mine\fwc" cdp ;
: bin  ( -- ) cmd z" cd \bin" s-cpy pl ;
: pull-all ( -- ) cls proj fwc green ." done." white ;
