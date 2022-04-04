# MinForth reference

## Built-in words

NOTE: The built-in words are not case sensitive

|Word|opcode|stack|notes|
|-|-|-|-|
|DUP|#|(a--a a)|Forth core word|
|OVER|%|(a b--a b a)|Forth core word|
|DROP|\\ |(a b--a)|Forth core word|
|+|+|(a b--n)|Forth core word|
|-|-|(a b--n)|Forth core word|
|\*|\*|(a b--n)|Forth core word|
|/|/|(a b--n)|Forth core word|
|/mod|&|(a b--q r)|Forth core word|
|1+|I|(a--b)|Forth core word|
|1-|D|(a--b)|Forth core word|
|=|=|(a b--f)|Forth core word|
|."|Z|(--)||
|FOR|\[|(--)|For loop|
|I|i|(--)|Current index|
|NEXT|\]|(--)|Next loop|
|DO|{|(--)|Start of while loop|
|WHILE|}|(--)|If TOS !=0, branch to BEGIN|
