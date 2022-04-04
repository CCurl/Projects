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
|AND|A|(a b--n)|Forth core word|
|OR|O|(a b--n)|Forth core word|
|XOR|X|(a b--n)|Forth core word|
|1+|I|(a--b)|Forth core word|
|1-|D|(a--b)|Forth core word|
|<|<|(a b--f)|Forth core word|
|=|=|(a b--f)|Forth core word|
|>|>|(a b--f)|Forth core word|
|0=|N|(a--b)|NOT: b: a==0 ? 1 : 0|
|."|Z|(--)|Forth core word (1)|
|"|(none)|(--a)|Forth core word|
|FOR|\[|(F T--)|For loop (2)|
|I|i|(--n)|Current index|
|NEXT|\]|(--)|Next: I += 1, if I <= T, jump to FOR|
|DO|{|(--)|Start of DO loop|
|WHILE|}|(n--)|If n != 0, jump to BEGIN|
|UNTIL|}|(n--)|If n == 0, jump to BEGIN|
|AGAIN|J|(--)|Jump to BEGIN|
|BREAK|^|(--)|Break out of FOR or DO loop|
|DUP     |#|||
|OVER    |%|||
|SWAP    |$|||
|DROP    |\|||
|ADD     |+|||
|SUB     |-|||
|MULT    |*|||
|DIV     |/|||
|DOT     |.|||
|CALL    |:|||
|RETURN  |;|||
|BLIT    |1|||
|WLIT    |2|||
|LIT     |4|||
|FETCH   |@|||
|STORE   |!|||
|CSTORE  |C|||
|1-      |D|||
|EXECUTE |G|||
|1+      |I|||
|BRANCH  |J|||
|LSHIFT  |L|||
|NOT (0=)|N|||
|RSHIFT  |R|||
|W!      |W|||
|RESET   |Y|||
|ZTYPE   |Z|||
|COM     |~|||
|=       |=|||
|>       |>|||
|<       |<|||
|EMIT    |,|||
|/MOD    |&|||
|        |^|||
|FOR     |[|||
|        |S|||
|        |S|||
|        |S|||
|NEXT    |]|||
|        | |||
|BEGIN   |{|||
|AGAIN   |}|||
|WHILE   |v|||
|UNTIL   |u|||
|AND     |a|||
|SPACE   |b|||
|C@      |c|||
|EDIT    |e|||
|I       |i|||
|IF      |j|||
|        | |||
|BL      |k|||
|CR      |n|||
|OR      |o|||
|+tmp    |p|||
|-tmp    |q|||
|readTemp|r|||
|setTemp |s|||
|TIMER   |t|||
|w@      |w|||
|XOR     |x|||

```
(1) Notes on .":
- %d print TOS as integer
- %x print TOS as hex
- %b print TOS as binary
- %n print new-line
- %q print the quote (") character

example: : ascii $20 '~' for i i i i ." %n%d: (%c) %x %b" next ;
```
```
(2) Notes on FOR:
- F and T can be specified in any order.

example: "1 10 for i . next" prints 1 2 3 4 5 6 7 8 9 10
         "10 1 for i . next" prints 1 2 3 4 5 6 7 8 9 10
```




