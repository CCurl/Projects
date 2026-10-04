# Forth.js - A Forth Interpreter in JavaScript

A minimal, efficient Forth interpreter that runs in the browser. Supports word definitions, arithmetic, stack operations, memory access, and can be embedded directly in HTML via `<script type="application/forth">` tags.

## Overview

This is a complete Forth virtual machine implemented in ~200 lines of JavaScript. It features:
- Data stack and return stack with separate memory regions
- Compiled word definitions stored in shared memory
- Immediate words (executed during compilation)
- Dictionary of primitives and user-defined words
- Efficient string parsing with single-pass tokenization
- Automatic execution of embedded Forth scripts on page load
- Support for defining new words with `:` and `;`

## Architecture

### Core Components

- **Memory (`mem`)**: Unified array holding stacks, dictionary, and compiled code
  - Data Stack: base 0, grows upward
  - Return Stack: base 50, grows upward
  - Loop Stack: base 100, grows upward
  - Compiled Code: starts at address 150
- **Dictionary (`dictionary[]`)**: Array of word definitions with name, execution token (xt), and immediate flag
- **Input Buffer (`tib`, `pos`, `tibLen`)**: Tokenization state
- **Program Counter (`pc`)**: Current instruction pointer during execution
- **Compilation Flag (`compiling`)**: Tracks whether in compile or immediate mode

### Execution Model

1. **Outer Interpreter** (`outer()`) - Parses tokens from input string
   - Attempts to parse as number, colon definition, semicolon, or word lookup
   - Throws error on unknown word
   
2. **Inner Interpreter** (`inner()`) - Executes compiled code
   - Fetches and executes opcodes from memory
   - Supports both primitive functions and compiled word addresses
   - Manages return stack for nested word calls
   - Supports tail-call optimization
   
3. **Word Types**
   - **Immediate**: Executed during compilation (e.g., `:`, `;`)
   - **Primitive**: Native functions that manipulate the stack
   - **Compiled**: User-defined sequences of words compiled to memory

### Compilation Process

- `:` begins a word definition, compiling subsequent words to memory
- `;` ends compilation and appends `exit` token
- Numbers are compiled as `lit` (literal) followed by the value
- Word references are compiled as their execution token (address or function)
- When compiling, `Comma()` stores values in memory at `here` pointer

## Usage

### HTML Integration

Include the interpreter in your HTML file:

```html
<!DOCTYPE html>
<html>
<head>
  <title>Forth.js Demo</title>
  <link rel="stylesheet" href="jsforth.css">
</head>
<body>
  <h1>JS-Forth v2026.10.04 - Chris Curl</h1>
  <input 
    id="forth-repl" 
    type="text" 
    placeholder="Enter Forth code" 
    autofocus
    onkeydown="if (event.key==='Enter') { repl(); }">
  <br>
  <textarea id="forth-input" rows="10" cols="50" style="margin-top: 10px;"></textarea>
  <button onclick="runForth(undefined)">Run</button>
  <pre id="forth-output"></pre>
  
  <script src="jsforth.js"></script>
</body>
</html>
```

The interface includes:
- **REPL Input**: Single-line text input for quick Forth commands (press Enter to execute)
- **Multi-line Editor**: Textarea for larger programs
- **Run Button**: Executes code from the textarea
- **Output Window**: Pre-formatted text display for results
- **Dark Mode**: Automatic detection of system color scheme preference

### Embedded Forth Scripts

Embed Forth code directly in HTML using `type="application/forth"`:

```html
<script type="application/forth">
  5 3 + .
</script>

<script type="application/forth" src="program.forth"></script>
```

- Inline scripts execute their `innerText`
- External scripts are fetched via `fetch()` and executed
- Auto-execution happens on window `load` event
- Browser ignores `application/forth` MIME type, preventing unwanted parsing

### Programmatic Execution

Call the interpreter directly from JavaScript:

```javascript
// Single execution
runForth('5 3 + .');  // Outputs: 8

// Direct parsing (captures output via console.log override)
outer('10 2 / .');    // Outputs: 5
```

## Primitive Words

### Stack Manipulation
- `dup` - Duplicate top of stack: `( n -- n n )`
- `drop` - Remove top of stack: `( n -- )`
- `swap` - Exchange top two items: `( a b -- b a )`
- `over` - Copy second to top: `( a b -- a b a )`

