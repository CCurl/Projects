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
        d_#Tag: dd LastTag     ; Link
                dd Tag         ; XT / Code-Field-Address (CFA)
                db Flags       ; Flags
                db Length      ; Length
                db Name        ; Name
                db 0           ; NULL-terminator
        align CELL_SIZE
        LastTag equ d_#Tag
        Tag:    dd  DOCOL
}

; ------------------------------------------------------------------------------
macro DefVar Name, Length, Flags, Tag
{
        d_#Tag: dd LastTag      ; Link
                dd Tag          ; XT / Code-Field-Address (CFA)
                db Flags        ; Flags
                db Length       ; Length
                db Name         ; Name
                db 0            ; NULL-terminator
        align CELL_SIZE
        LastTag equ  d_#Tag
        Tag:    dd   c_#Tag
        c_#Tag: push v_#Tag
        NEXT
}

; ------------------------------------------------------------------------------
macro DefCode Name, Length, Flags, Tag
{
        d_#Tag: dd LastTag     ; Link
                dd Tag         ; XT / Code-Field-Address (CFA)
                db Flags       ; Flags
                db Length      ; Length
                db Name        ; Name
                db 0           ; NULL-terminator
        align CELL_SIZE
        LastTag equ  d_#Tag
        Tag:    dd   c_#Tag
        c_#Tag:
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
        mov [lSP], lStack
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
DefWord "BL",2,0,fBL
        dd LIT, 32, EXIT

; -------------------------------------------------------------------------------------
DefWord "SPACE",5,0,SPACE
        dd fBL, EMIT, EXIT

; -------------------------------------------------------------------------------------
DefWord "OK",2,0,OK
        dd SPACE, LIT, 'O', EMIT, LIT, 'K', EMIT
        dd CR, EXIT

; -------------------------------------------------------------------------------------
DefWord "BENCH",5,0,BENCH
        dd LIT, 's', EMIT
        dd LIT, 500000000, LIT, 0, xtDO, xtLOOP
        dd LIT, 'e', EMIT
        dd EXIT

; -------------------------------------------------------------------------------------
DefWord "INTERPRET",9,0,INTERPRET
        dd OK
        dd TIB, LIT, 128, ACCEPT, DROP
        dd TIB, TOIN, fSTORE
        dd BENCH
in01:   dd xtWORD, xtDUP, zBRANCH, inX
        dd NUMQ, zBRANCH, in02
        dd DROP                 ; TODO: compile?
        dd BRANCH, in01
in02:   ; It is in the dictionary?
        dd FIND, zBRANCH, inERR
        dd DROP2                ; TODO: compile or execute
        dd BRANCH, in01
inERR:  dd LIT, '[', EMIT, TYPE, LIT, ']', EMIT
        dd LIT, '?', xtDUP, EMIT, EMIT
        dd QUIT
inX:    dd DROP2, EXIT

; -------------------------------------------------------------------------------------
DefCode "NUMBER?",7,0,NUMQ         ; ( a n--(num 1)|(a n 0) )
        ; **TODO**
        push 0
        NEXT

; -------------------------------------------------------------------------------------
toLower: ; makes dl lowercase if between A-Z
        cmp dl, 'A'
        jl tlX
        cmp dl, 'Z'
        jg tlX
        add dl, 0x20
tlX:    ret

; -------------------------------------------------------------------------------------
; strEqI: Case insenstive compare
;         Params: eax: str1, ebx, len1
;                 ecx: str2, edx: len2
;         Returns: eax = 0: Not equal, eax != 0 equal
strEqI: cmp ebx, edx            ; Lens the same?
        jne eqiNo
eqi01:  test ebx, ebx
        jz eqiYes
        mov dl, [eax]           ; char1
        call toLower
        mov dh, dl
        mov dl, [ecx]           ; char 2
        call toLower
        cmp dl, dh
        jne eqiNo
        inc eax
        inc ecx
        dec ebx
        jmp eqi01
eqiNo:  xor eax, eax
        ret
eqiYes: mov eax, 1
        ret

; -------------------------------------------------------------------------------------
DefCode "FIND",4,0,FIND         ; ( a n--(xt f 1)|(a n 0) )
        pop edx                 ; len1
        pop ecx                 ; name1
        mov eax, [v_LAST]
fw01:   test eax, eax           ; end of dictionary?
        jz fwNo
        push edx                ; strEqI stomps on these
        push ecx
        push eax
        add eax, CELL_SIZE*2+1  ; add length offset
        movzx ebx, BYTE [eax]   ; len2
        inc eax                 ; name2
        call strEqI
        pop ebx                 ; current dict entry (was eax)
        pop ecx                 ; len1
        pop edx                 ; name1
        test eax, eax           ; Not 0 means they are equal
        jnz fwYes
        mov eax, DWORD [ebx]    ; Not equal, move to the next word
        jmp fw01
fwNo:   push ecx                ; Not found, return (--name len 0)
        push edx
        push 0
        NEXT
fwYes:  add ebx, CELL_SIZE      ; Offset to XT
        push DWORD [ebx]        ; XT
        add ebx, CELL_SIZE      ; Offset to Flags
        movzx eax, BYTE [ebx]   ; Flags
        push eax
        push 1                  ; FOUND!
        NEXT

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
DefCode "DUP",3,0,xtDUP
        mov eax, TOS
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "2DUP",4,0,DUP2
        pop ebx
        pop eax
        push eax
        push ebx
        push eax
        push ebx
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
DefCode "!",1,0,fSTORE
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
DefCode "-",1,0,xtSUB
        pop edx
        pop eax
        sub eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "*",1,0,xtMUL
        pop edx
        pop eax
        mul edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "/",1,0,xtDIV
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
DefCode "TIMER",5,0,TIMER     ; ( --n )
        ioTIMER
        NEXT

; -------------------------------------------------------------------------------------
DefWord "WORDS",5,0,WORDS
        ; **TODO**
        dd EXIT

; -------------------------------------------------------------------------------------
DefCode "DO", 2, 0, xtDO
        mov edx, [lSP]
        pop ecx                 ; From / I
        pop ebx                 ; To
        add edx, CELL_SIZE
        mov [edx], DWORD esi    ; loop start
        add edx, CELL_SIZE
        mov [edx], DWORD ebx
        add edx, CELL_SIZE
        mov [edx], DWORD ecx
        mov [lSP], DWORD edx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "LOOP", 4, 0, xtLOOP
        mov edx, [lSP]
        mov ecx, DWORD [edx]            ; I
        inc ecx
        mov [edx], ecx
        cmp ecx, DWORD [edx-CELL_SIZE]  ; TO
        jge lpDone
        mov esi, DWORD [edx-CELL_SIZE*2]            ; Loop start
        NEXT
lpDone: sub edx, CELL_SIZE*3
        mov [lSP], DWORD edx
        NEXT

; -------------------------------------------------------------------------------------
; Some variables ...
; -------------------------------------------------------------------------------------
DefVar "(HERE)",6,0,HERE
DefVar "(LAST)",6,0,LAST
DefVar "STATE",5,0,STATE
DefVar "BASE",4,0,BASE
DefVar "PAD",4,0,PAD

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { section '.bdata' readable writable }
match =LINUX,   FOR_OS { segment readable writable }

; -------------------------------------------------------------------------------------
ver db 'f3 v0.1', 13, 10, 0

InitialESP dd 0
InitialEBP dd 0
SaveEBP    dd 0

rStack   dd 64 dup (0)           ; The return stack
lStack   dd 64 dup (0)           ; The loop stack
lSP      dd lStack               ; The loop stack pointer

regs     dd  100 dup (0)           ; My pseudo-registers
rbase    dd    0

toIn     dd    0                ; >IN - current char ptr
tib      db  128 dup (0)        ; The Tef Input Buffer
curWord  db   32 dup (0)        ; The current word
buf4     db    4 dup (0)        ; A buffer for EMIT (LINUX)

v_HERE:  dd xHERE
v_LAST:  dd LastTag
v_STATE: dd 0
v_BASE:  dd 10
v_PAD:   dd 64 dup(0)

xHERE:
MEM:     rb MEM_SZ  
MEM_END:
