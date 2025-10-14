(Block 999 - local words)

forget
999 block!

 3 load(strings)
 4 load(case/switch)
 5 load(screen)
 6 load(utility)
50 load(blocks)

rb  (--)forget 999 load ;        ls  (--)z" ls -l"    system ;
pwd (--)z" pwd"     system ;     lg  (--)z" lazygit"  system ;
vi  (--)z" nvim ."  system ;

128 var cmd
go(s--)green dup ztype white cr system ;
home(--s)cmd z" cd /home/chris/code/" s-cpy ;
c4(--s)home z" c4" s-cat ;          proj(--s)home z" Projects" s-cat ;
pull(s--)z"  && git pull -p" s-cat go ;
pull-all(--)c4 pull proj pull ;

dict-sz(--n)mem-end last - ;
hused(--n)vhere vars - ;       hfree(--n)last vhere - ;
cf(--n)code-sz here - ;        nw(--)dict-sz de-sz / ;

." %WC4 - %Gv" .version ." %W - https://github.com/CCurl/c4%n"
memory mem-sz    ."  %W    Memory: %Y%d bytes starting at $%x%n"
cf here code-sz  ."  %W      Code: %Y%d word-codes, %d used, %d available%n"
de-sz nw dict-sz ."  %WDictionary: %Y%d bytes used, %d words, %d bytes per word%n"
hfree hused      ."  %W      Heap: %Y%d bytes used, %d available%W"

