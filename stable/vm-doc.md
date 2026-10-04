# VM bytecode map

The interpreter in `orig.c` is a tiny stack VM: each byte in the loaded program is read as an opcode and dispatched through `q[u]()`.

Important model:
- `st[]` is the stack
- `a` through `z` select a variable
- `A` through `Z` and `{` / `}` are used for jump/label-style control
- Bytes not listed below map to `N`, which is a no-op

| Char | Meaning | Notes |
|---|---|---|
| `!` | indirect store | `mem[mem[v]] = pop()` |
| `"` | string literal output | prints text until next `"` |
| `#` | duplicate top of stack | pushes a copy of `st[s]` |
| `$` | swap top two stack items | `x y -> y x` |
| `%` | modulo | `a = a % b` |
| `&` | bitwise AND | `a &= b` |
| `'` | character literal push | pushes each char value until closing `'` |
| `(` | conditional skip | if top of stack is zero, skip to matching `)` |
| `*` | multiply | `a *= b` |
| `+` | add | stack add; if variable mode active, increment variable |
| `,` | print char | prints `st[s]` as a character |
| `-` | subtract | stack subtract; if variable mode active, decrement variable |
| `.` | print integer | prints `st[s]` as a number |
| `/` | divide | `a /= b` |
| `0`-`9` | number literal | reads a decimal number and pushes it |
| `:` | store to variable | `var[v] = pop()` |
| `;` | load variable | pushes `var[v]` |
| `<` | comparison | actually checks `a > b` and returns `-1`/`0` |
| `=` | comparison | checks equality and returns `-1`/`0` |
| `>` | comparison | actually checks `a < b` and returns `-1`/`0` |
| `?` | indirect load | pushes `mem[mem[v]]` |
| `@` | rotate top 3 stack items | cycle `x y z -> y z x` |
| `A`-`Z` | jump/label slot | used as indirect jump targets via `st[u-35]` |
| `[` | loop guard | if top is zero, skip to matching `]` |
| `\` | pop/discard | drops the top stack value |
| `]` | loop end | if top is nonzero, jump back to matching `[` |
| `^` | read char | reads one byte from stdin and pushes it |
| `_` | unary negation | `-x` |
| `` ` `` | output current byte | prints `u` directly |
| `a`-`z` | select variable | sets active variable index (`a=0`, `b=1`, …, `z=25`) |
| `{` | label definition | records the address after `{` in a slot |
| `|` | bitwise OR | `a |= b` |
| `}` | return/jump back | restores `p = st[r]` and pops return stack |
| `~` | bitwise NOT | `~x` |

## A few important quirks

- This VM is not C syntax; it is custom bytecode.
- The symbols are chosen as opcodes, not normal operators in a conventional language.
- The lower-case letters are not “characters to print”; they are variable selectors.
- The interpreter uses `k` to switch between stack operations and variable operations, which is why `+`, `-`, etc. sometimes modify a variable instead of the stack.

In short: this is a compact, custom stack-based bytecode VM where the raw bytes themselves are the instructions.
