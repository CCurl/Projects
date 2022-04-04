# MinForth reference

## Built-in words

NOTE: The built-in words are not case sensitive

|Word|opcode|stack|notes|
|-|-|-|-|
|DUP|#|(--)|Forth core word|
|OVER|%|(--)|Forth core word|
|DUP|#|(--)|Forth core word|
|DROP|\\ |(--)|Forth core word|
|+|+|(--)|Forth core word|
|-|-|(--)|Forth core word|
|\*|\*|(--)|Forth core word|
|/|/|(--)|Forth core word|
|/mod|&|(--)|Forth core word|
|1+|I|(--)|Forth core word|
|1-|D|(--)|Forth core word|
|."|Z|(--)||
|FOR|\[|(--)|For loop|
|I|i|(--)|Current index|
|NEXT|\]|(--)|Next loop|
|DO|{|(--)|Start of while loop|
|WHILE|}|(--)|If TOS !=0, branch to BEGIN|
