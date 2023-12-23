format ELF64 executable 3

; Linux ABI:
;     Volatile registers: RAX, RCX, RDX, R8, R9, R10, R11, RSI, RDI
;     Non-volatile registers: RBX, RSP, RBP, R12, R13, R14, R15

; Windows ABI:
;     Volatile registers: RAX, RCX, RDX, R8, R9, R10, R11
;     Non-volatile registers: RBX, RBP, RSP, R12, R13, R14, R15, RDI, RSI



segment readable executable
entry cold

; ------------------------------------------------------------------------------
CELL_SZ    =   8
CODE_SZ    =  64*1024
VARS_SZ    = 128*1024
DICT_SZ    =  32*1024          ; 1024 entries
DSTK_SZ    =  64*CELL_SZ
RSTK_SZ    =  64*CELL_SZ
LSTK_SZ    =  30*CELL_SZ
TIB_SZ     = 256

; INDEX    equ r11
DSP        equ r12
RSPTR      equ r13
PC         equ r14
TOS        equ r15

STDIN      = 0
STDOUT     = 1
SYS_READ   = 0
SYS_WRITE  = 1

; Dictionary entry
; [xt:CELL_SZ][flags:1][len:1][name:NAME_SZ][null:1]
XT_OFF     = 0
FLG_OFF    = CELL_SZ
LEN_OFF    = FLG_OFF+1
NAME_OFF   = LEN_OFF+1
NAME_SZ    = 21

; ------------------------------------------------------------------------------
macro dPUSH val {
    add DSP, CELL_SZ
    mov [DSP], TOS
    mov TOS, QWORD val
}

; ------------------------------------------------------------------------------
macro dPOP val {
    mov val, TOS
    mov TOS, [DSP]
    sub DSP, CELL_SZ
}

; ------------------------------------------------------------------------------
macro rPUSH val {
    add RSPTR, CELL_SZ
    mov [RSPTR], qword val
}

; ------------------------------------------------------------------------------
macro rPOP val {
    mov val, [RSPTR]
    sub RSPTR, CELL_SZ
}

; ------------------------------------------------------------------------------
macro CComma val {
    mov r10, [here]
    mov [r10], byte val
    inc qword [here]
}

; ------------------------------------------------------------------------------
macro Comma val {
    mov r10, [here]
    mov qword ptr r10, val
    add qword ptr here, CELL_SZ
}

; ------------------------------------------------------------------------------
macro nextCell val {
    mov val, qword [PC]
    add PC, CELL_SZ
}

; ------------------------------------------------------------------------------
macro mLit val {
    Comma _lit
    Comma val
}

; ------------------------------------------------------------------------------
;macro NEXT { ret }
macro NEXT { jmp runJ }

macro mEmit val {
        mLit val
        Comma _emit
}

; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
cold:
        mov DSP, dstk             ; Start of data STACK
        mov RSPTR, rstk           ; Start of return STACK
        lea rax, [lstk]           ; Start of loop STACK
        mov [lsp], rax
        mov qword ptr last, lastA
        ;mov rax, lastA           ; LAST
        ;mov [last], rax
        mov qword ptr here, hereA
        ;mov [here], rax          ; HERE: start of word-code
        lea rax, [regs]           ; Start of registers
        mov [regBase], rax
        mov qword ptr base, 10
        mov qword ptr state, 0

        mov rsi, hi
        call strlen
        call stype

        jmp _repl

; ------------------------------------------------------------------------------
prim:   call rax
runC:   nextCell rax
        cmp rax, primEnd
        jl prim
        btr rax, 1          ; Must be a "word-code" ...
        jc .1               ; Bit #1 ON means JMP
        rPUSH PC            ; Bit #1 OFF means CALL
.1:     mov PC, rax
        jmp runC

; ------------------------------------------------------------------------------
runJ:   nextCell rax
        cmp rax, wcStart
        jge .wc
        ;test rax, rax
        ;jz .x
        jmp rax
.wc:    btr rax, 1          ; Must be a "word-code" ...
        jc .1               ; Bit #1 ON means JMP
        rPUSH PC            ; Bit #1 OFF means CALL
.1:     mov PC, rax
        jmp runJ
.x:     ret

; ------------------------------------------------------------------------------
wcTest:
        mov PC, [here]
        push PC
        ; Some WordCode for testing ...
        mLit hi
        Comma _qtype

        mLit 'R'
        mLit 1
        Comma _add
        Comma _emit
        mEmit 10

        mLit 1000
        mLit 1000
        Comma _dup
        Comma _mult
        Comma _mult
        mLit 0
        Comma _do
        Comma _loop

        mLit 'F'
        mLit 1
        Comma _sub
        Comma _emit
        mEmit 10

        mLit -1
        mLit  9
        Comma _do
        Comma _index
        Comma _dot
        Comma _loop2

        Comma _stop
        call runJ
        pop PC
        mov [here], PC
        ret

