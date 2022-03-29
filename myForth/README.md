# MyForth

The main goals for this minimal Forth are as follows:

- For the implementation of it to be minimal and "intuitively obvious upon casual inspection".
- To be able to run under any system that has a C compiler.
- To be deployable to as many different kinds of development boards as possible via the Arduino IDE.
- To be easy to modify and extend the primitives.

To these ends, I have wandered off the beaten path in the following ways:

- This is a byte-coded implementation to save code space.
- The primitives/core words are built into the compiler, and not included in the dictionary.
- The dictionary is intersparsed with the code, it is not separated.
- A dictionary entry has no XT element, only 3 additional bytes (offset,flags,null terminator).
- To save space, code addresses are 2 bytes, so code space is limited to 16 bits (64kb).
- Variable space is separated, and supports 32-bit addresses.
- Temporary words: There are 10 temporary words that can be re-defined without any dictionary overhead.
- Temporary variables: management of these is totally up to the programmer.
- The primitive/core words are NOT case sensitive, user-defined words ARE case sensitive.
- "WORDS" is built in, and cannot be called by another word at runtime (it's IMMEDIATE)

## Some details:

### Temporary variables:
- Temporary variables are allocated in sets of 10.
- They are completely under the control of the programmer.
- They are not built into the call sequence.
- They can be accessed across words.
- They have 2 operations: read and set; you don't use @ or ! with them.
- The words to manage temps are: +tmps, r0..r9, s0..s9 and -tmps
  - +tmps: allocates 10 new temps, r0 thru r9
  - r0..r9 reads a temps, pushing it value onto the stack
  - s0..s9 sets a temps, popping the value off the stack
  - -tmps: destroys the most recently allocated temps.

### Temporary words:
- I think of temporary words as named transient words that don't incur any dictionary overhead.
- Their purpose is to support code reuse without the overhead of the dictionary entries.
- They are run-time only, You cannot create an IMMEDIATE temporary word.
- When defined, they refer to the current value of HERE.
- A temporary word's name is T[0..9]. E.G. - ": T4 ... ;" will define word T4, but T4 is not added to the dictionary.
- All previous references to T4 still refer to the previous definition. New references to T4 refer to the new defintion.
- Here is a simple example:
```
: T1 dup $20 < if drop '.' then ; \ Word "T1" is not added to the dictionary
: .code user dup here + 1- for i c@ T1 emit next ;
```
