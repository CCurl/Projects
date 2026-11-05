; fwc.asm - Freestanding Tachyon/fwc-inspired Forth VM
; Linux x86-64, pure FASM, zero libc
; Uses mmap for the 16 MB memory block (binary stays small)

format ELF64 executable 3
entry start

; ------------------------------------------------------------
; Constants
; ------------------------------------------------------------
MEM_SZ          = 0x1000000
STK_SZ          = 63
NAME_SZ         = 22
CELL_SZ         = 8
IMMED           = 0x80
INLINE          = 0x40

LIT_MASK        = 0x7FF8000000000000
LIT_BITS        = 0x0007FFFFFFFFFFFF

INTERPRET       = 0
COMPILE         = 1
BYE             = 999

SYS_read        = 0
SYS_write       = 1
SYS_open        = 2
SYS_close       = 3
SYS_mmap        = 9
SYS_exit        = 60
SYS_clock_gettime = 228
SYS_nanosleep   = 35

PROT_READ       = 1
PROT_WRITE      = 2
MAP_PRIVATE     = 0x02
MAP_ANONYMOUS   = 0x20

STDIN           = 0
STDOUT          = 1

; ------------------------------------------------------------
; Data
; ------------------------------------------------------------
segment readable writeable

align 8
dstk            rq STK_SZ+1
rstk            rq STK_SZ+1
lstk            rq STK_SZ+1
tstk            rq STK_SZ+1

dsp             dq 0
rstkp           dq 0
lsp             dq 0
tsp             dq 0

here            dq 0
last            dq 0
base            dq 10
state           dq INTERPRET
outputFp        dq 0

toIn            dq 0
mem_ptr         dq 0              ; points to the 16 MB block allocated by mmap
wd              rb 32
tib             rb 256
fn              rb 64
tmpWords        rb 10*32

; ------------------------------------------------------------
; Code
; ------------------------------------------------------------
segment readable executable

; ---------- Syscall helpers ----------
f_write:
    mov rax, SYS_write
    syscall
    ret

f_read:
    mov rax, SYS_read
    syscall
    ret

f_exit:
    mov rax, SYS_exit
    syscall

; ---------- Stack primitives ----------
dpush:                              ; rdi = value
    mov rax, [dsp]
    cmp rax, STK_SZ
    jge .overflow
    inc rax
    mov [dsp], rax
    mov [dstk + rax*8], rdi
.overflow:
    ret

dpop:
    mov rax, [dsp]
    test rax, rax
    jz .underflow
    mov rdi, [dstk + rax*8]
    dec rax
    mov [dsp], rax
    mov rax, rdi
    ret
.underflow:
    xor rax, rax
    ret

rpush:                              ; rdi = value
    mov rax, [rstkp]
    cmp rax, STK_SZ
    jge .overflow
    inc rax
    mov [rstkp], rax
    mov [rstk + rax*8], rdi
.overflow:
    ret

rpop:
    mov rax, [rstkp]
    test rax, rax
    jz .underflow
    mov rdi, [rstk + rax*8]
    dec rax
    mov [rstkp], rax
    mov rax, rdi
    ret
.underflow:
    xor rax, rax
    ret

; ---------- Emit / Type ----------
emit:
    push rdi
    mov byte [rsp], dil
    mov rdi, STDOUT
    mov rsi, rsp
    mov rdx, 1
    call f_write
    add rsp, 8
    ret

zType:
    mov rsi, rdi
    xor rdx, rdx
.len:
    cmp byte [rsi + rdx], 0
    je .do
    inc rdx
    jmp .len
.do:
    mov rdi, STDOUT
    call f_write
    ret

; ---------- String helpers ----------
strlen:
    xor rax, rax
.loop:
    cmp byte [rdi + rax], 0
    je .done
    inc rax
    jmp .loop
.done:
    ret

strcpy:
.copy:
    lodsb
    stosb
    test al, al
    jnz .copy
    ret

strcasecmp:
.loop:
    mov al, [rdi]
    mov bl, [rsi]
    cmp al, 'A'
    jb .n1
    cmp al, 'Z'
    ja .n1
    add al, 32
.n1:
    cmp bl, 'A'
    jb .n2
    cmp bl, 'Z'
    ja .n2
    add bl, 32
.n2:
    cmp al, bl
    jne .diff
    test al, al
    jz .eq
    inc rdi
    inc rsi
    jmp .loop
.eq:
    xor rax, rax
    ret
.diff:
    mov rax, 1
    ret

; ---------- Dictionary ----------
addToDict:
    test rdi, rdi
    jnz .have_name
    lea rdi, [wd]
.have_name:
    call strlen
    cmp rax, NAME_SZ
    jb .len_ok
    mov rax, NAME_SZ-1
.len_ok:
    mov rcx, rax

    mov rbx, [last]
    sub rbx, 32
    mov [last], rbx

    mov rax, [here]
    mov [rbx], rax
    mov byte [rbx+8], 0
    mov [rbx+9], cl
    lea rsi, [wd]
    lea rdi, [rbx+10]
    call strcpy
    mov rax, rbx
    ret

