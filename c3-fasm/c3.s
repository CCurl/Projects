format ELF64 executable 3

segment readable executable
entry cold

; ------------------------------------------------------------------------------
macro dPUSH val {
    add r14, 8
    mov [r14], QWORD val
}

; ------------------------------------------------------------------------------
macro dPOP val {
    mov val, QWORD [r14]
    sub r14, 8
}

; ------------------------------------------------------------------------------
macro rPUSH val {
    add r11, 8
    mov [r11], qword val
}

; ------------------------------------------------------------------------------
macro rPOP val {
    mov val, [r11]
    sub r11, 8
}

; ------------------------------------------------------------------------------
macro CComma val {
    mov [r13], byte val
    inc r13
}

; ------------------------------------------------------------------------------
macro Comma val {
    mov [r13], qword val
    add r13, 8
}

; ------------------------------------------------------------------------------
macro nextByte val {
    movzx val, byte [r15]
    inc r15
}

; ------------------------------------------------------------------------------
macro nextCell val {
    mov val, qword [r15]
    add r15, 8
}

; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
; ------------------------------------------------------------------------------
cold:
    lea r11, [rstk]                              ; r11: Return STACK
    lea r12, [code]                              ; r12: LAST: end of the code
    add r12, 65536
    lea r13, [code]                              ; r13: HERE: start of the code
    lea r14, [stk]                               ; r14: STACK
    lea r15, [code]                              ; r15: PC: start at beginning of the code

    ; testing
    ;lea rax, [hi]
    ;dPUSH rax
    ;mov rax, 5
    ;dPUSH rax
    ;call stype
    ; jmp bye

    CComma 2                  ; lit
    lea rax, [c3]             ; "c3: "
    Comma rax
    CComma 1                  ; lit1
    CComma 5                  ; 5
    CComma 8                  ; type

    CComma 0                  ; bye

; ------------------------------------------------------------------------------
next: nextByte rcx
    mov rax, [tbl+rcx*8]
    jmp rax

; ------------------------------------------------------------------------------
lit1: nextByte rax
    dPUSH rax
    jmp next

lit: nextCell rax
    dPUSH rax
    jmp next

exit: rPOP r15
    jmp next

fcall: nextCell rax
    rPUSH r15
    mov r15, rax
    jmp next

fjmp: nextCell rax
    mov r15, rax
    jmp next

fjmpz: dPOP rax
    test rax, rax
    jz fjmp
    add r15, 8
    jmp next

fjmpnz: POP rax
    test rax, rax
    jnz fjmp
    add r15, 8
    jmp next

emit: dPOP rax
    lea rsi, [buf]
    mov [rsi], al
    mov rdx, 1      ; len=1
    mov rdi, 1      ; stdout
    mov rax, 1      ; sys_write
    syscall
    jmp next

stype: dPOP rdx     ; len
    dPOP rsi        ; string
    mov rdi, 1      ; stdout
    mov rax, 1      ; sys_write
    ; push r11 r12 r13 r14 r15
    syscall
    ; pop r15 r14 r13 r12 r11
    ret

type: call stype
    jmp next

bye: xor rdi, rdi                                ; exit code 0
    mov rax, 60                                  ; sys_exit
    syscall

; ------------------------------------------------------------------------------
segment readable writable

hi:   db 'hi. ', 10, 0
c3:   db 'c3: ', 10, 0
buf:  rb 256
tbl:  dq bye, lit1, lit, exit, fcall, fjmp, fjmpz, fjmpnz
      dq type, emit
stk:  dq 256
rstk: dq 256
code: rb 1000
