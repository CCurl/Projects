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
CELL_SIZE = 4
;
; **************************************************************************

; FOR_OS equ WINDOWS
; FOR_OS equ LINUX

match =WINDOWS, FOR_OS { include 'win-su.inc' }
match =LINUX,   FOR_OS { include 'lin-su.inc' }

; -------------------------------------------------------------------------------------
ver db 'f3 v0.1', 13, 10, 0

InitialESP dd 0
InitialEBP dd 0
SaveEBP    dd 0

here      dd 0
last      dd 0
state     dd 0
base      dd 0

rStack  dd 64 dup (0)

regs  dd  100 dup (0)           ; My pseudo-registers
rbase dd    0

toIn     dd    0                ; >IN - current char ptr
tib      db  128 dup (0)        ; The Text Input Buffer
curWord  db   32 dup (0)        ; The current word
buf4     db    4 dup (0)        ; A buffer for EMIT (LINUX)

; -------------------------------------------------------------------------------------
macro NEXT
{
       lodsd
       jmp DWORD [eax]
}

; ------------------------------------------------------------------------------
macro rPUSH val
{
    add ebp, CELL_SIZE
    mov [ebp], DWORD val
}

; ------------------------------------------------------------------------------
macro rPOP val
{
    mov val, DWORD [ebp]
    sub ebp, CELL_SIZE
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
    Tag: dd cd_#Tag
    cd_#Tag:
}

; ------------------------------------------------------------------------------
LastTag equ 0

MEM_SZ    equ 4*1024*1024
IMMEDIATE equ 1
INLINE    equ 2

; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { include 'win-io.inc' }
match =LINUX,   FOR_OS { include 'lin-io.inc' }

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { section '.code' code readable executable }
match =LINUX,   FOR_OS { segment readable executable }

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
entry $
        mov [InitialESP], esp
        mov [InitialEBP], ebp

        mov ebp, rStack
        
        cld
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
        ; TRC ':'
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
DefWord "CR",2,0,CR
        dd LIT, 13, EMIT, LIT, 10, EMIT, EXIT

; -------------------------------------------------------------------------------------
DefWord "BL",2,0,xtBL
        dd LIT, 32, EXIT

; -------------------------------------------------------------------------------------
DefWord "SPACE",5,0,SPACE
        dd xtBL, EMIT, EXIT

; -------------------------------------------------------------------------------------
DefWord "OK",2,0,OK
        dd SPACE, LIT, 'O', EMIT, LIT, 'K', EMIT
        dd CR, EXIT

; -------------------------------------------------------------------------------------
DefWord "INTERPRET",9,0,INTERPRET
        dd OK
        dd TIB, LIT, 128, ACCEPT, DROP
        dd TIB, TOIN, xtSTORE
in01:   dd xtWORD, fDUP, zBRANCH, inX
        dd LIT, '[', EMIT, TYPE, LIT, ']', EMIT ; **TEMP**
        ; **TODO**
        dd BRANCH, in01
inX:    dd DROP2, EXIT

; -------------------------------------------------------------------------------------
DefWord "TIB",3,0,TIB
        dd LIT, tib, EXIT

; -------------------------------------------------------------------------------------
DefWord ">IN",3,0,TOIN
        dd LIT, toIn, EXIT

; -------------------------------------------------------------------------------------
DefCode "WORD",4,0,xtWORD       ; ( --addr len )
        mov ebx, curWord
        push ebx                ; addr
        push DWORD 0            ; len
        mov edx, [toIn]
        xor eax, eax
wd01:   mov al, [edx]           ; Skip whitespace
        cmp al, 32
        jg wd02
        cmp al, 13
        je wdX
        cmp al, 10
        je wdX
        cmp al, 0
        je wdX
        inc edx
        jmp wd01
wd02:   mov [ebx], al           ; Collect word
        inc ebx
        inc edx
        inc DWORD TOS           ; Increment len
        mov al, [edx]           ; Next char
        cmp al, 33
        jl wdX
        cmp TOS, DWORD 32
        jl wd02
wdX:    mov [toIn], edx
        mov [ebx], BYTE 0
        NEXT

; -------------------------------------------------------------------------------------
DefCode "ACCEPT",6,0,ACCEPT     ; ( addr sz--num )
        ACCEPTx
        ; add eax, 65
        ; TRC eax
        NEXT

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
DefCode "!",1,0,xtSTORE
        pop edx
        pop eax
        mov [edx], eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "C!",2,0,CSTORE
        pop edx
        pop eax
        mov [edx], al
        NEXT

; -------------------------------------------------------------------------------------
DefCode "+",1,0,xtADD
        pop edx
        pop eax
        add eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "-",1,0,xSUB
        pop edx
        pop eax
        sub eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "*",1,0,xMUL
        pop edx
        pop eax
        mul edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "/",1,0,xDIV
        pop ebx
        pop eax
        xor edx, edx
        idiv ebx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "/MOD",4,0,xSLMOD
        pop ebx
        pop eax
        xor edx, edx
        idiv ebx
        push edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "KEY",3,0,KEY         ; ( ch-- )
        KEYx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "KEY?",4,0,KEYQ         ; ( ch-- )
        KEYq
        NEXT

; -------------------------------------------------------------------------------------
DefCode "EMIT",4,0,EMIT         ; ( ch-- )
        EMITx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "TYPE",4,0,TYPE         ; ( addr len-- )
        TYPEx
        NEXT
; -------------------------------------------------------------------------------------
DefWord "WORDS",5,0,WORDS
        ; **TODO**
        dd EXIT

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { section '.mem' readable writable }
match =LINUX,   FOR_OS { segment readable writable }

MEM:    
xHERE:  rb MEM_SZ
MEM_END:
