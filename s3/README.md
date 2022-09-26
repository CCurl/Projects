# s3 - A minimal interpreter for PCs and Development boards

## What is s3?
- A full-featured, interactive, stack-based interpreter/VM, implemented in less than 200 lines of C code.
- Supports up to 1023 (MAX_FN) function variable-length definitions.
- Provides 26 registers, rA - rZ.
- Supports locals, 10 at a time, l0 - l9.
- Supports file operations, open, read, write, close, load.
- Supports floating point math.

## Why s3?
Many interpreted environments use tokens and a large SWITCH statement in a loop to execute the user's program. In these systems, the "machine code" (i.e. - byte-code ... the cases in the SWITCH statement) are often arbitrarily assigned and are not human-readable, so they have no meaning to the programmer when looking at the code that is actually being executed. Additionally there is a compiler and/or interpreter, which is used to create and execute the programs in that environment. For these enviromnents, there is a steep learning curve ... the programmer needs to learn the user environment and the hundreds or thousands of user functions in the libraries (or "WORDS" in Forth). I wanted to avoid as much as that as possible, and have only one thing to learn: the "machine code".

## Goals for s3
1. No need for a multiple gigabyte tool chain and the edit/compile/run paradigm for developing everyday programs.

2. A simple, minimal, and interactive programming environment that I can modify easily.

3. An environment that can be easily configured for and deployed to many different types of development boards via the Arduino IDE.

4. To be able to use the same environment on my personal computer as well as development boards.

5. Short and intuitive commands, so there is not a lot of typing needed.

## Building s3
```
- Windows: I use Visual Studio, the s3.sln. Use the x86 configuration, 64-bit doesn't work.
- Linux: There is a simple ./make shell script. GCC or CLANG, your choice; 32 or 64 bit both work.
- Development Boards: Use the Arduino IDE, edit the BSZ, ISZ and FILES_SZ as appropriate.
  - NOTE: this part is currently under development
```

