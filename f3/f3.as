; **************************************************************************
;
; This is a program that implements a forth CPU/VM.
; The CPU's 'opcodes' are identified by their order in the [jmpTable].
; The VM's memory space is the reserved buffer at THE_MEMORY.
; When the 'VM' starts up, the PC/IP is set to point to memory address 0.
; This should be a JMP to the program's entry point.
;
; **************************************************************************
;                 x86 Register usage
; **************************************************************************
;
REG1 equ eax         ; Free register #1
REG2 equ ebx         ; Free register #2
REG3 equ ecx         ; Free register #3
REG4 equ edx         ; Free register #4
TOS  equ edi         ; Top-Of-Stack
PCIP equ esi         ; Program-Counter/Instruction-Pointer
STKP equ ebp         ; Stack-Pointer
;
; **************************************************************************

format PE console 

include 'win32ax.inc'

; -------------------------------------------------------------------------------------
section '.idata' data readable import

library kernel32, 'kernel32.dll', msvcrt, 'msvcrt.dll', conio, 'conio.dll'

import kernel32, ExitProcess,'ExitProcess', GetFileAttributes, 'GetFileAttributesA' \
    , GetStdHandle, 'GetStdHandle', GetTickCount, 'GetTickCount'

import msvcrt, __getmainargs, '__getmainargs' \
    , fopen,'fopen', fclose, 'fclose', fseek, 'fseek', ftell, 'ftell' \
    , fread, 'fread', fwrite, 'fwrite', fgetc, 'fgetc' \
    , putchar, 'putchar', fgets, 'fgets' \
    , getch, '_getch', kbhit, 'kbhit'

; -------------------------------------------------------------------------------------
include 'opcodes.inc'
include 'macros.inc'

MEM_SZ    equ 4*1024*1024
IMMEDIATE equ 1
INLINE    equ 2

; -------------------------------------------------------------------------------------
; section '.bss' data readable writable executable
section '.bss' data readable writable

printArgError db 'Error: ', 0
printFileError db 'Error: Cannot open file.', 0
unknownOpcode db 'unknown opcode!', 0
divByZero db 'cannot divide by 0.', 0
hello db 'f3 v0.1', 13, 10, 0
bye db 'Bye.', 13, 10, 0
ok db ' ok', 13, 10, 0
openModeRB db 'rb', 0
openModeRT db 'rt', 0
xxxyyy db 13,10,'4th>> ', 0

CELL_SIZE = 4
STD_INPUT_HANDLE = -10
STD_OUTPUT_HANDLE = -11
STD_ERROR_HANDLE = -12

argc dd ?
argv dd ?
env dd ?
stup dd ?

STDIN    dd ?
STDOUT   dd ?
inputFP  dd 0
outputFP dd 0
InitialESP dd 0

fileName  dd 0
rDepth    dd 0
rStackPtr dd 0
state     dd 0
here      dd 0
last      dd 0
tmpBuf1 db  16 dup (0)          ; Buffer for data stack
dStack  dd 256 dup (0)

dstackE db  16 dup (0)          ; Buffer between stacks

rStack  dd 256 dup (0)
tmpBuf3 db  16 dup (0)          ; Buffer for return stack

stopHere    db 'stop here!', 0
dsUnderFlow db '(Stack underflow!)', 0
dsOverFlow  db '(Stack overflow!)', 0

blockFile db "block-0000.fs", 0

buf4  db   4 dup (0)          ; Buffer for tib
tib   db 128 dup (0)          ; Text Input buffer
toIn  dd   0                  ; Ptr to current char
wd    db  64 dup (0)          ; Current word buffer

regs  dd   0 dup (100)        ; 
rbase dd   0                  ; 

