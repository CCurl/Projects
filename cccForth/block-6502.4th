\ ------ test file 6502.mfc ------

\ A simple 6502 emulation benchmark                         cas                    
\ only 11 opcodes are implemented. The memory layout is:                            
\  2kB RAM at 0000-07FF, mirrored throughout 0800-7FFF                              
\ 16kB ROM at 8000-BFFF, mirrored at C000                                            
             
\ adapted from: https://theultimatebenchmark.org/

reset
1 load

// hex

// variable rom $0800 allot
// variable ram $4000 allot
variable vm $FFFF allot

$8000 CONSTANT rom-start

: >vm ( vmAddr--addr ) vm + ;
// : >rom ( addr--vmAddr ) ram + rom-start + ;
// : >vm ( o--a ) $07FF and ram + ;
// :  ( o--a ) $3FFF and rom + ;

\ 6502 registers                                                                    
variable reg-a
variable reg-x
variable reg-y
variable reg-s


\ 6502 flags                                                                        
variable flag-c
variable flag-n
variable flag-z
variable flag-v

: .status 
    r5 >vm C@ r5 ." %npc: %i (%i)" 
    reg-a @ ." %na: %i" 
    reg-x @ ." %nx: %i" 
    reg-y @ ." %ny: %i" 
    flag-c @ flag-n @ flag-z @ ." %nflags: z: %d, n: %d, c: %d"
    ;

: pc r5 ;
: pc! s5 ;
: pc+ ( n-- )   r5 + s5 ;
: pc++ ( n-- )  i5 ;

: cycle  r6 ;
: cycle! s6 ;
: cycles  ( n-- )  r6 + s6 ;

: next-inst ( --b ) r5 >vm c@ i5 ;

: >int8 ( b--n ) DUP $80 AND IF $100 - THEN ;
: vw@ ( vmAddr--word ) >vm dup c@ swap 1+ C@ $0100 * or ;
: vc@ ( vmAddr--byte ) >vm C@ >int8 ;

: set-flags ( n-- ) DUP 0= flag-z ! $0080 and flag-n ! ;

: dojmp ( JMP aaaa ) pc vw@ pc! 3 cycles ;
: dolda ( LDA aa - WRONG!! )
    next-inst dup reg-a ! set-flags
    pc++ 2 cycles ;
: dosta ( STA -zp- ) reg-a C@ next-inst >vm c! 3 cycles ;
: dobeq ( BEQ aa ) 
    flag-z @
    if pc vc@ 1+ pc+ exit then
    pc++
    3 cycles ;
: dodex ( DEX ) reg-x C@ 1- $FF AND dup reg-x c! set-flags 2 cycles ;
: dodey ( DEX ) reg-y C@ 1- $FF AND dup reg-y c! set-flags 2 cycles ;
: doinc ( INC aa: r0: address ) 
    r5 >vm C@ >vm s0
    r0 C@ 1+ $FF AND dup r0 C! set-flags
    pc++ 2 cycles ;
: doldai ( LDA #b ) next-inst dup reg-a ! set-flags 2 cycles ;
: doldx  ( LDX #b ) next-inst dup reg-x ! set-flags 2 cycles ;
: doldy  ( LDY #b ) next-inst dup reg-y ! set-flags 2 cycles ;
: dobne ( BNE aa )
    2 cycles
    flag-z @ 0= if next-inst >int8 pc+ exit then
    pc++ ;
: doNOP 1 cycles ;

: one-inst ( instr -- ) s9 i7
    r9 $4C = IF dojmp  EXIT THEN
    r9 $A5 = IF dolda  EXIT THEN
    r9 $85 = IF dosta  EXIT THEN
    r9 $F0 = IF dobeq  EXIT THEN
    r9 $D0 = IF dobne  EXIT THEN
    r9 $A9 = IF doldai EXIT THEN
    r9 $CA = IF dodex  EXIT THEN
    r9 $88 = IF dodey  EXIT THEN
    r9 $E6 = IF doinc  EXIT THEN
    r9 $A0 = IF doldy  EXIT THEN
    r9 $EA = IF doNOP  EXIT THEN
    r9 $A2 = IF doldx  EXIT THEN
    ;

variable bp
: b, ( c -- ) bp @ C! 1 bp +! ;

: testcode
    rom-start >vm bp !
    $A9 b, $00 b,         \ start: LDA #0
    $85 b, $08 b,         \        STA 08
    $A2 b, $0A b,         \        LDX #10
    $A0 b, $0A b,         \ loop1: LDY #10
    $E6 b, $08 b,         \ loop2: INC 08
    $88 b,                \        DEY
    $D0 b, $FB b,         \        BNE loop2 (offset)
    $CA b,                \        DEX
    $D0 b, $F6 b,         \        BNE loop1
    $4C b, $00 b, $80 b,  \        JMP start
    ;

: init-vm testcode 0 cycle! rom-start s5 ;

: step next-inst one-inst .status cr 8 >vm c@ . ;

: 6502emu ( cycles -- )
    BEGIN cycle OVER >
        IF DROP UNLOOP EXIT THEN
        next-inst one-inst
    AGAIN ;

: mil #1000 DUP * * ;
: bench6502 ( cycles-- ) 0 s7 init-vm 6502emu ;

: elapsed timer swap - ;
: do-bench timer SWAP bench6502 elapsed r7 ;

// decimal
// 1 mil do-bench
: reload #6502 load ;

init-vm ." 6502 VM initialized%n" 
hex .status
