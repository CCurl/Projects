(Block 999 - local words)

forget

 3 load(strings)
 4 load(case/switch)
 5 load(screen)
 6 load(utility)
 9 load(encode/decode)
50 load(blocks)

green ." c4 - v" .version cr white
code-sz mem-sz ." Memory: %d bytes, Code: %d word-codes%n"

rb  (--)forget 999 load ;
ls  (--)z" ls -l"       system ;
pwd (--)z" pwd"         system ;
lg  (--)z" lazygit"     system ;
vi  (--)z" nvim ."      system ;
pull(--)z" git pull -p" system ;
bm  (--)200 load ;
ky  (--)6 for key . next ;
ed! (blk--)block! ;inline

." Hello."
9 block! rl9 load ;