; -------------------------------------------------------------------------------------
str_001 db ': here ;', 0
str_002 db ': last ;', 0
str_003 db ': cells cell NOOOO ;', 0
str_999 db 'abort', 0

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
jmpTable dd f_STOP      ; Hex: 00 ( 0)
dd f_EXIT               ; Hex: 01 ( 1)
dd f_CALL               ; Hex: 02 ( 2)
dd f_JMP                ; Hex: 03 ( 3)
dd f_JMPZ               ; Hex: 04 ( 4)
dd f_JMPNZ              ; Hex: 05 ( 5)
dd f_STORE              ; Hex: 06 ( 6)
dd f_CSTORE             ; Hex: 07 ( 7)
dd f_FETCH              ; Hex: 08 ( 8)
dd f_CFETCH             ; Hex: 09 ( 9)
dd f_LIT1               ; Hex: 0A (10)
dd f_LIT4               ; Hex: 0B (11)
dd f_DUP                ; Hex: 0C (12)
dd f_SWAP               ; Hex: 0D (13)
dd f_OVER               ; Hex: 0E (14)
dd f_DROP               ; Hex: 0F (15)
dd f_ADD                ; Hex: 10 (16)
dd f_MUL                ; Hex: 11 (17)
dd f_SLASHMOD           ; Hex: 12 (18)
dd f_SUB                ; Hex: 13 (19)
dd f_LT                 ; Hex: 14 (20)
dd f_EQ                 ; Hex: 15 (21)
dd f_GT                 ; Hex: 16 (22)
dd f_NOT                ; Hex: 17 (23)
dd f_DO                 ; Hex: 18 (24)
dd f_LOOP               ; Hex: 19 (25)
dd f_LOOP2              ; Hex: 1A (26)
dd f_INDEX              ; Hex: 1B (27)
dd f_DTOR               ; Hex: 1C (28)
dd f_RFETCH             ; Hex: 1D (29)
dd f_RTOD               ; Hex: 1E (30)
dd f_INC                ; Hex: 1F (31)
dd f_INCADDR            ; Hex: 20 (32)
dd f_DEC                ; Hex: 21 (33)
dd f_DECADDR            ; Hex: 22 (34)
dd f_COM                ; Hex: 23 (35)
dd f_AND                ; Hex: 24 (36)
dd f_OR                 ; Hex: 25 (37)
dd f_XOR                ; Hex: 26 (38)
dd f_EMIT               ; Hex: 27 (39)
dd f_TIMER              ; Hex: 28 (40)
dd f_SYSTEM             ; Hex: 29 (41)
dd f_KEY                ; Hex: 2A (42)
dd f_QKEY               ; Hex: 2B (43)
dd f_ISNUM              ; Hex: 2C (44)
dd f_DEFINE             ; Hex: 2D (45)
dd f_ENDWORD            ; Hex: 2E (46)
dd f_CREATE             ; Hex: 2F (47)
dd f_FIND               ; Hex: 30 (48)
dd f_WORD               ; Hex: 31 (49)
dd f_FOPEN              ; Hex: 32 (50)
dd f_LOAD               ; Hex: 33 (51)
dd f_FCLOSE             ; Hex: 34 (52)
dd f_FREAD              ; Hex: 35 (53)
dd f_FWRITE             ; Hex: 36 (54)
dd f_REG_I              ; Hex: 37 (55)
dd f_REG_D              ; Hex: 38 (56)
dd f_REG_R              ; Hex: 39 (57)
dd f_REG_S              ; Hex: 3A (58)
dd f_REG_NEW            ; Hex: 3B (59)
dd f_REG_FREE           ; Hex: 3C (60)
dd f_FREE1              ; Hex: 3D (61)
dd f_FREE2              ; Hex: 3E (62)
dd f_FREE3              ; Hex: 3F (63)

; -------------------------------------------------------------------------------------
; section '.code' code readable writable executable
section '.code' code readable executable


; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; The VM opcodes (primitives)
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; STOP
f_STOP: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; DO
f_DO: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; LOOP
f_LOOP: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; LOOP2
f_LOOP2: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; INDEX
f_INDEX: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; INCADDR
f_INCADDR: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; DECADDR
f_DECADDR: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; SYSTEM
f_SYSTEM:
            m_NEXT

; -------------------------------------------------------------------------------------
; s_IsNum ( strZ len--(n 1)|(0) )
s_IsNum:
        m_Save2 ecx, edx
        m_pop ecx
        m_pop eax
        ; **TODO**
        m_push 0
IN99:   m_Restore2 ecx, edx
        ret

; -------------------------------------------------------------------------------------
; ISNUM ( strZ len--(n 1)|(0) )
f_ISNUM:
        call s_IsNum
        m_NEXT

; -------------------------------------------------------------------------------------
; DEFINE
f_DEFINE:
        call s_CREATE
        mov [state], 1
        m_NEXT

; -------------------------------------------------------------------------------------
; ENDWORD
f_ENDWORD:
        m_push 1                ; EXIT
        call s_CComma
        mov [state], 0
        m_NEXT

; -------------------------------------------------------------------------------------
; s_CREATE ( -- )
s_CREATE:
        call s_WORD
        m_Save3 ebx, ecx, edx
        m_pop ecx               ; len
        m_pop edx               ; addr
        cmp ecx, 1
        jl cr99
        mov ebx,[here]
        m_Comma [last]          ; link
        m_CComma 0              ; flags
        m_CComma ecx            ; len
cr01:   cmp ecx, 1              ; name
        jl crNT
        m_CComma [edx]
        inc edx
        dec ecx
        jmp cr01
crNT:   m_CComma 0             ; NULL terminator
        mov [last], ebx
cr99:   m_Restore3 ebx, ecx, edx
        ret

; -------------------------------------------------------------------------------------
; CREATE ( -- )
f_CREATE:
        call s_CREATE
        m_NEXT

