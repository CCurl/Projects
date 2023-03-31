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

state     dd 0
here      dd 0
last      dd 0

rStack  dd 256 dup (0)

regs  dd   0 dup (100)        ; 
rbase dd   0                  ; 

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
DOCOL:
    rPUSH esi           ; push current esi on to the return stack
	add eax, 4          ; eax points to codeword, so add 4 to make
	mov esi, eax        ; esi point to first data word
	NEXT

; -------------------------------------------------------------------------------------
DefCode "EXIT",4,0,EXIT
    rPOP esi            ; get esi back
	NEXT

; -------------------------------------------------------------------------------------
DefWord "QUIT",4,0,QUIT
Q1: dd RSPINIT
    dd INTERPRET
    dd BRANCH, Q1

; -------------------------------------------------------------------------------------
DefCode "0RSP",4,0,RSPINIT
    mov ebp, rStack
    NEXT

; -------------------------------------------------------------------------------------
DefWord "INTERPRET",9,0,INTERPRET
    ; **TODO**
    dd EXIT

; -------------------------------------------------------------------------------------
DefCode "BRANCH",9,0,BRANCH
    lodsd
    mov esi, eax
    NEXT

; -------------------------------------------------------------------------------------
DefCode "DUP",3,0,xDUP
    mov eax, TOS
    push eax
    NEXT

; -------------------------------------------------------------------------------------
DefCode "DROP",4,0,DROP
    pop eax
    NEXT

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
section '.mem' data readable writable
MEM:    
xHERE:  rb MEM_SZ
MEM_END:
