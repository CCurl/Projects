; **************************************************************************
;
; This is a program that implements a Forth CPU/VM.
; The VM's memory space is the reserved buffer at THE_MEMORY.
;
; **************************************************************************
match =WINDOWS, FOR_OS { include 'win-su.inc' }
match =LINUX,   FOR_OS { include 'lin-su.inc' }

; **************************************************************************
;                 x86 Register usage
; **************************************************************************
;
IP   equ esi       ; Program-Counter/Instruction-Pointer
SP   equ edi       ; Data Stack-Pointer
RSP  equ ebp       ; Return Stack-Pointer
TOS  equ [SP]      ; Top Of Stack
RTOS equ [RSP]     ; Top Of Return Stack
CELL_SIZE = 4

; ------------------------------------------------------------------------------
macro dPUSH val
{
    add SP, CELL_SIZE
    mov TOS, DWORD val
}

; ------------------------------------------------------------------------------
macro dPOP val
{
    mov val, DWORD TOS
    sub SP, CELL_SIZE
}

; ------------------------------------------------------------------------------
macro rPUSH val
{
    add RSP, CELL_SIZE
    mov RTOS, DWORD val
}

; ------------------------------------------------------------------------------
macro rPOP val
{
    mov val, DWORD RTOS
    sub RSP, CELL_SIZE
}

MEM_SZ    equ 4*1024*1024
IMMEDIATE equ 1
INLINE    equ 2

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { section '.code' code readable executable }
match =LINUX,   FOR_OS { segment readable executable }

match =WINDOWS, FOR_OS { include 'win-io.inc' }
match =LINUX,   FOR_OS { include 'lin-io.inc' }

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
entry $
cold:   mov [InitialESP], esp
        mov [InitialEBP], ebp
        cld

        mov SP, dStack
        mov RSP, rStack

        ; Temp
        dPUSH 65
        call xtEMIT
        push 0
        call [ExitProcess]
        ; invoke  ExitProcess, 0
        ; call xtBYE
        ret

;       mov [v_HERE], HereInit
;       mov [v_LAST], LastInit
        mov esi, CODE

        jmp wcRUN

; -------------------------------------------------------------------------------------
wcPRIM: call eax
wcRUN:  lodsd
        cmp eax, PRIMEND
        jl wcPRIM
        cmp eax, $F0000000
        jge wcNUM
wcWORD: rPUSH IP
        mov IP, eax
        jmp wcRUN
wcNUM:  and eax, $0FFFFFFF
        dPUSH eax
        jmp wcRUN

; -------------------------------------------------------------------------------------
xtEXIT: rPOP IP
        ret

; -------------------------------------------------------------------------------------
xtBYE:  pop eax
        ret

; -------------------------------------------------------------------------------------
; DefCode "BRANCH",6,0,BRANCH
xtBRANCH: lodsd
xBR:    mov IP, eax
        ret

; -------------------------------------------------------------------------------------
; DefCode "0BRANCH",7,0,zBRANCH
xt0BRANCH:
        lodsd
        dPOP edx
        test edx, edx
        jz xBR
        ret

; -------------------------------------------------------------------------------------
; DefCode "?BRANCH",7,0,nzBRANCH
xtQBRANCH:
        lodsd
        dPOP edx
        test edx, edx
        jnz xBR
        ret

; -------------------------------------------------------------------------------------
xtDUP:  mov eax, TOS
doDUP:  dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtQDUP: mov eax, TOS
        test eax, eax
        jnz doDUP
        ret

; -------------------------------------------------------------------------------------
xtSWAP: dPOP eax
        dPOP ebx
        dPUSH eax
        dPUSH ebx
        ret

; -------------------------------------------------------------------------------------
xtOVER: dPOP eax
        mov ebx, TOS
        dPUSH eax
        dPUSH ebx
        ret

; -------------------------------------------------------------------------------------
xtDROP: dPOP eax
        ret

; -------------------------------------------------------------------------------------
xtLIT:  lodsd
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtINC:  inc DWORD TOS
        ret

; -------------------------------------------------------------------------------------
xtDEC:  dec DWORD TOS
        ret

; -------------------------------------------------------------------------------------
xtFET:  mov edx, TOS
        mov eax, [edx]
        mov TOS, eax
        ret

; -------------------------------------------------------------------------------------
xtCFET: mov edx, TOS
        movzx eax, BYTE [edx]
        mov TOS, eax
        ret

