\ A simple 6502 emulator
\ only a few opcodes are implemented.
\ adapted from: https://theultimatebenchmark.org/

reset
1 load

variable vm $1000 allot

$0800 CONSTANT org

: >vm ( vmAddr--addr ) vm + ;

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
    r5 >vm C@ r5 ." %npc: %x (%x)"
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

: next-inst ( --b ) r5 vm + c@ i5 ;

: >int8 ( b--n ) DUP $80 AND IF $100 - THEN ;
: vw@ ( vmAddr--word ) >vm dup c@ swap 1+ C@ $0100 * or ;
: vc@ ( vmAddr--char ) >vm C@ >int8 ;

: set-flags ( n-- ) DUP 0= flag-z ! $0080 and flag-n ! ;

: dojmp ( JMP aaaa ) r5 vw@ s5 3 cycles ;
: dolda ( LDA aa - WRONG!! )
    next-inst dup reg-a ! set-flags
    pc++ 2 cycles ;
: dosta ( STA -zp- ) reg-a C@ next-inst >vm c! 3 cycles ;
: dobeq ( BEQ aa ) 
    2 cycles
    flag-z @ if next-inst >int8 pc+ exit then
    pc++ ;
: dodex ( DEX ) reg-x C@ 1- $FF AND dup reg-x c! set-flags 2 cycles ;
: dodey ( DEX ) reg-y C@ 1- $FF AND dup reg-y c! set-flags 2 cycles ;
: doinc ( INC aa: r0: address )
    next-inst vm + s0 2 cycles
    r0 C@ 1+ $FF AND DUP r0 C! set-flags ;
: doldai ( LDA #b ) next-inst dup reg-a ! set-flags 2 cycles ;
: doldx  ( LDX #b ) next-inst dup reg-x ! set-flags 2 cycles ;
: doldy  ( LDY #b ) next-inst dup reg-y ! set-flags 2 cycles ;
: dobne  ( BNE aa )
    2 cycles
    flag-z @ 0= if next-inst >int8 pc+ exit then
    pc++ ;
: doNOP 1 cycles ;

: one-inst ( instr -- ) i7
    DUP $E6 = IF doinc  EXIT THEN
    DUP $D0 = IF dobne  EXIT THEN
    DUP $88 = IF dodey  EXIT THEN
    DUP $CA = IF dodex  EXIT THEN
    DUP $4C = IF dojmp  EXIT THEN
    DUP $A9 = IF doldai EXIT THEN
    DUP $85 = IF dosta  EXIT THEN
    DUP $A2 = IF doldx  EXIT THEN
    DUP $A0 = IF doldy  EXIT THEN
    DUP $A5 = IF dolda  EXIT THEN
    DUP $F0 = IF dobeq  EXIT THEN
    DUP $EA = IF doNOP  EXIT THEN
    ;

variable bp
: b, ( c -- ) bp @ C! 1 bp +! ;

: testcode
    org >vm bp !
    $A9 b, $00 b,         \ start: LDA #0
    $85 b, $08 b,         \        STA 08
    $A2 b, $10 b,         \        LDX #10
    $A0 b, $10 b,         \ loop1: LDY #10
    $E6 b, $08 b,         \ loop2: INC 08
    $88 b,                \        DEY
    $D0 b, $FB b,         \        BNE loop2 (offset)
    $CA b,                \        DEX
    $D0 b, $F6 b,         \        BNE loop1
    $4C b, $00 b, $08 b,  \        JMP start
    ;

: init-vm testcode 0 cycle! org s5 ;

: 6502emu ( cycles -- )
    BEGIN cycle OVER >
        IF DROP UNLOOP EXIT THEN
        next-inst one-inst DROP
    AGAIN ;

: mil #1000 DUP * * ;
: bench6502 ( cycles-- ) 0 s7 init-vm 6502emu ;

: elapsed timer swap - ;
: do-bench timer SWAP bench6502 elapsed r7 ;

// decimal
// 1 mil do-bench
: reload #6502 load ;
: step next-inst one-inst DROP .status cr 8 >vm c@ . ;

init-vm ." 6502 VM initialized%n" 
.status
decimal
