# SL - Simple Lang

A minimal stack-based programming language designed for educational purposes and simple scripting tasks.

## Overview

SL (Simple Lang) is a Forth-like stack-based language with a simple bytecode interpreter. It features:

- **Stack-based computation**: All operations work with a data stack
- **Minimal syntax**: Easy to learn and understand
- **Extensible**: Add new words and operations
- **Efficient**: Compiled to bytecode for execution

## Building

```bash
make
```

This will create the `sl` executable.

## Usage

### Interactive Mode (REPL)

```bash
./sl
```

This starts an interactive session where you can type commands directly.

### Running Scripts

```bash
./sl examples/hello.sl
```

## Language Reference

### Numbers

Numbers are pushed onto the stack:
```
5          \ pushes 5 onto the stack
10         \ pushes 10 onto the stack
0xFF       \ hexadecimal
```

### Arithmetic Operations

| Word | Stack Effect | Description |
|------|-------------|-------------|
| `+`  | ( a b -- a+b ) | Addition |
| `-`  | ( a b -- a-b ) | Subtraction |
| `*`  | ( a b -- a*b ) | Multiplication |
| `/`  | ( a b -- a/b ) | Division |
| `MOD` | ( a b -- a%b ) | Modulo |

Example:
```
5 3 +     \ Result: 8
10 4 -    \ Result: 6
6 7 *     \ Result: 42
```

### Comparison Operations

| Word | Stack Effect | Description |
|------|-------------|-------------|
| `=`  | ( a b -- flag ) | Equal |
| `<`  | ( a b -- flag ) | Less than |
| `>`  | ( a b -- flag ) | Greater than |

Note: True is represented as -1, False as 0.

### Logic Operations

| Word | Stack Effect | Description |
|------|-------------|-------------|
| `AND` | ( a b -- a&b ) | Bitwise AND |
| `OR`  | ( a b -- a\|b ) | Bitwise OR |
| `NOT` | ( a -- ~a ) | Bitwise NOT |

### Stack Operations

| Word | Stack Effect | Description |
|------|-------------|-------------|
| `DUP`  | ( a -- a a ) | Duplicate top of stack |
| `DROP` | ( a -- ) | Remove top of stack |
| `SWAP` | ( a b -- b a ) | Swap top two items |
| `OVER` | ( a b -- a b a ) | Copy second item to top |
| `ROT`  | ( a b c -- b c a ) | Rotate top three items |

### I/O Operations

| Word | Stack Effect | Description |
|------|-------------|-------------|
| `EMIT` | ( n -- ) | Print character with ASCII code n |
| `.`    | ( n -- ) | Print number followed by space |
| `CR`   | ( -- ) | Print newline |

### Control Flow

| Word | Description |
|------|-------------|
| `BYE` | Exit the interpreter |

## Examples

### Hello World

```
72 EMIT   \ H
101 EMIT  \ e
108 EMIT  \ l
108 EMIT  \ l
111 EMIT  \ o
33 EMIT   \ !
CR
```

### Simple Math

```
5 3 + .      \ Prints: 8
10 4 - .     \ Prints: 6
6 7 * .      \ Prints: 42
```

### Stack Manipulation

```
1 2 3        \ Stack: 1 2 3
DUP          \ Stack: 1 2 3 3
.            \ Prints: 3, Stack: 1 2 3
SWAP         \ Stack: 1 3 2
. .          \ Prints: 2 3, Stack: 1
```

## Comments

Lines starting with `\` are comments and are ignored by the interpreter.

## Architecture

SL uses a virtual machine with:
- **Data stack**: For operands and results
- **Return stack**: For subroutine calls (internal)
- **Bytecode interpreter**: Executes compiled code
- **Dictionary**: Stores word definitions

## Future Enhancements

Possible additions to the language:
- User-defined words (`:` and `;`)
- Loops (`DO` ... `LOOP`)
- Conditionals (`IF` ... `THEN` ... `ELSE`)
- Variables
- Memory access
- More I/O operations

## License

This is an educational project. Feel free to use and modify as needed.

## Author

Chris Curl
