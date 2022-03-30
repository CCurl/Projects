# Thoughts on the relationship between computer languages, compilers, and interpreters ...

## The "Problem":
The user wants to add 2 numbers (A and B) and store the result in C. Or, in some languages, simply "C = A + B".

## Some background:
The typical computer processor has some number of registers, a set of operations it can perform, a program counter, a stack, and some amount of dynamic memory to which it can read and write.

The set of operations is called the processor's machine language (ML).

If you think about it, a processor's ML is really just a "hard-wired" byte-code interpreter, where the interpretation part is implemented by how the components that make up the processor are wired together.

For example, take byte 105 ($69 hex):
- To a 6502, it means "Add with Carry".
- To a Z80, it means "Load register L from C".
- To an x86, it means "Signed multiply"

So byte 105 is "interpreted" by different processors differently. That is what I mean when I say that a processor's machine language is its "hard-wired" byte-code.

## Some thoughts on how to address the Problem:
How can we turn "C = A + B" into terms the computer understands? We obviously have to perform some sort of translation. This is the problem a language designer/implementer faces: how to turn what the human wants the computer to do into terms that the computer understands. 

Of course, the solution will ultimately depend on the target processor, because each processor's ML is unique.

Sure, on the surface, it seems like this shouldn't be too hard, especially for this extremely simple example problem (C = A + B). But what about more involved problems? It can get really complex, really fast. The devil is in the details.

Where to start? Many solutions start by defining their own "virtual processor" with a corresponding ML (a VML, if you will). A single instruction in the VML is generally pretty simple but also abstract.

It is usually easier to translate given Source code into a VML than it is to translate it directly into ML for a processor. Additionally, a VML is closer to a "real" ML than the Source. It's a step in the right direction at least. So when tasked with solving this problem, one might then start by designing a VML, a Syntax for the Source, and provide a way to translate the Source into that VML. At least that gets us closer to being able to tell the processor what the user wants it to do.

But we are obviously not done yet, as the processor only natively understands its ML, not our VML. We still have a gap to close.

### Compilers and Interpreters:
In a compiler-based world, a VML and Syntax for the Source is defined. The source is first translated into the VML. That VML is then transformed into the target processor's ML. Then the ML must be put together (linked) to some pre-written code that can set things up and kick off the ML on the given system. Those tasks must be performed one after the other (chained together), hence the term "tool-chain".

In an interpreter-based world, a VML and Syntax for the Source is also defined. But instead of other steps, the implementer provides a pre-written program that makes the system be able to load and understand (execute/interpret) the VML. Hence the term "interpreter".

On some interpreted systems (e.g. - S4 and MINT) there is no need to translate the Source at all, because the VML is human-readable, and the Source IS the VML.

On other interpreted systems (e.g. - many Forths), the is a focus on ease of the translation, but the VML is generally not very "human-readable".

On compiled systems, all bets are off, and there is no way to avoid tranforming the Source into ML for the target system.

## Translation into a VML:

For this exercise, I'll use the VML I created for myForth and a Forth-like syntax:

The Syntax/Source:
```
VARIABLE A
VARIABLE B
VARIABLE C

: DoWork A @ B A + C ! ;

1234 A ! 5678 B ! DoWork
```

A small piece of the VML:
```
Byte $02: "WLIT"   - Turn the next 2 bytes from the instruction stream into a number and PUSH that value
Byte $40: "FETCH"  - POP an address and PUSH that address' 32-bit value
Byte $2B: "ADD"    - POP the top 2 items, add them, and PUSH the result.
Byte $21: "STORE"  - POP an address and a value and store the value to that address
```

And I need to be translate it into:
```
Define A to be $A110 (VARIABLE A)
Define B to be $A112 (VARIABLE B)
Define C to be $A114 (VARIABLE C)

Push A (VML: WLIT A)
Fetch  (VML: FETCH)
Push B (VML: WLIT B)
Fetch  (VML: FETCH)
Add    (VML: ADD)
Push C (VML: WLIT C)
Store  (VML: STORE)

And the VML for "DoWork" would be:

$02 $10 $A1 $40  [WLIT A, FETCH]   
$02 $12 $A1 $40  [WLIT B, FETCH]  
$2B              [ADD]  
$02 $14 $A1 $21  [WLIT C, STORE]
```

Generation of the VML is the job of the parser. It has 2 tasks: (1) break the input stream (the Source) into tokens (the tokenizer), and (2) process the tokens to generate the VML. Sometimes, depending on the syntax, part (2) can be done in one pass. Othertimes, it might take more than one pass to correctly generate the desired VML.

As I mentioned earlier, some systems (S4, MINT) don't need parser at all. I call them "pure interpreters".

Also mentioned, a Forth system tries to make these tasks as easy as possible because there is just ONE token type (word), and the separator between the tokens is (almost) always whitespace.

The parser for many compilers, depending on their syntax, will have to know about multiple token types, and have some sort of a "state machine", so that it has an idea what types of tokens are valid depending on what state it is in. For example, in C, the parser would expect '(' after it encounters the "if" token. In this world, the tokenizer has to know what state it is in so that it can pull out the next token. For example, '+=' is a valid token for assignment, but in the "if ()" state, '+=' is not a valid token.

OK, so now we have a VML (simetimes generated, other times provided directly), that expresses the Source in a way we can work with. Now what? We still can't actually execute the VML on the target system.

## Bridging the GAP:
S4, Forth, MINT, Basic, to name just a few, and many other implementations take the interpreter route. Different Forths have differnnt VMLs, but they are still pretty much all interpreters (there are, I'm sure, some exceptions). Me ... I generally write the interpreter in a C compiler. I have, in the past, for some implementations/systems, written the interpreter in assembly, but that is a very time-consuming activity, and is just for that one architecture, so I generally shy waway from that when possible. In more "hard-core" Forths, the implementation of the VML is done in assembly, and uses one of the typical threading models.

There are also many compiled languages, of which C is probably the best known. The LLVM C implementation has a VML, the Syntax for C is quite complex, and the optimization phase (when enabled/used) is (I believe) applied to the VML before the transformation of the ML to the target processor is done.

### Summary:
It is pretty easy for an experienced programmer to parse source into a VML (depending, again, on the Syntax for the Source).

It is my opinion that the "post VML" steps (transformation/link) that a compiler needs to do makes the solution orders of magnitude more difficult to pull off. And if one wants to include some sort optimization of the generated ML, it is even more difficult. Many compiler tool-chains are multiple MB or GB beasts (not exactly minimal!). Modern machines (PCs) and Harvard architectures (many development boards) add even more complexity, because they impose more limits on what the program can do, and how to end up with code that actually runs on those systems. Older (vintage) systems were much simpler, and so it would be (at least somewhat) less difficult for those systems.

Interpreters are able to avoid all that extra complexity, making them much more "minimal".

I am a minimalist. So I will choose "interpreter" evvery time when implementing a language.