; -------------------------------------------------------------------------------------
; FIND  ( strZ len--(xt fl 1)|(0) )
f_FIND: call s_FindWord
        m_NEXT

; -------------------------------------------------------------------------------------
; s_NextCh ( --[al] )
s_NextCh:
        push ebx
        mov ebx, [toIn]
nc01:   mov al, [ebx]
        cmp al, 0
        je nc99
        inc [toIn]
nc99:   pop ebx
        ret

; -------------------------------------------------------------------------------------
; s_SKIP ( -- )
s_SKIP:
sk01:   call s_NextCh
        cmp al, 0
        je sk99
        cmp al, 32
        jl sk01
sk99:   ret

; -------------------------------------------------------------------------------------
; s_WORD ( --a n )
s_WORD:
        m_Save2 ecx, edx
        mov edx, wd
        m_push edx
        xor ecx, ecx
        call s_SKIP
wd01:   mov [edx], al
        cmp al, 33
        jl wd99
        inc edx
        inc ecx
        call s_NextCh
        jmp wd01
wd99:   mov BYTE [edx], 0
        m_push ecx
        m_Restore2 ecx, edx
        ret

; -------------------------------------------------------------------------------------
; WORD
f_WORD:
        call s_WORD
        m_NEXT

; -------------------------------------------------------------------------------------
; LOAD
f_LOAD: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; REG_I
f_REG_I: ; TODO!!!
        movzx ecx, BYTE [PCIP]
        inc PCIP
        mov ebx, [rbase]
        inc [regs+ebx+ecx*4]
        m_NEXT

; -------------------------------------------------------------------------------------
; REG_D
f_REG_D:
        movzx ecx, BYTE [PCIP]
        inc PCIP
        mov ebx, [rbase]
        dec [regs+ebx+ecx*4]
        m_NEXT

; -------------------------------------------------------------------------------------
; REG_R
f_REG_R:
        movzx ecx, BYTE [PCIP]
        inc PCIP
        mov ebx, [rbase]
        m_push [regs+ebx+ecx*4]
        m_NEXT

; -------------------------------------------------------------------------------------
; REG_S
f_REG_S:
        m_pop eax
        movzx ecx, BYTE [PCIP]
        inc PCIP
        mov ebx, [rbase]
        mov [regs+ebx+ecx*4], eax
        m_NEXT

; -------------------------------------------------------------------------------------
; REG_NEW
f_REG_NEW:
        cmp [rbase], 90
        jge rn99
        add [rbase], 10
rn99:   m_NEXT

; -------------------------------------------------------------------------------------
; REG_FREE
f_REG_FREE:
        cmp [rbase], 10
        jl rf99
        sub [rbase], 10
rf99:   m_NEXT

; -------------------------------------------------------------------------------------
; FREE1
f_FREE1: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; FREE2
f_FREE2: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; FREE3
f_FREE3: ; TODO!!!
            m_NEXT

; -------------------------------------------------------------------------------------
; LIT4 (32-bit)
f_LIT4:
            mov eax, [PCIP]
            add PCIP, CELL_SIZE
            m_push eax
            m_NEXT

; -------------------------------------------------------------------------------------
; FETCH
f_FETCH:
            add TOS, THE_MEMORY
            mov TOS, [TOS]
            m_NEXT

; -------------------------------------------------------------------------------------
; STORE
f_STORE:
            m_pop REG3
            m_pop eax
            add REG3, THE_MEMORY
            mov [REG3], eax
            m_NEXT

; -------------------------------------------------------------------------------------
; SWAP
f_SWAP:
            m_get2ND eax
            m_set2ND TOS
            m_setTOS eax
            m_NEXT

; -------------------------------------------------------------------------------------
; SWAP
s_SWAP:
            m_get2ND eax
            m_set2ND TOS
            m_setTOS eax
            ret

; -------------------------------------------------------------------------------------
; DROP
f_DROP:
            m_drop
            m_NEXT

; -------------------------------------------------------------------------------------
; DUP
f_DUP:
            m_push TOS
            m_NEXT

; -------------------------------------------------------------------------------------
; JMP
f_JMP:
            mov PCIP, [PCIP]
            add PCIP, THE_MEMORY
            m_NEXT

; -------------------------------------------------------------------------------------
; JMPZ
f_JMPZ:
            m_pop eax
            cmp eax, 0
            je f_JMP
            ; jmp noJMP
noJMP:      add PCIP, CELL_SIZE
            m_NEXT

; -------------------------------------------------------------------------------------
; JMPNZ
f_JMPNZ:
            m_pop eax
            cmp eax, 0
            jne f_JMP
            jmp noJMP

; -------------------------------------------------------------------------------------
; CALL
f_CALL:
            push dword [PCIP]
            add PCIP, CELL_SIZE
            sub PCIP, THE_MEMORY
            m_rpush PCIP
            pop PCIP
            add PCIP, THE_MEMORY
            m_NEXT

; -------------------------------------------------------------------------------------
checkStack:
            cmp STKP, dStack
            jl csUF
            cmp STKP, dstackE
            jge csOF
            ret

csUF:       ; push dsUnderFlow
            mov STKP, dstackE
            sub STKP, CELL_SIZE
            ret
            ; call s_TYPEz
            ; pop eax
            ; jmp f_RESET

csOF:       ; push dsOverFlow
            mov STKP, dStack
            ret
            ; call s_TYPEz
            ; pop eax
            ; jmp f_RESET

; -------------------------------------------------------------------------------------
; EXIT
f_EXIT:
            call checkStack
            m_rpop PCIP
            add PCIP, THE_MEMORY
            m_NEXT

; -------------------------------------------------------------------------------------
; OR
f_OR:
            m_pop eax
            or TOS, eax
            m_NEXT

; -------------------------------------------------------------------------------------
; XOR
f_XOR:
            m_pop eax
            xor TOS, eax
            m_NEXT

; -------------------------------------------------------------------------------------
; LIT1
f_LIT1:
            xor eax, eax
            mov al, [PCIP]
            m_push eax
            inc PCIP
            m_NEXT

; -------------------------------------------------------------------------------------
; CFETCH
f_CFETCH:
            xor eax, eax
            mov al, [TOS + THE_MEMORY]
            m_setTOS eax
            m_NEXT

; -------------------------------------------------------------------------------------
; CSTORE
f_CSTORE:
            m_pop REG3
            m_pop eax
            mov [REG3 + THE_MEMORY], al
            m_NEXT

; -------------------------------------------------------------------------------------
; ADD
f_ADD:
            m_pop eax
            add TOS, eax
            m_NEXT

; -------------------------------------------------------------------------------------
; SUB
f_SUB:
            m_pop eax
            sub TOS, eax
            m_NEXT

; -------------------------------------------------------------------------------------
; MUL
f_MUL:
            push edx
            m_pop eax
            xor edx, edx
            mul TOS
            m_setTOS eax
            pop edx
            m_NEXT

; -------------------------------------------------------------------------------------
f_SLASHMOD:
        call s_SLASHMOD
        m_NEXT

; -------------------------------------------------------------------------------------
s_SLASHMOD:
           m_pop REG3
           m_pop eax
           cmp REG3, 0
           je smDivBy0
           xor edx, edx
           div REG3
           m_push edx          ; Remainder
           m_push eax          ; Quotient
           ret

smDivBy0:
           m_push divByZero
           call s_TYPEz
           jmp f_RESET

; -------------------------------------------------------------------------------------
; LT
f_LT:
            m_pop eax
            cmp TOS, eax
            jl eq_T
            jmp eq_F

; -------------------------------------------------------------------------------------
; EQ
f_EQ:
            m_pop eax
            cmp TOS, eax
            je eq_T

eq_F:       m_setTOS 0
            m_NEXT

eq_T:       m_setTOS -1
            m_NEXT

; -------------------------------------------------------------------------------------
; GT
f_GT:
            m_pop eax
            cmp TOS, eax
            jg eq_T
            jmp eq_F

; -------------------------------------------------------------------------------------
; s_EMIT ( ch-- )
s_EMIT:
        m_Save3 ebx, ecx, edx
        m_pop eax
        push eax
        call [putchar]
        pop eax
        m_Restore3 ebx, ecx, edx
        ret

; -------------------------------------------------------------------------------------
; EMIT
f_EMIT:
            call s_EMIT
            m_NEXT

; -------------------------------------------------------------------------------------
; OVER
f_OVER:
            m_get2ND eax
            m_push eax
            m_NEXT

; -------------------------------------------------------------------------------------
; Makes al lowerCase if upperCase
u_ToLower:
                cmp al, 'A'
                jl u2lR
                cmp al, 'Z'
                jg u2lR
                add al, 32
u2lR:           ret

; -------------------------------------------------------------------------------------
; s_STRLEN ( str--n )
s_STRLEN:
        m_getTOS edx
        xor ecx, ecx
len01:  cmp BYTE [edx], 0
        je len99
        inc ecx
        inc edx
        jmp len01
len99:  m_setTOS ecx
        ret

; -------------------------------------------------------------------------------------
; s_STRCPYn ( dst src n-- )
s_STRCPYn:
        m_pop ecx
        m_pop ebx
        m_pop edx
sc01:   cmp ecx, 0
        je sc99
        mov al, [ebx]
        mov [edx], al
        inc ebx
        inc edx
        dec ecx
        jmp sc01
sc99:  ret