; ---------- Selected primitives ----------
prim_DUP:
    mov rax, [dsp]
    mov rdi, [dstk + rax*8]
    call dpush
    ret

prim_DROP:
    call dpop
    ret

prim_SWAP:
    mov rax, [dsp]
    mov rdi, [dstk + rax*8]
    mov rsi, [dstk + (rax-1)*8]
    mov [dstk + rax*8], rsi
    mov [dstk + (rax-1)*8], rdi
    ret

prim_OVER:
    mov rax, [dsp]
    mov rdi, [dstk + (rax-1)*8]
    call dpush
    ret

prim_ADD:
    call dpop
    mov rbx, rax
    mov rax, [dsp]
    add qword [dstk + rax*8], rbx
    ret

prim_SUB:
    call dpop
    mov rbx, rax
    mov rax, [dsp]
    sub qword [dstk + rax*8], rbx
    ret

prim_MULT:
    call dpop
    mov rcx, rax
    mov rax, [dsp]
    mov rbx, qword [dstk + rax*8]
    imul rbx, rcx
    mov qword [dstk + rax*8], rbx
    ret

prim_EMIT:
    call dpop
    mov rdi, rax
    call emit
    ret

prim_ZTYPE:
    call dpop
    mov rdi, rax
    call zType
    ret

; ---------- Inner interpreter ----------
inner:
    mov r12, rdi                    ; pc
.next:
    mov rbx, [mem_ptr]
    mov rax, qword [rbx + r12*8]
    inc r12

    mov rcx, LIT_MASK
    mov rdx, rax
    and rdx, rcx
    cmp rdx, rcx
    jne .not_lit
    mov rcx, LIT_BITS
    and rax, rcx
    mov rdi, rax
    call dpush
    jmp .next
.not_lit:

    test rax, rax
    jz .do_exit

    mov rdi, r12
    call rpush
    mov r12, rax
    jmp .next

.do_exit:
    call rpop
    test rax, rax
    jz .done
    mov r12, rax
    jmp .next
.done:
    ret

; ---------- Stubs ----------
; ---------- Simple word parser ----------
nextWord:
    ; returns length in rax, word in wd (null-terminated)
    mov rsi, [toIn]
    xor rcx, rcx                    ; length

.skip:
    mov al, [rsi]
    test al, al
    jz .empty
    cmp al, 32
    ja .copy
    inc rsi
    jmp .skip

.copy:
    cmp al, 32
    jbe .done
    cmp rcx, 31
    jae .done
    mov [wd + rcx], al
    inc rcx
    inc rsi
    mov al, [rsi]
    jmp .copy

.done:
    mov byte [wd + rcx], 0
    mov [toIn], rsi
    mov rax, rcx
    ret

.empty:
    mov [toIn], rsi
    xor rax, rax
    ret

; ---------- Minimal outer interpreter ----------
outer:
    mov [toIn], rdi
.word_loop:
    call nextWord
    test rax, rax
    jz .done

    ; very simple test: if the word is "bye" → set state = BYE
    lea rdi, [wd]
    lea rsi, [bye_str]
    call strcasecmp
    test rax, rax
    jnz .not_bye
    mov qword [state], BYE
    jmp .done
.not_bye:

    ; otherwise just echo the word + space (for testing)
    lea rdi, [wd]
    call zType
    mov dil, '-'
    call emit
    jmp .word_loop

.done:
    mov dil, 10                     ; newline
    call emit
    ret

bye_str db "bye", 0

; ---------- Init (allocates the 16 MB block) ----------
fwcInit:
    ; mmap(NULL, MEM_SZ, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)
    mov rax, SYS_mmap
    xor rdi, rdi
    mov rsi, MEM_SZ
    mov rdx, PROT_READ or PROT_WRITE
    mov r10, MAP_PRIVATE or MAP_ANONYMOUS
    mov r8, -1
    xor r9, r9
    syscall
    ; rax = address of the block
    mov [mem_ptr], rax

    mov qword [here], 64
    ; last points near the end of the allocated block
    add rax, MEM_SZ
    mov [last], rax
    ret

boot:
    ret

; ---------- Simple REPL ----------
repl:
    ; prompt
    lea rdi, [prompt]
    call zType

    ; read a line
    mov rdi, STDIN
    lea rsi, [tib]
    mov rdx, 255
    call f_read
    test rax, rax
    jle .eof                        ; ≤0 → EOF or error

    ; null-terminate
    mov byte [tib + rax], 0

    ; feed to outer
    lea rdi, [tib]
    call outer
    ret

.eof:
    mov qword [state], BYE
    ret

prompt db " ok", 10, 0


; ---------- Entry ----------
start:
    call fwcInit

.main_loop:
    cmp qword [state], BYE
    je .bye
    call repl
    jmp .main_loop

.bye:
    lea rdi, [bye_msg]
    call zType
    xor rdi, rdi
    call f_exit

bye_msg db "bye", 10, 0
