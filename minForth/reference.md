# MinForth reference

## Built-in words

NOTE: The built-in words are not case sensitive

|Word|opcode|stack|notes|
|-|-|-|-|
|DUP|#|(a--a a)|Forth core word|
|OVER|%|(a b--a b a)|Forth core word|
|DROP|\\ |(a b--a)|Forth core word|
|IF|j|(f--)|Forth core word|
|ELSE|(none)|(--)|Forth core word|
|THEN|(none)|(--)|Forth core word|
|+|+|(a b--n)|Forth core word|
|-|-|(a b--n)|Forth core word|
|\*|\*|(a b--n)|Forth core word|
|/|/|(a b--n)|Forth core word|
|/mod|&|(a b--q r)|Forth core word|
|1+|I|(a--b)|Forth core word|
|1-|D|(a--b)|Forth core word|
|<|<|(a b--f)|Forth core word|
|=|=|(a b--f)|Forth core word|
|>|>|(a b--f)|Forth core word|
|."|Z|(--)|Forth core word (1)|
|"|(none)|(--a)|Forth core word|
|FOR|\[|(f t--)|For loop|
|I|i|(--n)|Current index|
|NEXT|\]|(--)|Next: if I <= t, jump to FOR|
|DO|{|(--)|Start of DO loop|
|WHILE|}|(n--)|If n != 0, jump to BEGIN|
|UNTIL|}|(n--)|If n == 0, jump to BEGIN|
|AGAIN|J|(--)|Jump to BEGIN|
|BREAK|^|(--)|Break out of FOR or DO loop|

```
(1) Notes on .":
- %d print TOS as integer
- %x print TOS as hex
- %b print TOS as binary
- %n print new-line
- %q print the quote (") character

example: : ascii $20 '~' for i i i i ." %n%d: (%c) %x %b" next ;
```
