(Block 999 - local words)

3 6 thru(strings, case/switch, screen, utility)
50 load(blocks)

green ." c4 - v" .version cr white
code-sz mem-sz ." Memory: %d bytes, Code: %d word-codes%n"

rb  (--)forget 999 load ;
ls  (--)z" ls -l"    system ;
pwd (--)z" pwd"      system ;
lg  (--)z" lazygit"  system ;
nvim(--)z" nvim ."   system ;
pull(--)z" git pull -p" system ;
bm  (--)200 load ;
ky  (--)6 for key . next ;
ed! (blk--)block! ; inline

999 block! ." Hello."

 marker











