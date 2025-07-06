(Encode and decode)

find fn! loaded?

64 var fn  cell var sz

max-sz  20 1024 * ; max-sz var txt
fn!(fn--)fn s-scpy drop ;
clr-txt(--)txt max-sz 0 fill ;
dmp(--)txt 256 dump ;
.txt(--)txt ztype ;
read-file(addr max fn--sz)fopen-rb ?dupif02drop 0 exitthen>a  a@ fread  a> fclose ;
write-file(addr sz fn--)fopen-wb ?dupif02drop exitthen>a  a@ fwrite drop  a> fclose ;
rd(--)clr-txt txt max-sz fn read-file sz ! ;
wt(--)txt sz @ fn write-file ;
go(n--)>t txt >a sz @ for @a t@ xor !a+ next atdrop ;
encode(n--)$100 /mod rd go go wt ;(e.g.-$CC66)
decode(n--)$100 /mod rd swap go go wt ;

z" test.dat" fn!












