Machine Forth opcodes:

 (MuP21 Assembler Instructions)
From: http://www.ultratechnology.com/p21fchp9.html (chapter 9)

   CODE Name     Function

   Transfer Instructions
   00   JUMP     Jump to 10 bit address in the lower 10
                 bits of the current word.  Must be the
                 first or second instruction in a word
   01   ;'       Subroutine return.  (pop the address
                 from the top of the return stack and
                 jump to it)
   02   T=0      Jump if T=0
   03   C=0      Jump if carry is reset
   04   CALL     Subroutine call.  (push the address of
                 the next location in memory to the
                 return stack, and jump to the 10 bit
                 address in the lower 10 bits of the
                 current word.)
   05            reserved
   06            reserved
   07            reserved

   Memory Access Instructions
   08            reserved
   09   @A+      fetch a value from memory pointed to by
                 the A register, place it on the top of
                 the data stack, and increment A
   0A   #        fetch the next cell from memory as a
                 literal and place it on the top of the
                 data stack
   0B   @A       fetch a value from memory pointed to by
                 the A register, place it on the top of
                 the data stack, and increment A
   0C            reserved
   0D   !A+      remove the item in the top of data stack
                 and store it into memory pointed to by
                 the A register, increment A
   0E            reserved
   0F   !A       remove the item in the top of data stack
                 and store it into memory pointed to by
                 the A register

   ALU Instructions
   10   COM      complement all 21 bits in T (top of data
                 stack)
   11   2*       shift T left 1 bit ( the bottom bit
                 becomes 0)
   12   2/       shift T right 1 bit ( the top two bits
                 remain unchanged)
   13   +*       Add the second item on the data stack to
                 the top item without removing the second
                 item, if  the least signifigant bit of T
                 is 1
   14   XOR      remove the top two items from the data
                 stack and replace them with the result
                 of logically exclusively-oring them
                 together
   15   AND      remove the top two items from the data
                 stack and replace them with the result
                 of logically and-ing them together
   16            reserved
   17   +        remove the top two items from the data
                 stack and replace them with the result
                 of adding them together

   Register Instructions
   18   POP      move one item from the return stack to
                 the data stack
   19   A        copy the contents of the A register to
                 the top of stack
   1A   DUP      copy the top of stack to the top of
                 stack
   1B   OVER     copy the second item on the data stack
                 and make it the new top of the data
                 stack
   1C   PUSH     move one item from the data stack to the
                 return stack
   1D   A!       move the top of stack to the A register
   1E   NOP      null operation (delay 10ns)
   1F   DROP     discard the item on the top of the data
                 stack