; ------------------------------------------------------------------------------
; readLine: read 1 line into [RSI], size in RDX
; ------------------------------------------------------------------------------
readLine:
        mov rax, SYS_READ
        mov rdi, STDIN
        push r11
        syscall
        pop r11
        ret

; ------------------------------------------------------------------------------
; out: RSI: the word, RDX: length
; ------------------------------------------------------------------------------
nextWord: cld
        lea rdi, [wd]
        mov rsi, qword [toIn]
        xor rdx, rdx
        cmp byte [esi], 0
        je .9
.1:     lodsb               ; Skip WS
        test al, al
        jz .9
        cmp al, 33
        jl .1
.2:     stosb               ; Build the word
        inc rdx             ; RDX: len
        lodsb
        cmp al, 32
        jg .2
.9:     xor al, al          ; Done
        stosb
        mov [toIn], esi     ; word is in RSI, len in RDX
        lea rsi, [wd]
        ret

; ------------------------------------------------------------------------------
; Dict Entry: 32 bytes ... [xt:8] [flags:1] [len:1] [name:21] [null:1]
; Word is in RSI
; ------------------------------------------------------------------------------
addToDict:
        call strlen
        test dl, dl
        jz .r
        push qword [here]      ; For the XT
        mov r9, [last]
        add r9, 32
        mov [last], r9
        pop qword ptr r9       ; XT
        add r9, CELL_SZ
        mov byte ptr r9, 0     ; Flags
        inc r9
        cmp dl, NAME_SZ        ; Check len
        jle .len
        mov dl, NAME_SZ        ; trunc to 21
.len:   mov byte ptr r9, dl    ; Len
        inc r9
.nm:    lodsb                  ; Name
        mov byte ptr r9, al
        inc r9
        dec dl
        jnz .nm
        mov byte ptr r9, 0     ; NULL
.r:     ret

; ------------------------------------------------------------------------------
; Find in dictionary
; In: Name in RSI
; Out: RBX=(addr of entry), 0 if not found
; ------------------------------------------------------------------------------
findInDict:
        mov rbx, [last]
.1:     cmp rbx, dStart      ; End of list?
        jl .nf
        mov rdi, rbx
        add rdi, NAME_OFF
        ;push rbx
        ;push rdi
        ;push rsi
        ;mov rsi, rdi
        ;call strlen
        ;call stype
        ;mov al, 32
        ;call semit
        ;pop rsi
        ;pop rdi
        ;pop rbx
        call strcmpi
        jnc .2               ; Carry Clear = not equal
        ret
.2:     sub rbx, 32
        jmp .1
.nf:    xor rbx, rbx
        ret

; ------------------------------------------------------------------------------
; C3 subroutines
; ------------------------------------------------------------------------------
; input:        ; rax: the number to print - destroyed
;               : rbx: base
; output:       ; rsi: the start of the string
;               ; rdx: the length of the string
iToA:   xor     rcx, rcx        ; output length
        test    rbx, rbx        ; Set base to 10 if 0
        jnz     .0
        mov     rbx, 10
.0:     mov     rsi, buf2+63    ; output string start
        mov     BYTE [rsi], 0
        push    0               ; isNegative flag
        cmp     rbx, 10         ; base 10 only
        jne     .1
        bt      rax, 63
        jnc     .1
        inc     BYTE [rsp]
        neg     rax
.1:     mov     rdx, 0          ; loop starts here
        div     rbx             ; RDX <- remainder, RAX <- quotient
        add     dl, '0'
        cmp     dl, '9'
        jle     .2
        add     dl, 7           ; Make it HEX
.2:     dec     esi
        mov     BYTE [esi], dl
        inc     rcx
        test    rax, rax        ; Stop when RAX=0
        jnz     .1
        pop     rax
        test    rax, rax
        jz      .x
        dec     esi
        mov     BYTE [rsi], '-'
        inc     rcx
.x:     mov     rdx, rcx
        ret

semit:                         ; The char to EMIT is in al
        mov rsi, buf2
        mov [rsi], al
        mov rdx, 1             ; len=1

; ------------------------------------------------------------------------------
; The string is in RSI, length is in RDX
; ------------------------------------------------------------------------------
stype:
        mov rdi, STDOUT
        mov rax, SYS_WRITE
        push r11
        syscall
        pop r11
        ret

; ------------------------------------------------------------------------------
; The string is in RSI, put the length into RDX
; ------------------------------------------------------------------------------
strlen: xor rdx, rdx
.1:     cmp byte [rsi+rdx], 0
        je .9
        inc rdx
        jmp .1
.9:     ret

; ------------------------------------------------------------------------------
; Make AL lower-case if it is UPPER-CASE
; ------------------------------------------------------------------------------
lower:  cmp al, 'A'
        jl .9
        cmp al, 'Z'
        jg .9
        add al, 32
.9:     ret

