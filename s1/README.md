# S1
S1 is an interactive stack-based interpreter, implemented in fewer than 100 lines of C code.

S1 was inspired by Sandor Schneider's program `STABLE`<br/>
See `https://w3group.de/stable.html` for details.

S1 supports:
- 26 functions (A-Z).
- 26 registers (a-z) with auto- increment and decrement.
- Simple floating point math.
- Simple file operations. 

## Some Examples
```
0(this is a comment)
:H"Hello World":      0(define function H)
H                     0(print "Hello World!")
#("yes")~("no")       0(print "yes" or "no" depending on TOS)
:B" ":                0(define B to print a blank)
10[^I.B]              0(print out numbers from 0 through 9)
100 {#.B--}           0(count down and print out from 123 to 1)
355. 113. f/ f.       0(floating point - PI)
95[^I32+#.": ",10,]   0(print the ascii table)
a@ 0fO#(fR{,fR}fC)    0(print the contents of the file named by a)
```
## S1 Reference
```
*** STACK ***
#  (a--a a)       Duplicate TOS     (DUP)
\  (a b--a)       Drop TOS          (DROP)
$  (a b--b a)     Swap TOS and NOS  (SWAP)
%  (a b--a b a)   Push NOS          (OVER)
_  (a--b)         b: -a             (NEGATE)
++ (a--b)         b: a+1            (INCREMENT)
-- (a--b)         b: a-1            (DECREMENT)

*** ARITHMETIC ***
+   (a b--n)      n: a+b
-   (a b--n)      n: a-b
*   (a b--n)      n: a*b
/   (a b--q)      q: a/b
&   (a b--r q)    q: Quotient, r: Remainder


*** FLOATING POINT ***
N.N  (--n)        n: a floating point number
f+   (a b--n)     n: a+b
f-   (a b--n)     n: a-b
f*   (a b--n)     n: a*b
f/   (a b--q)     q: a/b
f<   (a b--a f)   f: if (a < b) then -1 else 0
f>   (a b--a f)   f: if (a > b) then -1 else 0
f.   (n--)        Output `n` as a floating point number


*** MEMORY ***
    LAYOUT:      [ SYS  | STK    |  FUNCS  |  REGS   | RSTK    | LSTK     | CODE     ]
    32-bit ints: [ 0-3  |  4-64  |  65-90  |  97-122 | 128-199 | 200-255  | 256-END  ]
    8-bit bytes: [ 0-15 | 16-259 | 260-363 | 388-491 | 512-799 | 800-1023 | 1024-END ]
@     (a--n)      Fetch INT  n from S1 address a
c@    (a--n)      Fetch BYTE n from S1 address a
!     (n a--)     Store INT  n to S1 address a
c!    (n a--)     Store BYTE n to S1 address a


*** REGISTERS ***
    NOTE: A register name is a single lowercase character, a-z.
a@    (--n)       Fetch of register `a`.
a!    (n--)       Store (n) to register `a`.
a+    (--)        Increment `a`.
a-    (--)        Decrement `a`.
a@+   (--n)       Fetch `a`, then increment `a`.
a@-   (--n)       Fetch `a`, then decrement `a`.


*** FUNCTIONS ***
    NOTE: A function name is a single UPPERCASE character, A-Z.
:X:   (--)        Define function `X`. Copy chars to (HERE++) until next ':'.
X     (?--?)      Call function `X`.
:     (--)        End function definition (compile ';').
;     (--)        Return from function.
    NOTE: 1) When in a WHILE loop, unwind the WHILE stack first using (^W;).
          2) When in a FOR loop, unwind the FOR stack first using (^F;).
0@    (--n)       n: HERE


*** INPUT/OUTPUT ***
.      (n--)      Output `n` as a decimal
,      (c--)      Output ASCII character `c`
".."   (--)       Output characters until the next '"'.
0..9   (--n)      Scan DECIMAL number n. For multiple numbers, separate them by space (47 33).
        NOTES: 1) To enter a negative number, use '_' (eg - `490_`).
               2) If "." immediately follows the number (eg - `355.`), then n is converted to a float.
'x     (--n)      n: the ASCII value of x
`XXX`  (--)       Executes "XXX" as a shell command (ie - system(xxx))
|XXX|  (A--B)     Copies XXX[null] to BYTE address A. B: Next char after the [null]
?      (--c)      c: next character from STDIN (0 if EOF)


*** CONDITIONS/LOOPS/FLOW CONTROL ***
<     (a b--f)    f: if (a <  b) then -1 else 0.
=     (a b--f)    f: if (a == b) then -1 else 0.
>     (a b--f)    f: if (a >  b) then -1 else 0.
<=    (a b--f)    f: if (a <= b) then -1 else 0.
>=    (a b--f)    f: if (a >= b) then -1 else 0.
~     (a--f)      f: if (a == 0) then -1 else 0.  (Logical NOT)
(     (f--)       IF: if (f != 0), continue into '()', else skip to next ')'.
[     (n--)       FOR: start a FOR/NEXT loop for `n` iterations.
]     (--)        Restart loop if less than `n` iterations have occurred.
{     (f--f)      BEGIN: if (f == 0) skip to next '}'.
}     (f--f?)     WHILE: if (f != 0) jump to opening '{', else drop f and continue.


*** CONDITIONS/LOOPS/FLOW CONTROL ***
^I    (--n)       Index of the current FOR stack.
^F    (--)        Unwind the FOR stack.
^W    (--)        Unwind the WHILE stack.
^T    (--n)       n: Timer (clock())
^Y    (n--)       n: the BYTE offset for a string to send to system().
^X    (--)        Exit S1


*** FILE ***
fO    (a n--f)    OPEN  - n: 0=>READ, else WRITE (usage: "file.txt" 0 fO)
fC    (f--)       CLOSE - f: file handle
fR    (f--f c)    FREAD - f: file handle, c: char read (0 if EOF)
fW    (f c--f)    WRITE - f: file handle, c: char to write
```
