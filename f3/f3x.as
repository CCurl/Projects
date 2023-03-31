; **************************************************************************
;
; This is a program that implements a Forth CPU/VM.
; The VM's memory space is the reserved buffer at THE_MEMORY.
;
; **************************************************************************
;                 x86 Register usage
; **************************************************************************
;
IP   equ esi       ; Program-Counter/Instruction-Pointer
SP   equ esp       ; Stack-Pointer
RSP  equ ebp       ; Return Stack-Pointer
TOS  equ [esp]     ; Top Of Stack
;
; **************************************************************************

format PE console 

include 'win32ax.inc'
library msvcrt, 'msvcrt.dll'
import msvcrt, putchar, 'putchar'

; -------------------------------------------------------------------------------------
macro NEXT
{
       lodsd
       jmp eax
}

; ------------------------------------------------------------------------------
macro rPUSH val
{
    mov [ebp], DWORD val
    add ebp, 4
}

; ------------------------------------------------------------------------------
macro rPOP val
{
    mov val, [ebp]
    sub ebp, 4
}

; ------------------------------------------------------------------------------
macro DefWord Name, Length, Flags, Tag
{
    d#Tag: dd LastTag     ; Link
           dd Tag         ; XT
           db Flags       ; Flags
           db Length      ; Length
           db Name        ; Name
           db 0           ; NULL-terminator
    LastTag equ Tag
    align CELL_SIZE
    Tag:   dd DOCOL
}

; ------------------------------------------------------------------------------
macro DefCode Name, Length, Flags, Tag
{
    d#Tag: dd LastTag     ; Link
           dd Tag         ; XT
           db Flags       ; Flags
           db Length      ; Length
           db Name        ; Name
           db 0           ; NULL-terminator
    LastTag equ Tag
    align CELL_SIZE
    Tag:
}

; ------------------------------------------------------------------------------
LastTag equ 0

MEM_SZ    equ 4*1024*1024
IMMEDIATE equ 1
INLINE    equ 2

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
section '.bss' data readable writable

ver db 'f3 v0.1', 13, 10, 0

CELL_SIZE = 4
InitialESP dd 0

here      dd 0
last      dd 0
state     dd 0
base      dd 0

rStack  dd 256 dup (0)

regs  dd   0 dup (100)        ; My pseudo-registers
rbase dd   0

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
section '.code' code readable executable

; -------------------------------------------------------------------------------------
entry $
        mov [InitialESP], esp
        mov ebp, rStack
        
        mov esi, coldStart
        NEXT

; -------------------------------------------------------------------------------------
coldStart:
    dd QUIT

; -------------------------------------------------------------------------------------
DefWord "QUIT",4,0,QUIT
q1:     dd zRSP
        dd INTERPRET
        dd BRANCH, q1

; -------------------------------------------------------------------------------------
DOCOL:
        rPUSH esi           ; push current esi on to the return stack
        add eax, CELL_SIZE  ; eax points to codeword, so add (CELL_SIZE) to make
        mov esi, eax        ; esi point to first data word
        NEXT

; -------------------------------------------------------------------------------------
DefCode "EXIT",4,0,EXIT
        rPOP esi            ; get esi back
        NEXT

; -------------------------------------------------------------------------------------
DefCode "0RSP",4,0,zRSP
        mov ebp, rStack
        NEXT

; -------------------------------------------------------------------------------------
DefCode "0SP",3,0,zSP
        mov esp, [InitialESP]
        NEXT

; -------------------------------------------------------------------------------------
DefWord "INTERPRET",9,0,INTERPRET
        ; **TODO**
        dd LIT, 'I', EMIT
        dd EXIT

; -------------------------------------------------------------------------------------
DefCode "BRANCH",6,0,BRANCH
        lodsd
        mov esi, eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "0BRANCH",7,0,zBRANCH
        lodsd
        pop edx
        test edx, edx
        jnz zBX
        mov esi, eax
zBX:    NEXT

; -------------------------------------------------------------------------------------
DefCode "?BRANCH",7,0,nzBRANCH
        lodsd
        pop edx
        test edx, edx
        jz nzBX
        mov esi, eax
nzBX:   NEXT

; -------------------------------------------------------------------------------------
DefCode "DUP",3,0,fDUP
        mov eax, TOS
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "?DUP",4,0,qDUP
        mov eax, TOS
        test edx, edx
        jz qdX
        push eax
qdX:    NEXT

; -------------------------------------------------------------------------------------
DefCode "SWAP",4,0,SWAP
        pop eax
        pop ebx
        push eax
        push ebx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "OVER",4,0,OVER
        pop eax
        mov ebx, TOS
        push eax
        push ebx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "DROP",4,0,DROP
        pop eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "2DROP",5,0,DROP2
        pop eax
        pop eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "LIT",3,0,LIT
        lodsd
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "1+",2,0,INCTOS
        inc DWORD TOS
        NEXT

; -------------------------------------------------------------------------------------
DefCode "1-",2,0,DECTOS
        dec DWORD TOS
        NEXT

; -------------------------------------------------------------------------------------
DefCode "@",1,0,FETCH
        mov edx, TOS
        mov eax, [edx]
        mov TOS, eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "C@",2,0,CFETCH
        mov edx, TOS
        movzx eax, BYTE [edx]
        mov TOS, eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "!",1,0,STORE
        pop edx
        pop eax
        move [edx], eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "C!",2,0,CSTORE
        pop edx
        pop eax
        move [edx], al
        NEXT

; -------------------------------------------------------------------------------------
DefCode "+",1,0,fADD
        pop edx
        pop eax
        add eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "-",1,0,fSUB
        pop edx
        pop eax
        sub eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "*",1,0,fMUL
        pop edx
        pop eax
        mul eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "/",1,0,fDIV
        pop edx
        pop eax
        ; **TODO**
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "/MOD",4,0,fSLMOD
        pop edx
        pop eax
        ; **TODO**
        push eax
        push edx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "EMIT",4,0,EMIT         ; ( ch-- )
        call [putchar]
        pop eax
        NEXT

; -------------------------------------------------------------------------------------
; : type ( addr count-- ) ?dup if begin over c@ emit swap 1+ swap 1- dup while then 2drop ;
DefWord "TYPE",4,0,TYPE
        dd qDUP, zBRANCH, tyX
tyS:    dd OVER, CFETCH, EMIT
        dd SWAP, INCTOS, SWAP, DECTOS
        dd fDUP, nzBRANCH, tyS
tyX:    dd DROP2, EXIT

; -------------------------------------------------------------------------------------
DefCode "WORDS",5,0,WORDS
        ; **TODO**
        dd EXIT

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
section '.mem' data readable writable
MEM:    
xHERE:  rb MEM_SZ
MEM_END:
