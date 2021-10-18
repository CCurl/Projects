format PE console

include 'win32ax.inc' ; you can simply switch between win32ax, win32wx, win64ax and win64wx here

.data

hStdIn      dd  0
hStdOut     dd  0
hello       db  "hello.", 0
okStr       db  " s4:()>", 0
crlf        db  13, 10
tib         db  128 dup ?
outBuf      db  256 dup ?
bytesRead   dd  0
isNum       db "isNum"
regs        dd  676 dup 0

.code
dstPush:    ; push something onto the stack
            ret

; register
reg:    sub     al, 'a'
        and     eax, $ff
        mov     edx, eax
reg1:   lodsb
        mov     bx, 'az'
        call    betw
        jz      regx
        sub     al, 'a'
        imul    edx, edx, 26
        add     edx, eax
regx:   mov     ebx, [regs+4*edx]
        call    dstPush ; EDX
        jmp     runS

; Number input
num:    sub     al, '0'
        and     eax, $ff
        mov     edx, eax
        invoke WriteConsole, [hStdOut], isNum, 5, NULL, NULL
n1:     lodsb
        mov     ebx, '09'
        call    betw
        jz      nx
        sub     al, '0'
        imul    edx, edx, 10
        add     edx, eax
        jmp     n1
nx:     call    dstPush ; EDX
        jmp runS

; Quote
qt:     lodsb
        cmp     al, '"'
        je      qx
        call    emit
        jmp     qt
qx:     inc     esi
        jmp     runS

betw:   cmp     al, bl
        jl      betF
        cmp     al, bh
        jg      betF
        mov     bl, 1
        ret
betF:   mov     bl, 0
        ret

f_UnknownOpcode:
        ret

run:    mov     esi, edx
runS:   lodsb
        cmp     al, 32
        je      runS
        jl      rx
        cmp     al, 'Q'
        je      bye
        mov     bx, '09'
        call    betw
        cmp     bl, 1
        je      num
        mov     bx, 'az'
        call    betw
        cmp     bl, 1
        je      reg
        cmp     al, '"'
        je      qt
        jmp     runS
rx:     ret

bye:    invoke  ExitProcess, 0

ok:
        invoke WriteConsole, [hStdOut], okStr, 6, NULL, NULL
        mov al, 13
        call emit
        mov al, 10
        call emit
        ret

input:
        ret

emit:
        mov     [outBuf], al
        invoke  WriteConsole, [hStdOut], outBuf, 1, NULL, NULL
        ret

start:
        invoke GetStdHandle, STD_INPUT_HANDLE
        mov    [hStdIn], eax
        
        invoke GetStdHandle, STD_OUTPUT_HANDLE
        mov    [hStdOut], eax
    
        invoke WriteConsole, [hStdOut], hello, 6, NULL, NULL

s0:     call   ok
        call   input
        invoke ReadConsole, [hStdIn], tib, 128, bytesRead, 0
        mov    edx, tib
        call   run
        jmp    s0
    
        invoke ExitProcess, 0


.data

jmpTable dd f_UnknownOpcode ; 0
dd f_UnknownOpcode            ; Hex: 01 (1)
dd f_UnknownOpcode            ; Hex: 02 (2)
dd f_UnknownOpcode            ; Hex: 03 (3)
dd f_UnknownOpcode            ; Hex: 04 (4)
dd f_UnknownOpcode            ; Hex: 05 (5)
dd f_UnknownOpcode            ; Hex: 06 (6)
dd f_UnknownOpcode            ; Hex: 07 (7)
dd f_UnknownOpcode            ; Hex: 08 (8)
dd f_UnknownOpcode            ; Hex: 09 (9)
dd f_UnknownOpcode            ; Hex: 0A (10)
dd f_UnknownOpcode            ; Hex: 0B (11)
dd f_UnknownOpcode            ; Hex: 0C (12)
dd f_UnknownOpcode            ; Hex: 0D (13)
dd f_UnknownOpcode            ; Hex: 0E (14)
dd f_UnknownOpcode            ; Hex: 0F (15)
dd f_UnknownOpcode            ; Hex: 10 (16)
dd f_UnknownOpcode            ; Hex: 11 (17)
dd f_UnknownOpcode            ; Hex: 12 (18)
dd f_UnknownOpcode            ; Hex: 13 (19)
dd f_UnknownOpcode            ; Hex: 14 (20)
dd f_UnknownOpcode            ; Hex: 15 (21)
dd f_UnknownOpcode            ; Hex: 16 (22)
dd f_UnknownOpcode            ; Hex: 17 (23)
dd f_UnknownOpcode            ; Hex: 18 (24)
dd f_UnknownOpcode            ; Hex: 19 (25)
dd f_UnknownOpcode            ; Hex: 1A (26)
dd f_UnknownOpcode            ; Hex: 1B (27)
dd f_UnknownOpcode            ; Hex: 1C (28)
dd f_UnknownOpcode            ; Hex: 1D (29)
dd f_UnknownOpcode            ; Hex: 1E (30)
dd f_UnknownOpcode            ; Hex: 1F (32)
dd f_UnknownOpcode            ; Hex: 20 (33)

.end start
