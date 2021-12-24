# R4 - A small and fast stack machine VM/CPU

R4 is an simple, fast, minimal, and interactive virtual machine environment where the source code IS the machine code. There is no compilation in R4.

Why R4? There are multiple reasons:

1. Many programming environments use tokens and a large SWITCH statement in a loop to execute the user's program. In those systems, the machine code (aka - byte-code ... the cases in the SWITCH statement) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when looking at the code that is actually being executed. Additionally there is a compiler and/or interpreter, often something similar to Forth, that is used to work in that environment. For these enviromnents, there is a steep learning curve ... the programmer needs to learn the user environment and the hundreds or thousands of user functions in the libraries (or "words" in Forth). I wanted to avoid as much as that as possible, and have only one thing to learn: the machine code.

2. I wanted to be free of the need for a multiple gigabyte tool chain and the edit/compile/run paradigm for developing everyday programs.

3. I wanted a simple, minimal, and interactive programming environment that I could modify and extend easily.

4. I wanted an environment that could be easily deployed to many different types and sizes of development boards via the Arduino IDE.

5. I wanted to be able to use the same environment on my personal computer.

6. I wanted short commands so there was not a lot of typing needed.

R4 is the result of my work towards those goals.

- The entire system is implemented in 5 files: config.h, R4.h, R4.cpp, pc-main.cpp, and R4.ino.
- The same code runs on Windows, Linux, and multiple development boards (via the Arduino IDE).

The reference for R4 is here:   https://github.com/CCurl/Projects/blob/main/R4/reference.txt

There are examples for R4 here: https://github.com/CCurl/Projects/blob/main/R4/examples.txt

# Building R4

- For Windows, I use Microsoft's Visual Studio (Community edition). 
- For Development boards, I use the Arduino IDE. 
- For Linux systems, I use vi and clang. See the make script.
- I do not have an Apple system, so I haven't tried to build R4 for that environment.
- However, being such a simple and minimal C program, it should not be difficult to port R4 to any environment.

R4 was inspired by STABLE. See https://w3group.de/stable.html for details on STABLE.
A big thanks to Sandor Schneider for his inspiration for this project.