; ------------------------------------------------------------------------------
; The strings are in RSI and RDI
; Return: IF strings are equal Carry=1,  ELSE Carry=0
; ------------------------------------------------------------------------------
strcmpi: xor rdx, rdx
.1:     mov al, byte [rsi+rdx]
        call lower
        mov ah, al
        mov al, byte [rdi+rdx]
        call lower
        cmp al, ah
        jne _clc
        inc rdx
        test al, al
        jnz .1
_stc:   stc
        ret
_clc:   clc
        ret

; ------------------------------------------------------------------------------
; The strings are in RSI and RDI
; Return: IF strings are equal Carry=1,  ELSE Carry=0
; ------------------------------------------------------------------------------
strcmp: xor rdx, rdx
.1:     mov al, byte [rsi+rdx]
        mov ah, al
        mov al, byte [rdi+rdx]
        cmp al, ah
        jne _clc
        inc rdx
        test al, al
        jnz .1
        jmp _stc

; ------------------------------------------------------------------------------
; The char is in AL
; Return: IF AL in ['0'..'9'], Carry=1, else Carry=0
; ------------------------------------------------------------------------------
is09:   cmp al, '0'
        jl .n
        cmp al, '9'
        jg .n
        sub al, '0'
        stc                 ; AL is between '0' and '9'
        ret
.n:     clc                 ; Not between '0' and '9'
        ret

_r1:    mov rax, 1
        ret

; ------------------------------------------------------------------------------
; The word is in RSI, length is in RDX
; Return: IF number, RAX=1 and number in TOS, ELSE RAX=0
; ------------------------------------------------------------------------------
isNum:
        dPUSH 0
        xor rax, rax
.1:     lodsb
        test al, al
        jz _r1
        call is09
        jnc .no
        imul TOS, 10
        add TOS, rax
        jmp .1
.no:    dPOP rax
        xor rax, rax
        ret

; ------------------------------------------------------------------------------
; The word is in RSI, length is in RDX
; ------------------------------------------------------------------------------
_parseWord:
        cmp byte [rsi], 'X'     ; Test?
        jne .num
        call wcTest
        jmp .r0

.num:   push rsi               ; Number?
        push rdx
        call isNum
        pop rdx
        pop rsi
        test rax, rax
        jz .wd
        cmp qword ptr state, 0
        je .r0
        dPOP rax
        mLit rax
.r0:    xor rax, rax
        ret

.wd:    call findInDict
        test rbx, rbx
        jz .err
        mov PC, [rbx]
        nextCell rax
        call rax
        jmp .r0

.err:   push rsi
        mov al, '-'
        call semit
        pop rsi
        call strlen
        call stype
        mov al, '-'
        call semit
        mov al, '?'
        call semit
        jmp _r1

; ------------------------------------------------------------------------------
_repl:
        mov rsi, ok
        mov rdx, 4
        call stype
        cmp DSP, dstk               ; Check stack underflow
        jge .0
        mov DSP, dstk
.0:     mov rsi, tib                ; Read line into TIB
        mov [toIn], rsi
        mov rdx, 256                ; Buffer size
        call readLine
        mov [rsi+rax], word 0       ; NULL Terminate
.lp:    call nextWord               ; Parse line
        test rdx, rdx               ; RDX=0 means end of line
        jz _repl
        call _parseWord             ; Will return RAX=0 if no error
        test rax, rax
        jz .lp
        jmp _repl

; ------------------------------------------------------------------------------
; The VM opcodes
; ------------------------------------------------------------------------------
include 'opcodes.s'

primEnd:

; ------------------------------------------------------------------------------
segment readable writable

macro DICT_E xt, fl, ln, nm {
      S=$
    dq xt
    db fl
    db ln
    db nm, 0
      E=$
    db 32-(E-S) dup (0)
}

hi:   db 'hello.', 0
ok:   db ' ok', 10, 0

buf1:      db 64 dup (0)

align 8
dStart:    DICT_E xtDot,  0, 1, '.'
           DICT_E xtBye,  0, 3, 'bye'
           DICT_E xtAdd,  0, 1, '+'
           DICT_E xtSub,  0, 1, '-'
           DICT_E _mult,  0, 1, '*'
           DICT_E _slmod, 0, 4, '/mod'
           DICT_E xtEmit, 0, 4, 'emit'
lastA:     DICT_E _qtype, 0, 5, 'qtype'
           rb DICT_SZ

align 8
wcStart:
xtDot:     dq _dot, _lit, 32, _emit, _exit
xtBye:     dq _bye, _exit
xtAdd:     dq _add, _exit
xtSub:     dq _sub, _exit
xtEmit:    dq _emit, _exit
hereA:     rq CODE_SZ

vStart:    rb VARS_SZ
toIn:      rq 1
last:      rq 1
here:      rq 1
base:      rq 1
state:     rq 1
dstk:      rq DSTK_SZ
rstk:      rq RSTK_SZ
lstk:      rq LSTK_SZ
lsp:       rq 1
regs:      rq 100
regBase:   rq 1
wd:        rb 32
tib:       rb TIB_SZ
buf2:      rb 64
