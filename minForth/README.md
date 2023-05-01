Thjis 

MuP21 (Machine Forth VM) opcodes

From: http://www.ultratechnology.com/p21fchp9.html (chapter 9)
```
   CODE Name     Function
   ---- -------- ----------------------------
   00   JUMP     Jump to 10 bit address in the lower 10 bits of the current word.
                    Must be the first or second instruction in a word

   01   ;'       Subroutine return.  (pop the address from the top of the return stack
                    and jump to it)

   02   T=0      Jump if T=0

   03   C=0      Jump if carry is reset

   04   CALL     Subroutine call.  (push the address of the next location in memory to the 
                    return stack, and jump to the 10 bit address in the lower 10 bits of 
                    the current word.)

   05            unused (used for AC!)

   06            unused (used for AC@)

   07            unused (used for SysOP)

   08            unused (used for LIT1)

   09   @A+      fetch a value from memory pointed to by the A register, place it on the top of
                    the data stack, and increment A

   10   #        fetch the next cell from memory as a literal and place it
                    on the top of the data stack

   11   @A       fetch a value from memory pointed to by the A register, place it on the top of
                    the data stack, and increment A 

   12            unused (used for A+)

   13   !A+      remove the item in the top of data stack and store it
                    into memory pointed to by the A register, increment A

   14            reserved

   15   !A       remove the item in the top of data stack and store it into 
                    memory pointed to by the A register

   16   COM      complement all 21 bits in T (top of data stack)

   17   2*       shift T left 1 bit (the bottom bit becomes 0)

   18   2/       shift T right 1 bit (the top two bits remain unchanged)

   19   +*       Add the second item on the data stack to the top item without 
                    removing the second item, if  the least signifigant bit of T is 1

   20   XOR      remove the top two items from the data stack and replace them with the result
                     of logically exclusively-oring them together

   21   AND      remove the top two items from the data stack and replace them with the result
                     of logically and-ing them together

   22            reserved

   23   +        remove the top two items from the data stack and replace them with the result
                     of adding them together

   24   POP      move one item from the return stack to the data stack

   25   A        copy the contents of the A register to the top of stack

   26   DUP      copy the top of stack to the top of stack

   27   OVER     copy the second item on the data stack and make it the new top of the data stack

   28   PUSH     move one item from the data stack to the return stack

   29   A!       move the top of stack to the A register

   30   NOP      null operation (delay 10ns)

   31   DROP     discard the item on the top of the data
                 stack
```

System operations
```
   CODE Name     Function
   ---- -------- ----------------------------
   101  EMIT     output 1 character (n--)

   102  .10      print 1 number in base 10

   103  .16      print 1 number in base 16

   104  FOPEN    open file (nm ty-fh)

   105  FCLOSE   close file (fh--)

   106  CCOMMA   standard c, (n--)

   107  COMMA    standard ,  (n--)

   108  CREATE   puts the next word into the dictionary (--)

   109  FIND     search for the next word in the dictionary (--dp)

   100  HA       address of H (HERE) (--a)

   111  LA       address of L (LAST) (--a)

   112  STA      address of ST (STATE) (--a)

   113  CSZ      size of a CELL (--n)
```