; -------------------------------------------------------------------------------------
; s_STRCPYz ( dst src-- )
s_STRCPYz:
        m_getTOS eax
        m_push eax
        call s_STRLEN
        inc TOS
        jmp s_STRCPYn

; -------------------------------------------------------------------------------------
; s_STREQi ( s1 l1 s2 l2--f )
; Case insensitive comparison of s1 and s2
s_STREQi:
        m_Save3 ebx, ecx, edx
        m_pop eax
        m_pop ebx
        m_pop ecx
        m_pop edx
        m_push 1                ; Default to TRUE
        cmp eax, ecx            ; Are the lengths the same?
        jne sEqNo
sEq01:  cmp ecx, 0
        je sEq99
        mov al, BYTE [ebx]
        cmp al, BYTE [edx]
        jne sEqNo
        inc ebx
        inc edx
        dec ecx
        jmp sEq01
sEqNo:  m_setTOS 0              ; Make it FALSE
sEq99:  m_Restore3 ebx, ecx, edx
        ret

; -------------------------------------------------------------------------------------
; do_COMPARE
; Compare strings pointed to by PCIP and REG4
; case sensitive: REG2 = 0
; case insensitive: REG2 != 0
; return in eax: -1 => strings are equal, 0 => strings are NOT equal
do_COMPARE:
                call s_STREQi
                test eax, eax
                jz cmpT
                mov eax, 0
                ret
cmpT:           mov eax, -1
                ret

; -------------------------------------------------------------------------------------
; COMPARE
f_COMPARE:
                push PCIP

                m_pop REG4
                add REG4, THE_MEMORY
                m_pop PCIP
                add PCIP, THE_MEMORY
                xor REG2, REG2
                call do_COMPARE
                m_push eax

                pop PCIP

                m_NEXT

; -------------------------------------------------------------------------------------
; FOPEN: ( name mode type -- fp success )
f_FOPEN:
                ; mode has to go first in the openMode
                ; put that on the top of the stack
                call s_SWAP

                ; save these
                push REG2
                push REG4

                mov REG4, tmpBuf1

                m_pop REG3                       ; mode: 0 => read, 1 => write
                mov al, 'r'
                cmp REG3, 0
                je fopen1
                mov al, 'w'
fopen1:         mov [REG4], al

                m_pop REG3                       ; type: 0 => text, 1 => binary
                mov al, 't'
                cmp REG3, 0
                je fopen2
                mov al, 'b'
fopen2:         inc REG4
                mov [REG4], al

                inc REG4
                mov [REG4], byte 0
                ; now [tmpBuf1] has the openMode for fopen()

                ; now for the filename
                m_pop eax
                inc eax                         ; skip the count byte
                add eax, THE_MEMORY

                ; function signature is fp = fopen(name, openMode);
                push tmpBuf1
                push eax
                call [fopen]                    ; returns the FP in eax
                pop REG3                         ; clean up the stack
                pop REG3

                m_push eax
                m_push eax

                ; restore these
                pop REG4
                pop REG2
                m_NEXT

; -------------------------------------------------------------------------------------
; s_FGETC ( fp -- ch num ) - if num==0, EOF
s_FGETC:
                ; signature is: EAX (num-read) = fread(addr, size, count, stream);
                mov BYTE [buf4], 0
                m_pop eax
                push eax        ; stream
                push 1          ; count
                push 1          ; size
                push buf4       ; addr
                call [fread]
                movzx ebx, BYTE [buf4]
                m_push ebx
                m_push eax
                ; clean up from the call
                pop eax
                pop eax
                pop eax
                pop eax
                ret

; -------------------------------------------------------------------------------------
; FREAD ( addr count fp -- num-read )
f_FREAD:
                ; save these
                push REG2

                ; signature is: EAX (num-read) = fread(addr, size, count, fp);
                ; args for [fread]
                m_pop eax       ; Stream
                push eax
                m_pop eax       ; count
                push eax
                push 1          ; size
                m_pop eax       ; addr
                add eax, THE_MEMORY
                push eax
                call [fread]
                m_push eax       ; EAX = return val (num-read)
                ; clean up from the call
                pop eax         ; addr
                pop eax         ; size
                pop eax         ; count
                pop eax         ; stream

                ; get these back
                pop REG2
                m_NEXT

; -------------------------------------------------------------------------------------
; s_READLINE ( addr max-sz fp -- num-read )
s_READLINE:
                ; signature is: buf = fgets(addr, size, fp);
                ; Returns addr if successful, NULL if EOF or Error
                ; NB: the string returned at addr should be null-terminated

                ; Save REG2
                push REG2

                m_pop eax       ; FP
                push eax
                m_pop eax       ; max
                push eax
                m_pop eax       ; addr
                push eax
                call [fgets]    ; returns addr if OK, else NULL
                m_push eax
                pop REG3        ; addr
                pop eax         ; max
                pop eax         ; FP

                cmp TOS, 0
                je rdlEOF

                push REG3        ; remember the original addr (+1)
                xor eax, eax
