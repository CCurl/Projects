# S3 - A minimal and extendable stack machine VM/CPU

S3 is a minimal, extendable, and interactive environment where the source code IS the machine code. There is no compilation in S3.

# What is S3?

S3 is a 32-bit stack-based, RPN, Forth-like, virtual CPU/VM that has 26 registers, 26 functions, and any amount of user ram as the system supports.

A register is identified by 1 UPPER-case letter, so there are 26 registers available. Registers are essentially built-in variables. Reading, setting, incrementing or decrementing a register is a single operation.

A word/function is also identified by 1 UPPER-case letter.

```
Example 1: "Hello World!"                    - the typical "hello world" program.
Example 2: 123 sA 456 sB rA rB ++ .          - prints 579. (rA and rB are registers)
Example 4: 32 126[rI##"%n%d %c (%x)"]        - prints the ASCII table
Example 3: 13xPO 1{0 1[rI 13 xPWD 1000 xW]}  - the typical Arduino "blink" program.
```

The reference for S3 is here: https://github.com/CCurl/S3/blob/main/doc/reference.txt

Examples for S3 are here: https://github.com/CCurl/S3/blob/main/doc/examples.txt

# Why S3?

There were multiple reasons:

1. Many interpreted environments use tokens and a large SWITCH statement in a loop to execute the user's program. In these systems, the "machine code" (i.e. - byte-code ... the cases in the SWITCH statement) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when looking at the code that is actually being executed. Additionally there is a compiler and/or interpreter, often something similar to Forth, that is used to create the programs in that environment. For these enviromnents, there is a steep learning curve ... the programmer needs to learn the user environment and the hundreds or thousands of user functions in the libraries (or "words" in Forth). I wanted to avoid as much as that as possible, and have only one thing to learn: the "machine code".

2. I wanted to be free of the need for a multiple gigabyte tool chain and the edit/compile/run paradigm for developing everyday programs.

3. I wanted a simple, minimal, and interactive programming environment that I could modify easily.

4. I wanted an environment that could be easily deployed to many different types of development boards via the Arduino IDE.

5. I wanted to be able to use the same environment on my personal computer as well as development boards.

6. I wanted short commands so there was not a lot of typing needed.

S3 is the result of my work towards those goals.

# The implementation of S3

- The entire system is implemented in a few files. The engine is in S3.cpp.
- The same code runs on Windows, Linux, and multiple development boards (via the Arduino IDE).
- See the file "config.h" for system configuration settings.

# Building S3

- The target machine/environment is controlled by the #defines in the file "config.h"
- For Windows, I use Microsoft's Visual Studio (Community edition). I use the x86 configuration.
- For Development boards, I use the Arduino IDE. See the file "config.h" for board-specific settings.
- For Linux systems, I use vi and clang. See the "make" script for more info.
- I do not have an Apple system, so I haven't tried to build S3 for that environment.
- However, being such a simple and minimal C program, it should not be difficult to port S3 to any environment.

S3 was inspired by STABLE. See https://w3group.de/stable.html for details on STABLE.
A big thanks to Sandor Schneider for the inspiration for this project.
