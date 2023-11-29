format ELF64 executable 3

segment readable executable
entry cold

; ------------------------------------------------------------------------------
CODE_SZ equ 65536
RSP     equ r13
PC      equ r14
TOS     equ r15
HERE    equ rbp

; ------------------------------------------------------------------------------
macro dPUSH val {
    push TOS
    mov TOS, QWORD val
}

; ------------------------------------------------------------------------------
macro dPOP val {
    mov val, TOS
    pop TOS
}

; ------------------------------------------------------------------------------
macro rPUSH val {
    add RSP, 8
    mov [RSP], qword val
}

; ------------------------------------------------------------------------------
macro rPOP val {
    mov val, [RSP]
    sub RSP, 8
}

; ------------------------------------------------------------------------------
macro CComma val {
    mov [rbp], byte val
    inc rbp
}

; ------------------------------------------------------------------------------
macro Comma val {
    mov [rbp], qword val
    add rbp, 8
}

; ------------------------------------------------------------------------------
macro nextByte val {
    movzx val, byte [PC]
    inc PC
}

; ------------------------------------------------------------------------------
macro nextCell val {
    mov val, qword [PC]
    add PC, 8
}

; ------------------------------------------------------------------------------
macro mLit1 val {
    CComma 1
    CComma val
}

; ------------------------------------------------------------------------------
macro mLit val {
    CComma 2
    Comma val
}

; ------------------------------------------------------------------------------
macro mSys val {
    CComma 47
    CComma val
}

; ------------------------------------------------------------------------------
macro mAdd val { CComma 16 }
macro mSub val { CComma 19 }
macro mEmit val { mSys 16 }

; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
cold:
        lea RSP, [rstk]           ; Start of return STACK
        lea rax, [lstk]           ; Start of loop STACK
        mov [lsp], rax
        lea rax, [buf2]           ; LAST: end of the code
        mov [last], rax
        lea PC, [code]            ; PC: start at beginning of the code
        mov HERE, PC              ; HERE: start of code

        ; Some overall testing ...
        lea rsi, [hi]
        mov [toIn], rsi
        call nwTest
        call nwTest
        call nwTest
        ; jmp bye

        ; The test Byte Code ...
        lea rax, [c3]             ; "c3: "
        mLit rax                  ; lit
        mLit1 5                   ; lit1 5 (len)
        CComma 37                 ; type
        mLit1 'R'                 ; lit1 'R'
        mLit1 1                   ; lit1 1
        mAdd                      ; add
        mEmit                     ; emit 'S'
        mLit1 10                  ; lit1 '\n'
        mEmit                     ; emit
        mov rax, 500000000
        mLit rax
        mLit1 0
        CComma 29                 ; DO
        CComma 30                 ; LOOP
        mLit1 'F'                 ; lit1 'F'
        mLit1 1                   ; lit1 1
        mSub                      ; sub
        mEmit                     ; emit 'E'
        mLit1 10                  ; lit1 '\n'
        mEmit
        mLit1 '9'+1
        mLit1 '0'
        CComma 29                 ; DO
        CComma 32                 ; (I)
        CComma 10                 ; FETCH
        mEmit
        CComma 30                 ; LOOP
        mEmit                     ; emit

        mLit1 10                  ; lit1 '\n'
        mEmit
        CComma 0                  ; bye

; ------------------------------------------------------------------------------
next:   nextByte rcx
        mov rax, [c3_ops+rcx*8]
        jmp rax

; ------------------------------------------------------------------------------
nwTest: call nextWord
        call stype
        mov al, ','
        call semit
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
        push HERE
        mov rax, HERE
        mov HERE, [last]
        sub HERE, 32
        mov [last], HERE
        Comma rax           ; XT
        CComma 0            ; Flags
        cmp dl, 21          ; Check len
        jle .l
        mov dl, 21          ; trunc to 21
.l:     CComma dl           ; Len
.s:     lodsb               ; Name
        CComma al
        dec dl
        jnz .s
        CComma 0            ; NULL
        pop HERE
.r:     ret

; ------------------------------------------------------------------------------
; C3 subroutines
; ------------------------------------------------------------------------------
semit:                      ; The char to EMIT is in al
        lea rsi, [buf2]
        mov [rsi], al
        mov rdx, 1          ; len=1
        mov rdi, 1          ; stdout
        mov rax, 1          ; sys_write
        syscall
        ret

; ------------------------------------------------------------------------------
stype:                      ; The string is in RSI, the len is in RDX
        mov rdi, 1          ; stdout
        mov rax, 1          ; sys_write
        syscall
        ret

; ------------------------------------------------------------------------------
; The VM opcodes
; ------------------------------------------------------------------------------
include 'opcodes.s'

; ------------------------------------------------------------------------------
segment readable writable

hi:   db 'test 123', 10, 0
c3:   db 'c3: ', 10, 0

c3_ops:
    dq _stop, _lit1, _lit, _exit, _call, _jmp, _jmpz, _jmpnz, _store, _cstore  ;  0 ->  9
    dq _fetch, _cfetch, _dup, _swap, _over, _drop, _add, _mult, _slmod, _sub   ; 10 -> 19
    dq _inc, _dec, _lt, _eq, _gt, _eq0, _rto, _rfetch, _rfrom, _do             ; 20 -> 29
    dq _loop, _loop2, _index, _com, _and, _or, _xor, _type, _ztype, _reg_i     ; 30 -> 39
    dq _reg_d, _reg_r, _reg_rd, _reg_ri, _reg_s, _reg_new, _reg_free           ; 40 -> 46
    dq _sys_ops, _str_ops, _flt_ops                                            ; 47 -> 49

sys_ops:
    dq _inline, _immediate, _dot, _nop, _itoa, _atoi, _colondef, _endword      ;  0 ->  7
    dq _create, _find, _word, _timer, _ccomma, _comma, _key, _qkey             ;  8 -> 15
    dq _emit, _qtype                                                           ; 16 -> 17

str_ops:
    dq _trunc, _lcase, _ucase, _nop, _strcpy, _strcat, _strcatc, _strlen       ;  0 ->  7
    dq _streq, _streqi, _ltrim, _rtrim                                         ;  8 -> 11

flt_ops:
    dq _fadd, _fsub, _fmul, _nop, _fdiv, _feq, _flt, _fgt                      ;  0 ->  7
    dq _f2i, _i2f, _fdot, _sqrt, _tanh                                         ;  8 -> 12

toIn: rq 1
last: rq 1
rstk: rq 64
lstk: rq 64
lsp:  rq 1
wd:   rb 32
tib:  rb 256
code: rb CODE_SZ
buf2: rb 16