rdlC:           mov ah, [REG3]
                cmp ah, 0
                jz rdlCX
                inc al          ; this is the length
                inc REG3
                jmp rdlC
                
rdlCX:          pop REG3         ; Make it a counted string
                dec REG3
                mov [REG3], al

rdlX:           m_setTOS eax
                ; Restore REG2
                pop REG2
                ret

rdlEOF:         dec REG3
                mov [REG3], word 0
                m_setTOS 0
                pop REG2
                ret

; -------------------------------------------------------------------------------------
; FREADLINE ( addr max-sz fp -- num-read )
f_FREADLINE:
        m_pop ecx
        m_pop ebx
        m_pop eax
        inc eax
        dec ebx
        m_push eax
        m_push ebx
        m_push ecx
        call s_READLINE
        m_NEXT

; -------------------------------------------------------------------------------------
; FWRITE: ( addr count fp -- num-written ) 
f_FWRITE:
                ; save these
                push REG2

                ; signature is: num-written = fwrite(addr, size, count, fp);
                ; args for [fread]
                m_pop eax       ; Stream
                push eax
                m_pop eax       ; count
                push eax
                push 1          ; size
                m_pop eax       ; addr
                add eax, THE_MEMORY
                push eax
                call [fwrite]
                m_push eax       ; EAX = return val (num-written)
                ; clean up from call
                pop eax
                pop eax
                pop eax
                pop eax

                ; get these back
                pop REG2
                m_NEXT

; -------------------------------------------------------------------------------------
; FCLOSE
f_FCLOSE:   ; ( fp -- )
            m_pop eax
            push REG2
            push eax
            call [fclose]
            pop eax
            pop REG2
            m_NEXT

; -------------------------------------------------------------------------------------
; DTOR
f_DTOR:
            m_pop eax
            m_rpush eax
            m_NEXT

; -------------------------------------------------------------------------------------
; RTOD
f_RTOD:
            m_rpop eax
            m_push eax
            m_NEXT

; -------------------------------------------------------------------------------------
; COM
f_COM:
            not TOS
            m_NEXT

; -------------------------------------------------------------------------------------
; RFETCH - R@ ( -- n)
f_RFETCH:
                mov REG3, [rStackPtr]
                mov eax, [REG3]
                m_push eax
                m_NEXT

; -------------------------------------------------------------------------------------
; AND
f_AND:
            m_pop eax
            and TOS, eax
            m_NEXT

; -------------------------------------------------------------------------------------
; NOT ( n1 -- n2 )
f_NOT:
            test TOS, TOS
            jz eq_T
            jmp eq_F

; -------------------------------------------------------------------------------------
; PICK
f_PICK:
            m_getTOS eax
            shl eax, 2
            mov REG3, STKP
            sub REG3, eax
            mov eax, [REG3]
            m_setTOS eax
            m_NEXT

; -------------------------------------------------------------------------------------
; sKEY ( --n )
s_KEY:
            push REG2
            call [getch]
            pop REG2
            cmp eax, 3
            je f_BYE
            m_push eax
            ret

; -------------------------------------------------------------------------------------
; sKEY ( --n )
f_KEY:
        call s_KEY
        m_NEXT

; -------------------------------------------------------------------------------------
; QKEY
f_QKEY: ; TODO!!!
            xor eax, eax
            m_push eax
            m_NEXT

; -------------------------------------------------------------------------------------
; COMPAREI
f_COMPAREI:
                push PCIP
                push REG4
                push REG2

                m_pop REG4
                m_pop PCIP
                add REG4, THE_MEMORY
                add PCIP, THE_MEMORY
                mov REG2, 1
                call do_COMPARE
                m_push eax

                pop REG2
                pop REG4
                pop PCIP
                m_NEXT

; -------------------------------------------------------------------------------------
; INC
f_INC:
            inc TOS
            m_NEXT

; -------------------------------------------------------------------------------------
; DEC
f_DEC:
            dec TOS
            m_NEXT

; -------------------------------------------------------------------------------------
; TIMER
f_TIMER:
                push REG2
                call [GetTickCount]
                m_push eax
                pop REG2
                m_NEXT

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; Implementation of # ...
; i.e. : # BASE @ /MOD SWAP '0' + ;
;   input:
;     - eax = BASE
;     - REG3 = end of buffer for char
;   output:
;     - eax: unchanged
;     - REG3: REG3-1
s_NumToText:
                push REG3
                push eax
                m_push eax
                call s_SLASHMOD
                call s_SWAP
                add TOS, '0'
                pop eax
                pop REG3
                m_pop REG2
                mov [REG3], bl
                dec REG3
                ret