### Arithmetic
- `+` - Add: `( a b -- a+b )`
- `-` - Subtract: `( a b -- a-b )`
- `*` - Multiply: `( a b -- a*b )`
- `/` - Divide (truncated): `( a b -- a/b )`

### Comparison
- `<` - Less than: `( a b -- flag )` where flag is -1 (true) or 0 (false)
- `=` - Equal: `( a b -- flag )` where flag is -1 (true) or 0 (false)
- `>` - Greater than: `( a b -- flag )` where flag is -1 (true) or 0 (false)
- `0=` - Test zero: `( n -- flag )` where flag is -1 (true) or 0 (false)

### Bitwise
- `and` - Bitwise AND: `( a b -- a&b )`
- `or`  - Bitwise OR:  `( a b -- a|b )`
- `xor` - Bitwise XOR: `( a b -- a^b )`
- `com` - Bitwise COMPLEMENT: `( a -- ~a )`

### Memory Access
- `@` - Fetch from memory: `( addr -- value )`
- `!` - Store to memory: `( value addr -- )`
- `,` - Compile value to code space: `( n -- )`

### I/O
- `.` - Print top of stack and remove: `( n -- )`

### Control Flow
- `exit` - Return from word (automatic at end of `:` definitions)
- `if` ... `then` - Conditional execution: `( flag -- )` executes code if flag is true (!= 0)
- `begin` ... `until` - Loop until flag is true: `( ... flag -- ... )` exits when flag is true
- `begin` ... `while` - Loop while flag is true: `( ... flag -- ... )` continues while flag is true
- `begin` ... `again` - Infinite loop: `( -- )` jumps back to begin unconditionally
- `for` ... `next` - Counted loop: `( n -- )` executes n times, use `i` to access loop counter
- `i` - Loop counter: `( -- count )` pushes current iteration number (0 to n-1)

## Examples

### Basic Arithmetic
```forth
5 3 + .          \ Output: 8
10 2 / .         \ Output: 5
7 2 * .          \ Output: 14
```

### Define Custom Words
```forth
: double dup + ;
5 double .       \ Output: 10

: square dup * ;
4 square .       \ Output: 16

: abs dup 0= drop swap drop ;
-5 abs .         \ Output: 5
```

### Stack Manipulation
```forth
1 2 3 swap .     \ Output: 2
1 2 3 over .     \ Output: 2
```

### Memory Operations
```forth
42 100 !         \ Store 42 at address 100
100 @ .          \ Load and print: 42
```

### Conditional Logic (using flags)
```forth
5 3 > .          \ Output: -1 (true)
5 3 < .          \ Output: 0 (false)
5 5 = .          \ Output: -1 (true)
```

## Implementation Details

### Memory Layout
- **0-49**: Reserved/unused
- **50-99**: Data stack (grows upward)
- **100-149**: Return stack (grows upward)
- **150+**: Compiled word definitions and literals

### Function Reference

- `push(val)` - Add value to data stack
- `pop()` - Remove and return top of stack (returns 0 on underflow)
- `TOS()` / `NOS()` - Peek at top/next-on-stack without removing
- `Comma(x)` - Store value at `here` pointer and increment
- `inner(start)` - Execute compiled code starting at address
- `outer(source)` - Parse and execute/compile source string
- `define(name, immediate)` - Add entry to dictionary
- `definePrim(name, fn)` - Add a primitive to dictionary
- `defineImm(name, fn)` - Add an IMMEDIATE primitive to dictionary
- `nextWord(delim)` - Extract next token from input buffer
- `doNum(token)` - Parse and handle numeric literal
- `doWord(token)` - Look up and execute/compile word
- `doColon(token)` - Begin word definition
- `doSemi(token)` - End word definition

### Limitations

- Limited error handling
- Stack underflow returns 0 instead of error
- No string literals or comments
- Fixed memory size (array length)

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
2. `compiling` flag is set to true
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
5. **Direct stack access**: `TOS()`, `setTOS()` for fast operations

## Browser Integration

The `load` event handler automatically:
1. Finds all `<script type="application/forth">` tags
2. Loads external files via `fetch()` if `src` is set
3. Executes inline Forth code
4. Routes output to console.log (captured and displayed)
