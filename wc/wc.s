format ELF64 executable 3

segment readable executable
entry cold

; ------------------------------------------------------------------------------
CELL_SZ    equ 8
CODE_SZ    equ  64*1024
VARS_SZ    equ 128*1024
DICT_SZ    equ   8*1024
DSTK_SZ    equ  64*CELL_SZ
RSTK_SZ    equ  64*CELL_SZ
LSTK_SZ    equ  30*CELL_SZ
TIB_SZ     equ 256

; INDEX    equ r11
DSP        equ r12
RSP        equ r13
PC         equ r14
TOS        equ r15

STDIN      equ 0
STDOUT     equ 1
SYS_READ   equ 0
SYS_WRITE  equ 1

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
    add RSP, CELL_SZ
    mov [RSP], qword val
}

; ------------------------------------------------------------------------------
macro rPOP val {
    mov val, [RSP]
    sub RSP, CELL_SZ
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
macro NEXT { ret }

macro mEmit val {
        mLit val
        Comma _emit
}

; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
cold:
        mov DSP, dstk             ; Start of data STACK
        mov RSP, rstk             ; Start of return STACK
        lea rax, [lstk]           ; Start of loop STACK
        mov [lsp], rax
        mov rax, dEnd             ; LAST
        mov [last], rax
        mov rax, wcStart
        mov [here], rax           ; HERE: start of code
        lea rax, [regs]           ; Start of registers
		mov [regBase], rax

        mov rsi, hi
        call strlen
        call stype

        jmp _repl

; ------------------------------------------------------------------------------
prim:   call rax
run:    nextCell rax
        cmp rax, wcStart
        jl prim
.wc:    btr rax, 1          ; It's a "word-code" ...
        jnc .1              ; Bit #1 ON means JMP
        rPUSH PC            ; Bit #1 OFF means CALL
.1:     mov PC, rax
        jmp run

; ------------------------------------------------------------------------------
wcTest:
        ; Some WordCode for testing ...
        mLit yy
        mLit 5
        Comma _type

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

        mLit '0'-1
        mLit '9'
        Comma _do
        Comma _index
        Comma _emit
        Comma _loop2

        Comma _stop
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
; Word is in RSI, len is in RDX (DL)
; ------------------------------------------------------------------------------
addToDict:
        test dl, dl
        jz .r
        mov rax, [here]     ; For the XT
        mov r9, [last]
        sub r9, 32
        mov [last], r9
        mov [r9], rax       ; XT
        add r9, CELL_SZ
        mov [r9], byte 0    ; Flags
        inc r9
        cmp dl, 21          ; Check len
        jle .l
        mov dl, 21          ; trunc to 21
.l:     mov [r9], dl        ; Len
.s:     lodsb               ; Name
        mov [r9], byte al
        inc r9
        dec dl
        jnz .s
        mov [r9], byte 0    ; NULL
.r:     ret

; ------------------------------------------------------------------------------
; C3 subroutines
; ------------------------------------------------------------------------------
semit:                      ; The char to EMIT is in al
        mov rsi, buf2
        mov [rsi], al
        mov rdx, 1          ; len=1

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

lower:  cmp al, 'A'
        jl .9
        cmp al, 'Z'
        jg .9
        add al, 32
.9:     ret

; ------------------------------------------------------------------------------
; The strings are in RSI and RDI
; Return: IF strings are equal RAX=1,  ELSE RAX=0
; ------------------------------------------------------------------------------
strcmpi: xor rdx, rdx
.1:     mov al, byte [rsi+rdx]
        call lower
        mov ah, al
        mov al, byte [rdi+rdx]
        call lower
        cmp al, ah
        jne _r0
        inc rdx
        test al, al
        jnz .1
_r1:    mov rax, 1
        ret
_r0:    xor rax, rax
        ret

; ------------------------------------------------------------------------------
; The strings are in RSI and RDI
; Return: IF strings are equal RAX=1,  ELSE RAX=0
; ------------------------------------------------------------------------------
strcmp: xor rdx, rdx
.1:     mov al, byte [rsi+rdx]
        mov ah, al
        mov al, byte [rdi+rdx]
        cmp al, ah
        jne _r0
        inc rdx
        test al, al
        jnz .1
        jmp _r1

; ------------------------------------------------------------------------------
_c3Test:
        mov r9, [here]
        cmp r9, wcStart
        jg .R
        call wcTest
.R:     mov PC, wcStart
        call run
        jmp _r0

; ------------------------------------------------------------------------------
; The word is "-ML-". Handle it.
; ------------------------------------------------------------------------------
doML:
        ; TODO: fill this in
        mov rsi, mlx
        call strlen
        call stype
        jmp _r0

; ------------------------------------------------------------------------------
; The char is in AL
; Return: IF AL in ['0'..'9'], AL=(AL-'0'), ELSE AH=1
; ------------------------------------------------------------------------------
is09:   mov ah, 1
        cmp al, '0'
        jl .x
        cmp al, '9'
        jg .x
        sub al, '0'
        dec ah
.x:     ret

; ------------------------------------------------------------------------------
; The word is in RSI, length is in RDX
; Return: IF number, RAX=1 and number in TOS, ELSE RAX=0
; ------------------------------------------------------------------------------
isNum:
		dPUSH 0
		xor rax, rax
.1:		lodsb
		test al, al
		jz _r1
		call is09
		test ah, ah
		jnz .f
		imul TOS, 10
		add TOS, rax
		jmp .1
.f:		dPOP rax
		xor rax, rax
		ret

; ------------------------------------------------------------------------------

; ------------------------------------------------------------------------------
; The word is in RSI, length is in RDX
; ------------------------------------------------------------------------------
_parseWord:
        ; TODO: fill this in!
        mov rdi, gb             ; "bye"?
        call strcmpi
        test rax, rax
        jnz _bye

		push rsi               ; Number?
		push rdx
		call isNum
		pop rdx
		pop rsi
		test rax, rax
		jz .1
		dPOP rax
		mLit rax
		jmp _r0

.1:     cmp byte [rsi], 'X'     ; Test?
        je _c3Test

.2:     mov rdi, ml             ; "-ML-"?
        call strcmp
        test rax, rax
        jnz doML

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
        mov rsi, tib                ; Read line into TIB
        mov [toIn], rsi
        mov rdx, 256                ; TIB size
        call readLine
        mov [rsi+rax], word 0       ; NULL Terminate
.1:     call nextWord               ; Parse line
        test rdx, rdx
        jz _repl                    ; End of line
        call _parseWord
        test rax, rax
        jz .1
        jmp _repl

; ------------------------------------------------------------------------------
; The VM opcodes
; ------------------------------------------------------------------------------
include 'opcodes.s'

primEnd:

; ------------------------------------------------------------------------------
segment readable writable



hi:   db 'hello.', 0
gb:   db 'bye', 0
ml:   db '-ML-', 0
mlx:  db '-MLX-', 0
ok:   db ' ok', 10, 0
yy:   db '-YY-', 10, 0

c3_ops:
    dq _stop, _lit, _exit, _call, _jmp, _jmpz, _jmpnz, _store, _cstore  ;  0 ->  9
    dq _fetch, _cfetch, _dup, _swap, _over, _drop, _add, _mult, _slmod, _sub   ; 10 -> 19
    dq _inc, _dec, _lt, _eq, _gt, _eq0, _rto, _rfetch, _rfrom, _do             ; 20 -> 29
    dq _loop, _loop2, _index, _com, _and, _or, _xor, _type, _ztype, _reg_i     ; 30 -> 39
    dq _reg_d, _reg_r, _reg_rd, _reg_ri, _reg_s, _reg_new, _reg_free           ; 40 -> 46
    dq _sys_ops, _str_ops, _flt_ops                                            ; 47 -> 49

sys_ops:
    dq _inline, _immediate, _dot, _nop, _itoa, _atoi, _colondef, _endword      ;  0 ->  7
    dq _create, _find, _word, _timer, _ccomma, _comma, _key, _qkey             ;  8 -> 15
    dq _emit, _qtype, _read                                                    ; 16 -> 18

str_ops:
    dq _trunc, _lcase, _ucase, _nop, _strcpy, _strcat, _strcatc, _strlen       ;  0 ->  7
    dq _streq, _streqi, _ltrim, _rtrim                                         ;  8 -> 11

flt_ops:
    dq _fadd, _fsub, _fmul, _nop, _fdiv, _feq, _flt, _fgt                      ;  0 ->  7
    dq _f2i, _i2f, _fdot, _sqrt, _tanh                                         ;  8 -> 12

toIn: rq 1
last: rq 1
here: rq 1
dstk: rq DSTK_SZ
rstk: rq RSTK_SZ
lstk: rq LSTK_SZ
lsp:  rq 1
regs: rq 100
regBase: rq 1
wd:   rb 32
tib:  rb TIB_SZ
buf2: rb 16

wcStart: rq CODE_SZ
vStart:  rb VARS_SZ
dStart:  rb DICT_SZ
dEnd:
