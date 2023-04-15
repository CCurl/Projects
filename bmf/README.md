# bmf - a "Bare Metal Forth" virtual machine

This system has 32-bit CELLs.
The stacks are 64 CELLs deep.

This system has 2 memory areas: code and data.
- The code area has a maximum of 64k (16-bit indexed).
- The data area can be larger (32-bit indexed).

There are three components to this system:
- The virtual machine itself (f-vm).
- A compiler/assembler for the virtual machine (f-comp).
- A disassembler for the virtual machine (f-dis).

The dictionary resides at the end of the code area and grows downward.

The data and return stacks are 32-bits wide.

There are 10 32-bit registers in this vm: r0 thru r9.
- s1 ( dw-- ) sets 

##Building
- Windows: There is a Visual Studio solution file
- Linux: There is a makefile

##Examples: 
```
```

## Reference
```
```
