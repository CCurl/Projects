# S1
S1 is a full-featured and interactive stack-based interpreter/VM, implemented in fewer than 100 lines of C code.

It was inspired by Sandor Schneider's STABLE program.

S1 supports:
- 26 registers (a-z) with auto- increment and decrement.
- 26 function definitions (A-Z).
- floating point math.
- Simple file operations. 

## Some Examples
```
0(this is a comment)
"Hello World!"        0("Hello World!")
:C10,;                0(define function C)
1a!10[a@+." "]        0(print out numbers from 1 through 10)
#("yes")~("no")       0(print "yes" or "no" depending on TOS)
a@ 0fO#(fR{,fR}fC)    0(print the contents of the file named by a)
123 {#." "1-}         0(count down and print out from 123 to 1)
355. 113. f/ f.       0(floating point - PI)
32a!95[a@+#.": ",10,] 0(print the ascii table)
```
## S1 Reference
```
*** STACK ***
#  (a--a a)       Duplicate TOS             (DUP)
\  (a b--a)       Drop TOS                  (DROP)
$  (a b--b a)     Swap top 2 stack items    (SWAP)
%  (a b--a b a)   Push 2nd                  (OVER)
_  (a--b)         b: -a                     (NEGATE)


*** ARITHMETIC ***
+   (a b--n)      n: a+b
-   (a b--n)      n: a-b
*   (a b--n)      n: a*b
/   (a b--q)      q: a/b
&   (a b--q r)    q:: DIV(a,b), r:MOD(a,b)  (/MOD)


*** FLOATING POINT ***
N.N  (--n)        n: a floating point number
ff   (a--b)       b: int `a` converted to float
fi   (a--b)       b: float `a` converted to int
f+   (a b--n)     n: a+b
f-   (a b--n)     n: a-b
f*   (a b--n)     n: a*b
f/   (a b--q)     q: a/b
f<   (a b--a f)   f: (a < b) ? -1 : 0;
f>   (a b--a f)   f: (a > b) ? -1 : 0;


*** BIT MANIPULATION ***
b&  (a b--n)      n: a AND b
b|  (a b--n)      n: a OR  b
b^  (a b--n)      n: a XOR b
b~  (a--b)        b: NOT a (ones-complement, e.g - 11001011 => 00110100)


*** MEMORY ***
        USAGE: ints:  [0- 64:stacks][ 65- 90:regs][NUM_FUNCS][locals:100][code/free]
               bytes: [0-259:stacks][260-359:regs][NUM_FUNCS][locals:400][code/free]
@     (a--n)      Fetch INT   n from S1 address a
c@    (a--n)      Fetch BYTE  n from S1 address a
!     (n a--)     Store INT   n to S1 address a
c!    (n a--)     Store BYTE  n to S1 address a


*** REGISTERS ***
        NOTE: A register name is a single lowercase character, a-z.
a@    (--n)       Read value of register `a`.
a!    (n--)       Store (n) into register `a`.
a@+   (--n)       Fetch value of `a`, then increment it.
a@-   (--n)       Fetch value of `a`, then decrement it.


*** FUNCTIONS ***
        NOTE: A function name is a single UPPERCASE character, A-Z.
:X:   (--)        Define function XX. Copy chars to (HERE++) until next ';'.
X     (?--?)      Call function XX.
:     (--)        End function definition (compile ';').
;     (--)        Return from function.
        NOTES: 1) When in a WHILE loop, unwind the loop stack first using (^U;).
               2) When in a FOR loop, unwind the loop stack first using (^U^U^U;).


*** INPUT/OUTPUT ***
.      (n--)      n: Number to output as a decimal
,      (c--)      c: Character to output
".."   (--)       Output characters until the next '"'.
0..9   (--n)      Scan DECIMAL number n. For multiple numbers, separate them by space (47 33).
        NOTES: 1) To enter a negative number, use '_' (eg - 490_).
               2) If "." immediately follows the number (eg - 355.), then n is converted to a float.
'x     (--n)      n: the ASCII value of x
`XXX`  (--)       Executes XXX as a shell command (ie - system(xxx))
|XXX|  (A--B)     Copies XXX<NULL> to BYTE address A. B: Next char after the <NULL>
?      (--c)      c: next character from STDIN (0 if EOF)


*** CONDITIONS/LOOPS/FLOW CONTROL ***
<     (a b--f)    f: (a <  b) ? -1 : 0;
=     (a b--f)    f: (a == b) ? -1 : 0;
>     (a b--f)    f: (a >  b) ? -1 : 0;
<=    (a b--f)    f: (a <= b) ? -1 : 0;
>=    (a b--f)    f: (a >= b) ? -1 : 0;
~     (a--f)      f: (a == 0) ? -1 : 0;  (Logical NOT)
(     (f--)       IF: if (f != 0), continue into '()', else jump to next ')'.
[     (n--)       FOR: start a FOR/NEXT loop for `n` iterations.
]     (--)        Restart loop if less than `n` iterations have occurred.
{     (f--f)      BEGIN: if (f == 0) skip to next '}'.
}     (f--f?)     WHILE: if (f != 0) jump to opening '{', else drop f and continue.
^O    (n--)       n: the BYTE offset for a string to output.
^U    (--)        Remove the top entry from the loop stack.
        NOTES: 1) ^U can be used with ';' to return from a function while in a loop.
               2) A WHILE loop puts ONE entry on the loop stack (e.g: ^U;).
               3) A FOR loop puts 3 entries on the loop stack  (e.g: ^U^U^U;).
^Y    (n--)       n: the BYTE offset for a string to sent to system().


*** FILE ***
fO    (a n--f)    OPEN  - n: 0=>READ, else WRITE (usage: "file.txt" 0 fO)
fC    (f--)       CLOSE - f: file handle
fR    (f--f c)    FREAD - f: file handle, c: char read (0 if EOF)
fW    (c f--)     WRITE - f: file handle, c: char to write


*** OTHER ***
0@    (--n)       n: HERE
^T    (--n)       n: Timer (clock())
^X    (--)        Exit S1
```
