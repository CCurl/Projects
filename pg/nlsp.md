# NLSP - Enhanced Lisp Interpreter

A compact yet feature-rich Lisp interpreter implemented in C with no REPL, no garbage collection, and under 310 lines of code.

## Features

- **File or stdin input**: Processes Lisp expressions from a file or standard input
- **No REPL**: Batch processing mode only - reads all input, evaluates, and exits
- **No garbage collection**: Uses fixed-size arrays for memory management
- **User-defined functions**: Support for `defun`, `lambda`, and `let` bindings
- **Enhanced parsing**: Robust handling of nested expressions 
- **Symbol table**: Variable and function storage with lexical scoping
- **Compact implementation**: Only 306 lines of C code
- **Code organization**: Clean structure with forward declarations and optimized formatting

## Compilation

```bash
gcc -o nlsp nlsp.c
```

## Usage

### From file:
```bash
./nlsp program.lisp
```

### From stdin:
```bash
echo "(+ 2 3)" | ./nlsp
```

## Supported Data Types

- **Atoms**: Numbers, symbols, and special constants (`t`, `nil`)
- **Lists**: S-expressions represented as cons cells
- **Functions**: User-defined functions created with `defun` or `lambda`

## Built-in Functions

### Arithmetic
- `(+ a b)` - Addition
- `(- a b)` - Subtraction  
- `(* a b)` - Multiplication
- `(/ a b)` - Division

### List Operations
- `(cons a b)` - Create cons cell with car=a, cdr=b
- `(car list)` - Return first element of list
- `(cdr list)` - Return rest of list after first element
- `(quote expr)` - Return expression without evaluation

### Predicates
- `(atom expr)` - Return `t` if expr is an atom, `nil` otherwise
- `(eq a b)` - Return `t` if a equals b, `nil` otherwise
- `(null expr)` - Return `t` if expr is `nil`, `nil` otherwise

### Control Flow
- `(if condition true-expr false-expr)` - Conditional evaluation

### User-Defined Functions
- `(defun name (params) body)` - Define named function
- `(lambda (params) body)` - Create anonymous function
- `(let ((var val) ...) body)` - Local variable binding

## Examples

### Basic Arithmetic
```lisp
(+ 2 3)          ; → 5
(* 4 5)          ; → 20
(- 10 3)         ; → 7
(/ 15 3)         ; → 5
```

### Nested Expressions
```lisp
(+ (* 2 3) (- 8 3))              ; → 11
(+ 1 (* 2 3))                    ; → 7
(* (+ 2 3) (- 10 4))             ; → 30
```

### List Manipulation
```lisp
(quote (1 2 3))                    ; → (1 2 3)
(cons 1 (cons 2 nil))             ; → (1 2)
(cons 1 2)                        ; → (1 . 2)
(car (quote (a b c)))             ; → a
(cdr (quote (a b c)))             ; → (b c)
```

### Predicates and Conditionals
```lisp
(atom 42)                         ; → t
(atom (quote (1 2)))              ; → nil
(null nil)                        ; → t
(null (quote hello))              ; → nil
(eq 5 5)                          ; → t
(eq (quote hello) (quote world))  ; → nil
(if (eq 2 2) (quote yes) (quote no)) ; → yes
```

### User-Defined Functions
```lisp
(defun square (x) (* x x))
(square 5)                        ; → 25

(defun factorial (n) 
  (if (eq n 0) 1 
      (* n (factorial (- n 1)))))
(factorial 4)                     ; → 24

(let ((x 10) (y 20)) (+ x y))     ; → 30

(let ((double (lambda (x) (* x 2))))
  (double 7))                     ; → 14
```

### Sample Program
Create a file `test.lisp`:
```lisp
(+ (* 2 3) (- 8 3))
(cons (quote hello) (cons (quote world) nil))
(if (eq 5 5) (quote yes) (quote no))
(defun add-one (x) (+ x 1))
(add-one 41)
(let ((name (quote Alice)) (age 25))
  (cons name age))
```

Run with:
```bash
./nlsp test.lisp
```

## Implementation Details

### Memory Management
- Uses a fixed array of 8192 cells (`MAX_CELLS`) 
- Environment table with 256 entries (`MAX_ENV`)
- No garbage collection - memory is allocated sequentially
- Simple bump allocator with bounds checking

### Parser
- Recursive descent parser with enhanced nested expression handling
- Handles complex S-expressions and dotted pairs
- Tokenizes input stream character by character
- Fixed segmentation faults from malformed expressions

### Evaluator  
- Tree-walking interpreter with symbol table support
- Atoms self-evaluate (numbers, symbols) or look up variables
- Lists are function calls with first element as operator
- Function application with parameter binding and lexical scoping

### Environment System
- Stack-based variable and function storage
- Lexical scoping with environment restoration
- Support for local bindings (`let`) and function parameters

### Data Structure
```c
typedef struct cell {
    int type;  // 0=atom, 1=list, 2=function
    union { 
        char atom[64]; 
        struct { struct cell *car, *cdr; } list; 
        struct { struct cell *params, *body; } func;
    } v;
} cell_t;

typedef struct env {
    char name[64];
    cell_t *value;
} env_t;
```

## Recent Enhancements

### Version 2.1 (306 lines)
- **✅ Code organization improvements** - Forward declarations moved to dedicated section
- **✅ Function formatting optimization** - Short functions compressed to single lines where appropriate
- **✅ Better code structure** - Improved maintainability and readability
- **✅ Maintained functionality** - All features preserved during optimization

### Version 2.0 (300 lines)
- **✅ Fixed segmentation faults** - Eliminated crashes from malformed expressions
- **✅ Enhanced nested expression parsing** - Complex arithmetic now works reliably
- **✅ User-defined functions** - Full support for `defun`, `lambda`, and `let`
- **✅ Symbol table system** - Variable and function storage with lexical scoping
- **✅ Enhanced built-ins** - Added `null` predicate and improved error checking
- **✅ Better memory management** - Increased capacity to 8192 cells and 256 environments
- **✅ Improved print formatting** - Proper handling of dotted pairs and complex structures

## Limitations

- **Fixed memory pool** - no dynamic memory allocation (8192 cells max)
- **Sequential parsing** - some complex nested expressions may have parsing edge cases
- **Integer arithmetic only** - no floating point support
- **No string literals** - symbols only
- **No macros** - no code transformation facilities
- **Limited recursion depth** - stack-based limitations for deep recursion

## Error Conditions

The interpreter may crash or produce unexpected results with:
- Extremely deeply nested expressions (stack overflow)
- Malformed S-expressions with unmatched parentheses
- Too many allocated cells (>8192) or environment entries (>256)
- Some complex lambda applications in certain contexts

## Technical Specifications

- **Lines of code**: 306
- **Memory usage**: ~512KB fixed allocation  
- **Cell storage**: 8192 cells
- **Environment entries**: 256 variable/function bindings
- **Dependencies**: Standard C library only
- **C standard**: C99 compatible
- **Platform**: POSIX systems (Linux, macOS, etc.)

## License

This is a minimal educational implementation. Use freely for learning purposes.