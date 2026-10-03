# Forth.js - A Forth Interpreter in JavaScript

A minimal, efficient Forth interpreter that runs in the browser. Supports word definitions, arithmetic, stack operations, and can be embedded directly in HTML via `<script type="application/forth">` tags.

## Overview

This is a complete Forth virtual machine implemented in ~200 lines of JavaScript. It features:
- Data stack and return stack
- Compiled word definitions
- Immediate words (executed during compilation)
- Dictionary of primitives and user-defined words
- Efficient string parsing without repeated allocations
- Automatic execution of embedded Forth scripts on page load

## Architecture

### Core Components

- **Data Stack (`stk`)**: Holds operands and results
- **Return Stack (`rstk`)**: Stores return addresses for word calls
- **Memory (`mem`)**: Stores compiled word definitions and literals
- **Dictionary**: Maps word names to execution tokens (primitives or code addresses)
- **Parser (`tib`, `pos`, `tibLen`)**: Input buffer with position tracking
- **Program Counter (`pc`)**: Tracks current instruction

### Execution Model

1. **Outer interpreter** parses tokens from input
2. **Inner interpreter** executes compiled code
3. Words can be immediate (execute at compile time) or deferred (execute at runtime)
4. User-defined words are compiled to memory with `Comma()`

## Usage

### Inline Forth in HTML

```html
<!DOCTYPE html>
<html>
<head>
  <title>Forth Demo</title>
</head>
<body>
  <textarea id="forth-input" rows="10" cols="50"></textarea>
  <button onclick="runForth()">Run</button>
  <pre id="forth-output"></pre>
  
  <script src="jsforth.js"></script>
</body>
</html>
```

### Embedded Forth Scripts

You can embed Forth code directly in HTML:

```html
<script type="application/forth">
  5 3 + .
</script>

<script type="application/forth" src="program.forth"></script>
```

The browser treats `application/forth` as an unknown MIME type, so it doesn't auto-execute. Instead, the `load` event handler finds and executes all Forth scripts.

### Running Forth Programmatically

```javascript
outer('5 3 + .');  // Prints: 8
```

## Primitive Words

### Arithmetic
- `+` - Add top two stack items
- `-` - Subtract
- `*` - Multiply
- `/` - Divide (truncated)

### Comparison
- `<` - Less than
- `=` - Equals
- `>` - Greater than
- `0=` - Test if zero

### Bitwise
- `and` - Bitwise AND
- `or` - Bitwise OR
- `xor` - Bitwise XOR
- `com` - Bitwise NOT

### Stack Manipulation
- `dup` - Duplicate top of stack
- `drop` - Remove top of stack
- `swap` - Exchange top two stack items
- `over` - Copy second item to top

### I/O
- `.` - Print top of stack

### Memory
- `,` - Write top of stack to memory

### Control
- `:` `name` ... `;` - Define a new word
- `exit` - Return from word

## Examples

### Basic Arithmetic
```forth
5 3 + .        \ Prints: 8
10 3 - .       \ Prints: 7
4 5 * .        \ Prints: 20
20 4 / .       \ Prints: 5
```

### Stack Operations
```forth
5 dup . .      \ Prints: 5 5
5 3 swap . .   \ Prints: 5 3
5 3 over . . . \ Prints: 5 5 3
```

### Word Definitions
```forth
: double  dup + ;
5 double .    \ Prints: 10

: square  dup * ;
3 square .    \ Prints: 9
```

### Comparisons
```forth
5 3 > .       \ Prints: 1 (true)
2 2 = .       \ Prints: 1 (true)
10 0= .       \ Prints: 0 (false)
```

## How It Works

### Parsing

`nextWord(delim)` reads the next token without repeatedly slicing the input string:
- Uses a position pointer (`pos`) to track location in `tib`
- Caches `tib.length` to avoid repeated lookups
- Skips whitespace when delimiter is space
- Returns word length

### Compilation

When `:` is encountered:
1. Next token becomes the word name
2. `compiling` flag is set
3. Subsequent tokens are added to memory via `Comma()`
4. `;` seals the definition and sets `compiling = false`

### Execution

- Primitives execute immediately via lambda functions
- User-defined words are stored as memory addresses
- `inner()` sets the program counter and executes compiled code
- Return addresses are pushed/popped using the return stack

## Performance Optimizations

1. **Index-based parsing**: `pos` pointer instead of string slicing
2. **Cached length**: `tibLen` computed once per input
3. **Efficient lookup**: `charCodeAt()` for whitespace detection
4. **Inline lambdas**: Primitives defined inline in `definePrimitives()`
5. **Direct stack access**: `getTOS()`, `setTOS()` for fast operations

## Browser Integration

The `load` event handler automatically:
1. Finds all `<script type="application/forth">` tags
2. Loads external files via `fetch()` if `src` is set
3. Executes inline Forth code
4. Routes output to console.log (captured and displayed)