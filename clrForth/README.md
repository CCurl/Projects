# c4 - a color-Forth inspired project

## What is c4?
C4 is a flavor of Forth that is inspired by colorForth. It is NOT an attempt to implement colorForth.

My other implementations use a byteCoded implementation, that uss a giant SWITCH statement for the implementation of te primitives.

In this implementation, the primitives are "void" functions; functions that take no parameters and return no value. All parameters and return values are placed on the (user-defined) parameter stack. Additionally, the code is a simple list of function addresses to primitives. There are 2 other stacks: a return stack and loop stack.

## Building c4

### Linux
- clang -m64 -o c4 c4.cpp
- I have created a ./make shell script for ease of use.

### Windows
- There is a Visual Studio solution; c4.sln
  - It can be built as either a 32- or 64-bit program