; -------------------------------------------------------------------------------------
xtSTO:  dPOP edx
        dPOP eax
        mov [edx], eax
        ret

; -------------------------------------------------------------------------------------
xtCSTO: dPOP edx
        dPOP eax
        mov [edx], al
        ret

; -------------------------------------------------------------------------------------
xtADD:  dPOP edx
        dPOP eax
        add eax, edx
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtSUB:  dPOP edx
        dPOP eax
        sub eax, edx
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtMUL:  dPOP edx
        dPOP eax
        mul edx
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtDIV:  dPOP ebx
        dPOP eax
        xor edx, edx
        idiv ebx
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtLT:   dPOP ebx
        dPOP eax
        cmp eax, ebx
        jl fTrue
        jmp fFalse

; -------------------------------------------------------------------------------------
xtEQ:   dPOP ebx
        dPOP eax
        cmp eax, ebx
        je fTrue
fFalse: dPUSH 0
        ret
fTrue:  dPUSH 1
        ret

; -------------------------------------------------------------------------------------
xtGT:   dPOP ebx
        dPOP eax
        cmp eax, ebx
        jg fTrue
        jmp fFalse

; -------------------------------------------------------------------------------------
xtSLMOD:
        dPOP ebx
        dPOP eax
        xor edx, edx
        idiv ebx
        dPUSH edx
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtLOOP: mov edx, [lSP]
        inc DWORD [edx]                         ; I
        mov ecx, DWORD [edx]
        cmp ecx, DWORD [edx-CELL_SIZE]          ; TO
        jge lpDone
        mov esi, DWORD [edx-CELL_SIZE*2]        ; Loop start
        ret
lpDone: sub edx, CELL_SIZE*3
        mov [lSP], DWORD edx
        ret

; -------------------------------------------------------------------------------------
xtTOR:  dPOP eax
        rPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtRAT:  mov eax, DWORD RTOS
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
xtRFROM: rPOP eax
        dPUSH eax
        ret

; -------------------------------------------------------------------------------------
PRIMEND:

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { section '.mem' readable writable }
match =LINUX,   FOR_OS { segment readable writable }

; -------------------------------------------------------------------------------------
ver db 'f3 v0.1', 13, 10, 0

InitialESP dd 0
InitialEBP dd 0

dStack   dd 64 dup (0)          ; The data stack
rStack   dd 64 dup (0)          ; The return stack
lStack   dd 64 dup (0)          ; The loop stack
lSP      dd lStack              ; The loop stack-pointer

regs     dd  100 dup (0)        ; My pseudo-registers
rbase    dd    0

toIn     dd    0                ; >IN - current char ptr
tib      db  128 dup (0)        ; The Text Input Buffer
curWord  db   32 dup (0)        ; The current word
buf4     db    4 dup (0)        ; A buffer for EMIT (LINUX)

isNeg    db  0
dotLen   db  0
dotPtr   dd  0
dotBuf   db 64 dup (0)

v_HERE:  dd HereInit
v_LAST:  dd LastInit
v_STATE: dd 0
v_BASE:  dd 10

; ------------------------------------------------------------------------------
macro DefWord Name, Length, Flags, XT
{
        dd XT          ; XT / Code-Field-Address (CFA)
        db Flags       ; Flags
        db Length      ; Length
        db Name        ; Name
        db 0           ; NULL-terminator
}

CODE:

xtGO:   dd xtOK, xtBYE
xtCR:   dd $F000000D, xtEMIT, $F000000A, xtEMIT, xtEXIT
xtBL:   dd $F0000020, xtEXIT
xtSPC:  dd xtBL, xtEMIT, xtEXIT
xtOK:   dd xtSPC, $F000006F, xtEMIT, $F000006B, xtEMIT, xtCR, xtEXIT
xtHA:   dd xtLIT, v_HERE, xtEXIT
xtHERE: dd xtHA, xtFET, xtEXIT
xtLA:   dd xtLIT, v_LAST, xtEXIT
xtLAST: dd xtLA, xtFET, xtEXIT
HereInit:
    db 1024 dup (0)

CODEEND:

DICT:
    DefWord "LA", 2, 0, xtLA
    DefWord "HA", 2, 0, xtHA
LastInit:
    DefWord "EXIT", 4, 0, xtEXIT
    db 1024 dup (0)
DICTEND:
    db    32 dup (0)