; -------------------------------------------------------------------------------------
; f_OPENBLOCK
f_OPENBLOCK:
                ; replace the "0000" in "block-0000.fs" with block #
                mov REG3, blockFile
                add REG3, 9
                mov eax, 10
                call s_NumToText
                call s_NumToText
                call s_NumToText
                call s_NumToText
                m_drop
                
                push REG2        ; save these
                push REG4

                push openModeRT
                push blockFile
                call [fopen]
                pop REG3
                pop REG3

                pop REG4         ; get them back
                pop REG2

                m_push eax
                m_push eax
                
                m_NEXT

; -------------------------------------------------------------------------------------
; NOP
f_NOP:
                m_NEXT

; -------------------------------------------------------------------------------------
; s_TYPE ( a n-- )
s_TYPE:
        m_Save2 ecx, edx
        m_pop ecx
        m_pop edx
t01:    cmp ecx, 0
        je t99
        movzx eax, BYTE [edx]
        m_push eax
        call s_EMIT
        inc edx
        dec ecx
        jmp t01
t99:    m_Restore2 ecx, edx
        ret

; -------------------------------------------------------------------------------------
; s_TYPEz ( a-- )
s_TYPEz:
        m_getTOS eax
        m_push eax
        call s_STRLEN
        jmp s_TYPE

; -------------------------------------------------------------------------------------
; BREAK
f_BREAK:
            mov REG3, REG4
            sub REG3, THE_MEMORY
            int3
            m_NEXT

; -------------------------------------------------------------------------------------
f_UnknownOpcode:
            m_push unknownOpcode
            call s_TYPEz
            jmp f_RESET

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; RESET
f_RESET:
        call sys_INIT
        mov esp, [InitialESP]
        jmp cpuLoop

; -------------------------------------------------------------------------------------
; sys_INIT: Initialize the VM
sys_INIT:
        ; Return stack
        mov eax, rStack - CELL_SIZE
        mov [rStackPtr], eax
        mov [rDepth], 0

        ; Data stack
        mov STKP, dStack

        ; PCIP = IP/PC
        mov PCIP, THE_MEMORY
        ret

; -------------------------------------------------------------------------------------
f_BYE:
            push 0
            call [ExitProcess]
            pop eax
            m_NEXT

; -------------------------------------------------------------------------------------
argError:
        ; invoke s_TYPEz, printArgError
        jmp f_BYE

; -------------------------------------------------------------------------------------
fileError:
        ; invoke s_TYPEz, printFileError, [fileName]
        jmp f_BYE

; -------------------------------------------------------------------------------------
; s_FindWord ( strZ len--(xt flg 1)|(0) )
s_FindWord:
        push edx
        mov edx, [last]
fw01:   cmp edx, 0
        je fwNo
        m_get2ND eax            ; OVER (strZ)
        m_push eax
        m_get2ND eax            ; OVER (len)
        m_push eax
        ; m_push '.'
        ; call s_EMIT
        mov eax, edx
        add eax, 10
        m_push eax              ; name
        movzx eax, BYTE [edx+9]
        m_push eax              ; length
        call s_STREQi
        m_pop eax
        cmp eax, 0
        je fwNxt
        ; m_push 'Y'
        ; call s_EMIT
        m_pop eax
        m_setTOS 0                ; **TODO** (xt)
        m_push 0                  ; **TODO** (flg)
        m_push 1                  ; FOUND
        jmp fw99
fwNxt:  mov edx, [edx]
        jmp fw01
fwNo:   m_pop eax               ; NOT FOUND
        m_setTOS 0
fw99:   pop edx
        ret

; -------------------------------------------------------------------------------------
; s_CComma ( n-- )
s_CComma:
        m_pop eax
        ; **TODO**
        ret;

; -------------------------------------------------------------------------------------
; s_Comma ( n-- )
s_Comma:
        m_pop eax
        ; **TODO**
        ret

; -------------------------------------------------------------------------------------
; s_ParseWord ( strZ len-- )
s_ParseWord:
        m_Save2 ecx, edx
        m_pop ecx
        m_pop edx

pwNUM:  ; NUMBER?
        m_push edx
        m_push ecx
        call s_IsNum
        m_pop eax
        cmp eax, 0
        je pwWd
        cmp [state], 0
        je pw99
        ; Compile the number
        m_push 11               ; LIT4
        call s_CComma
        call s_Comma
        jmp pw99

pwWd:   ; WORD?
        m_push edx
        m_push ecx
        call s_FindWord
        m_pop eax
        cmp eax, 0
        je pwNO
        ; Call or Compile the word
        ;**TODO**
        m_pop eax
        m_pop eax
        m_push 'F'
        call s_EMIT
        ;**TODO**
        jmp pw99

pwNO:   ; NOT found!
        m_push 'N'
        call s_EMIT
        m_push edx
        m_push ecx
        call s_TYPE
        m_push '?'
        call s_EMIT
        m_push '?'
        call s_EMIT
        ; jmp s_Abort
        
pw99:   m_Restore2 ecx, edx
        ret

; -------------------------------------------------------------------------------------
; s_ParseLine ( -- )
s_ParseLine:
        mov [toIn], tib
pl01:   call s_WORD
        cmp TOS, 0
        je pl99
        call s_ParseWord
        jmp pl01
pl99:   m_pop eax
        m_pop eax
        ret

; -------------------------------------------------------------------------------------
s_LoadLine:
        m_push tib
        m_push eax
        call s_STRCPYz
        jmp s_ParseLine

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
entry $
        ; To clean up the stack after calls
        mov [InitialESP], esp
        mov STKP, esp

        ; -10 = SDTIN, -11 = STDOUT, -12 = STDERR
        push STD_INPUT_HANDLE
        call [GetStdHandle]
        mov [STDIN], eax
        mov esp, STKP

        push STD_OUTPUT_HANDLE
        call [GetStdHandle]
        mov [STDOUT], eax
        mov esp, STKP

        invoke __getmainargs, argc, argv, env, 0, stup
        mov esp, STKP
        cmp [argc], 2
        jne go
        mov esi, [argv]
        mov eax, [esi + 4]
        mov [fileName], eax

go:     ; Initialize the VM
        call sys_INIT
        mov [last], xLAST
        mov [here], xHERE

hi:     m_push hello
        call s_TYPEz
        ; m_push [fileName]
        ; call s_TYPEz
        mov eax, str_001
        call s_LoadLine
        mov eax, str_002
        call s_LoadLine
        mov eax, str_003
        call s_LoadLine
        mov eax, str_999
        call s_LoadLine
        ; --TEMP-->>
            m_push ok
            call s_TYPEz
            m_push bye
            call s_TYPEz
        ; <<--TEMP--
        push 0
        call [ExitProcess]

; This is the main CPU Loop
cpuLoop:
        m_NEXT
; -------------------------------------------------------------------------------------
section '.mem' data readable writable
THE_MEMORY:
    m_Define DCell, INLINE, 4, "cell", 0, xCell
        m_Lit1 4
        m_Exit
    m_Define DCells, INLINE, 5, "cells", DCell, xtCells
        m_Lit1 4
        db op_MUL
        m_Exit
    m_Define DExit, INLINE, 6, "(exit)", DCells, xExit
        m_Lit1 op_EXIT
        m_Exit
    m_Define DHereAddr, 0, 6, "(here)", DExit, xHereAddr
        m_Lit4 here
        m_Exit
    m_Define DHere, 0, 4, "here", DHereAddr, xHere
        m_Call xHereAddr
        db op_FETCH
        m_Exit
    m_Define DLastAddr, 0, 6, "(last)", DHere, xLastAddr
        m_Lit4 last
        m_Exit
    m_Define DLast, 0, 4, "last", DLastAddr, xLast
        m_Call xHereAddr
        db op_FETCH
        m_Exit
    m_Define DStateAddr, 0, 5, "state", DLast, xStateAddr
        m_Lit4 state
        m_Exit
    m_Define DCComma, 0, 2, "c,", DStateAddr, xtCComma
        ; **TODO**
        m_Exit
    m_Define DComma, 0, 1, ",", DCComma, xtComma
        ; **TODO**
        m_Exit
    m_Define DDefineWord, 0, 1, ":", DComma, xDefineWord
        db op_DEFINE
        m_Lit1 1
        m_Lit4 state
        m_Lit1 op_STORE
        m_Exit
    m_Define DEndDef, IMMEDIATE, 1, ";", DDefineWord, xEndDef
        c_CComma op_EXIT
        m_Lit1 0
        m_Lit4 state
        m_Lit1 op_STORE
        m_Exit
    m_DefinePrim  DAdd,   1, "+",    DEndDef, xtAdd,   op_ADD
    m_DefinePrim  DSub,   1, "-",    DAdd,    xtSub,   op_SUB
    m_DefinePrim  DMult,  1, "*",    DSub,    xtMult,  op_MUL
    m_DefinePrim  DSlMod, 4, "/mod", DMult,   xtSlMod, op_SLASHMOD
    m_DefinePrim  DAnd,   3, "and",  DSlMod,  xtAnd,   op_AND
    m_DefinePrim  DOr,    2, "or",   DAnd,    xtOr,    op_OR
    m_DefinePrim  DXor,   3, "xor",  DOr,     xtXor,   op_XOR
xLAST:
   m_Define DAbort, 0, 5, "abort", DXor, xtAbort
        ; **TODO**
       m_Exit

xHERE:  rb MEM_SZ
MEM_END:
