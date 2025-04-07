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
dict-sz  (--n)mem-end last - ;
hu(--n)vhere vars - ;          hf(--n)last vhere - ;
cf(--n)code-sz here - ;        nw(--)dict-sz de-sz / ;

white ." C4 - v" .version ."  - https://github.com/CCurl/c4%n"
yellow mem-sz   ."      Memory: %d bytes%n"
cf here code-sz ."        Code: %d word-codes, %d used, %d available%n"
nw dict-sz      ."  Dictionary: %d bytes used, %d words%n"
hf hu           ."        Heap: %d bytes used, %d available"
white