## s3 Reference
```
*** ARITHMETIC ***
+   (a b--n)      n: a+b - addition
-   (a b--n)      n: a-b - subtraction
*   (a b--n)      n: a*b - multiplication
/   (a b--q)      q: a/b - division
^   (a b--r)      r: MODULO(a, b)
&   (a b--q r)    q: DIV(a,b), r: MODULO(a,b) - /MOD


*** BIT MANIPULATION ***
b&  (a b--n)      n: a AND b
b|  (a b--n)      n: a OR  b
b^  (a b--n)      n: a XOR b
b~  (a--b)        b: NOT a (ones-complement, e.g - 101011 => 010100)


*** STACK ***
#  (a--a a)       Duplicate TOS                    (DUP)
\  (a b--a)       Drop TOS                         (DROP)
$  (a b--b a)     Swap top 2 stack items           (SWAP)
%  (a b--a b a)   Push 2nd                         (OVER)
_  (a--b)         b: -a                            (Negate)
xA (a--b)         b: abs(a)                        (Absolute)


*** MEMORY ***
c@    (a--n)      Fetch BYTE n from address a
@     (a--n)      Fetch CELL n from address a
c!    (n a--)     Store BYTE n to address a
!     (n a--)     Store CELL n to address a


*** REGISTERS and LOCALS ***
  NOTES: 1) Register names are 1 UPPERCASE character: [rA..rZ]
         2) LOCALS: S3 provides 10 locals per call [r0..r9].
         3) Register rI is the FOR Loop index **special**
rA  (--n)         n: value of register A
sA  (n--)         n: value to store in register A
iA  (--)          increment register A
dA  (--)          decrement register A
nA  (--)          increment register A by the size of a cell (next cell)


*** WORDS/FUNCTIONS ***
        NOTE: Word/Function names are identified by 1 UPPERCASE character [A..Z]. 
:     (--)        Define word/function. Copy chars to (HERE++) until closing ';'.
A     (--)        Execute/call word/function A
;     (--)        Return: PC = rpop()
        NOTE: Returning while inside of a loop is not supported; use '|' to break out of the loop first.


*** INPUT/OUTPUT ***
.      (N--)      Output N as decimal number.
,      (N--)      Output N as character (Forth EMIT).
"      (?--?)     Output characters until the next '"'.
        NOTES: 1) %d outputs TOS as an integer
               2) %x outputs TOS as a hex number
               3) %c outputs TOS as a character
               4) %n outputs CR/LF
               5) %<x> outputs <x> (eg - "x%" %% %"x" outputs x" % "x)
0..9   (--n)      Scan DECIMAL number. For multiple numbers, separate them by space (47 33).
                     To enter a negative number, use "negate" (eg - 490_).
hNNN   (--h)      h: NNN as a HEX number (0-9, A-F, UPPERCASE only).
'x     (--n)      n: the ASCII value of x
`XXX`  (a--a b)   Copies XXX to address a, b is the next address after the NULL terminator.
xZ     (a--)      Output the NULL terminated string starting at address a.
xK?    (--f)      f: 1 if a character is waiting in the input buffer, else 0.
xK@    (--c)      c: next character from the input buffer. If no character, wait.


*** CONDITIONS/LOOPS/FLOW CONTROL ***
<     (a b--f)    f: (a < b) ? 1 : 0;
=     (a b--f)    f: (a = b) ? 1 : 0;
>     (a b--f)    f: (a > b) ? 1 : 0;
~     (n -- f)    f: (a = 0) ? 1 : 0; (Logical NOT)
[     (F T--)     FOR: start a For/Next loop. if (T < F), swap T and F
rI    (--n)       n: the index of the current FOR loop
]     (--)        NEXT: increment index (rI) and restart loop if (rI <= T)
        NOTE: A FOR loop always runs at least one iteration.
              It can be put it inside a '()' to keep it from running.
{     (f--f)      BEGIN: if (f == 0) skip to matching '}'
}     (f--f?)     WHILE: if (f != 0) jump to matching '{', else drop f and continue
|     (--)        BREAK: Break out of current WHILE of FOR loop
(     (f--)       IF: if (f != 0), continue into '()', else skip to matching ')'


*** OTHER ***
xPI   (p--)       Arduino: Pin Input  (pinMode(p, INPUT))
xPU   (p--)       Arduino: Pin Pullup (pinMode(p, INPUT_PULLUP))
xPO   (p--)       Arduino: Pin Output (pinMode(p, OUTPUT)
xPRA  (p--n)      Arduino: Pin Read Analog  (n = analogRead(p))
xPRD  (p--n)      Arduino: Pin Read Digital (n = digitalRead(p))
xPWA  (n p--)     Arduino: Pin Write Analog  (analogWrite(p, n))
xPWD  (n p--)     Arduino: Pin Write Digital (digitalWrite(p, n))
xR    (n--r)      r: a random number in the range [0..(n-1)]
                     NOTE: if n=0, r is the entire 32-bit random number
xT    (--n)       Milliseconds (Arduino: millis(), Windows: GetTickCount())
xN    (--n)       Microseconds (Arduino: micros(), Windows: N/A)
xW    (n--)       Wait (Arduino: delay(),  Windows: Sleep())
xIAF  (--a)       INFO: a: address of first function vector
xIAH  (--a)       INFO: a: address of HERE variable
xIAR  (--a)       INFO: a: address of first register vector
xIAS  (--a)       INFO: a: address of beeginning of system structure
xIAU  (--a)       INFO: a: address of beeginning of user area
xIF   (--n)       INFO: n: number of words/functions
xIH   (--n)       INFO: n: value of HERE variable
xIR   (--n)       INFO: n: number of registers
xIU   (--n)       INFO: n: number of bytes in the USER area
xSR   (--)        S3 system reset
xQ    (--)        PC: Exit S3
```
