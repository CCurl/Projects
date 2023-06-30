\ The idea here is to use Forth for the lexical parser, with the words pushing tokens onto the stack.
\ Then, when we hit the "end of statement" word (maybe '.'), output some machine code.
\ Perhaps r1 => EAX, R2 = EBX, and R3 = ECX.
\ Then, for:
\ r3 =  r1 + r2 .
\ ... we would want to output something like:
\ MOV ECX, EAX
\ ADD ECX, EBX
