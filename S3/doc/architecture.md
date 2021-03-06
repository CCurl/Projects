# S3 architecture:

The main interpreter loop in S3 is the run(addr start) function in the S3.cpp file. Parameter "start" is where the interpreter loop is to begin.

Variables:

```
pc      The 'program counter'; it points to the NEXT character in the instruction stream.
ir      The 'instruction register'; it holds the CURRENT instruction/opcode.
TOS     The 'top of stack'. This is very heavily used.
N       The 'next on stack'.
```

## Program flow:

Function run(addr start) is the heart of S3. It is very simple: 

```
(1) set pc to start, 
(2) get the next instruction, 
(3) execute the instruction, 
(4) repeat. 

If there is an error or if ir is ';' and the return stack is empty, then return.
```
