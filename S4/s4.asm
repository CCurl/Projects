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
; register
reg:    sub     al, 'a'
        and     eax, $ff
        mov     edx, eax
regx:   mov     ebx, [regs,edx]
        ; dstPush ebx
        jmp     runS

; Number input
num:    sub     al, '0'
        and     eax, $ff
        mov     edx, eax
        invoke WriteConsole, [hStdOut], isNum, 5, NULL, NULL
n1:     lodsb
        cmp     al, '0'
        jl      nx
        cmp     al, '9'
        jg      nx
        imul    edx, edx, 10
        add     edx, eax
        jmp     n1
nx:     ; dstPush edx
        jmp runS

; Quote
qt:     lodsb
        cmp     al, '"'
        je      qx
        call    emit
        jmp     qt
qx:     inc     esi
        jmp     runS

betw:   cmp     al, dl
        jl      betF
        cmp     al, dh
        jg      betF
        mov     dl, 1
        ret
betF:   mov     dl, 0
        ret

run:    mov     esi, edx
runS:   lodsb
        cmp     al, 32
        je      runS
        jl      rx
        cmp     al, 'Q'
        je      bye
        mov     dx, '09'
        call    betw
        cmp     dl, 1
        je      num
        mov     dx, 'az'
        call    betw
        cmp     dl, 1
        ; je      reg
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

.end start
