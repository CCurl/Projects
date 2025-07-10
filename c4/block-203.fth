( File Copy )

cell var fh
cell var th
 cell var byte
 cell var sz

copy-file( f t--sz )0 sz !
   fopen-wb th ! fopen-rb fh !
   begin
      byte 1 fh @ fread
      0= if fh @ fclose  th @ fclose sz @ exit then
      1 sz +! byte 1 th @ fwrite drop
    again ;





xxz" test.dat" z" t2.dat" copy-file ;












