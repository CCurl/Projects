The main goals for this minimal Forth are as follows:

- To be deployable to as many different kinds of development boards as possible.
- To also be able to run under any system that has a C compiler.
- To be minimal and "intuitively obvious upon casual inspection".
- To be easy to modify and extend.

To these ends, I have wandered off the beaten path in the following ways:

- This is a byte-coded implementation to save code space.
- The primitives/core words are built into the compiler, and not included in the dictionary.
- The dictionary is intersparsed with the code, it is not separated.
- A dictionary entry has no XT element, only 3 additional bytes (offest,flags,null terminator).
- To save space, code addresses are 2 bytes, so code space is limited to 16 bits (64kb).
- Variable space is separated, and supports 32-bit addresses.
- Locals: management of locals is totally up to the programmer.
- The primitive/core words are NOT case sensitive, other words ARE case sensitive.
- "WORDS" is built in, and cannot be called by another word at runtime (it's IMMEDIATE)

Some details:

Locals:
- Locals are allocated in sets of 10.
- They are completely under the control of the programmer.
- They are separate from the call sequence.
- They can be used across words.
- They have 2 operations read and set; you don't use @ or ! with them.
- The words to manage locals are: +locs, r0..r9, s0..s9 and -locs
  - +locs: allocates 10 new locals, r0 thru r9
  - r0..r9 reads a local, pushing it value onto the stack
  - s0..s9 sets a local, popping the value off the stack
  - -locs: destroys the most recently allocated locals.
