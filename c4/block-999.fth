(Block 999 - local words)

forget
999 block!

 3 load(strings)
 4 load(case/switch)
 5 load(screen)
 6 load(utility)
50 load(blocks)

rb  (--)forget 999 load ;
ls  (--)z" ls -l"       system ;
pwd (--)z" pwd"         system ;
lg  (--)z" lazygit"     system ;
vi  (--)z" nvim ."      system ;
ed! (blk--)block! ;inline

dict-sz  (--n)mem-end last - ; nwords(--)dict-sz de-sz / ;
hp-used(--n)vhere vars - ;     hp-free(--n)last vhere - ;
cf(--n)code-sz here - ;

." %WC4 - %Gv" .version ." %B - https://github.com/CCurl/c4%n"
mem-sz          ." %Y     Memory: %d bytes%n"
cf here code-sz ."        Code: %d word-codes, %d used, %d available%n"
nwords dict-sz  ."  Dictionary: %d bytes used, %d words%n"
hp-free hp-used ."        Heap: %d bytes used, %d available"
white




