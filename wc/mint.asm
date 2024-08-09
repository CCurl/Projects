; MINT - a Minimal Interpreter

format PE console
include 'win32ax.inc'

; ******************************************************************************
; ******************************************************************************
section '.code' code readable executable

; ------------------------------------------------------------------------------
; MACROS

CELL_SIZE equ 4

TOS  equ edi         ; Top-Of-Stack
PCIP equ esi         ; Program-Counter/Instruction-Pointer
STKP equ ebp         ; Stack-Pointer

REG1 equ eax         ; Free register #1
REG2 equ ebx         ; Free register #2
REG3 equ ecx         ; Free register #3
REG4 equ edx         ; Free register #4

; ******************************************************************************
macro m_getTOS val
{
       mov val, TOS
}

macro m_setTOS val
{
       mov TOS, val
}

macro m_push val
{
       inc [dDepth]
       add STKP, CELL_SIZE
       mov [STKP], TOS
       m_setTOS val
}

; ******************************************************************************
macro m_get2ND val
{
       mov val, [STKP]
}

macro m_set2ND val
{
       mov [STKP], val
}

; ******************************************************************************
macro m_pop val
{
       m_getTOS val
       dec [dDepth]
       mov TOS, [STKP]
       sub STKP, CELL_SIZE
}

; ******************************************************************************
rpush:  add     [rStackPtr], CELL_SIZE
        mov     edx, [rStackPtr]
        mov     [edx], esi
        ret
 
; ******************************

doExit: mov     eax, [rStackPtr]
        cmp     eax, rStack
        jle     .U
        mov     esi, [eax]
        sub     [rStackPtr], CELL_SIZE
        ret
.U:     m_push '-'
        call    doEmit
        m_push 'U'
        call    doEmit
        m_push '-'
        call    doEmit
        mov     esp, [InitialESP]
        jmp     repl

; ******************************************************************************
wCall:  call    eax
wcRun:  lodsd
        cmp     eax, primEnd
        jl      wCall
        cmp     eax, $70000000
        jge     .NUM
        add     [rStackPtr], CELL_SIZE
        mov     edx, [rStackPtr]
        mov     [edx], esi
        mov     esi, eax
        jmp     wcRun
.NUM:   and     eax, $0FFFFFFF
        m_push  eax
        jmp     wcRun

; ******************************************************************************
doNop:  ret

; ******************************************************************************
; input:        ; eax: the number to print - destroyed
; output:       ; ecx: the start of the string
;               ; ebx: the length of the string
;
iToA:   mov     ecx, outBuf+16  ; output string start
        mov     ebx, 0          ; output length
        mov     BYTE [ecx], 0
        push    0               ; isNegative flag
        bt      eax, 31
        jnc     i2a1
        inc     BYTE [esp]
        neg     eax
i2a1:   push    ebx
        mov     ebx, 10
        mov     edx, 0
        div     ebx
        add     dl, '0'
        dec     ecx 
        mov     BYTE [ecx], dl
        pop     ebx
        inc     ebx
        cmp     eax, 0
        jne     i2a1
        pop     eax             ; get the negative flag
        cmp     eax, 0          ; 0 means not negative
        je      i2aX
        dec     ecx
        mov     BYTE [ecx], '-'
        inc     ebx
i2aX:   ret

; ******************************************************************************
doCol:  lodsb
        mov     bx, 'AZ'
        call    betw
        cmp     bl, 0
        je      colX
        ; call    fnAddr
        mov     [edx], esi
col1:   cmp     esi, [HERE1]
        jge     colX
        lodsb
        cmp     al, ';'
        je      .E
        cmp     al, 0
        jne     col1
.E:     mov     [HERE], esi
        mov     al, 'h'
        mov     ebx, esi
        ; call    setReg
colX:   ret

; ******************************************************************************
doFetch: m_getTOS   edx
        m_setTOS    [edx]
        ret

; ******************************************************************************
doStore: m_pop  edx
        m_pop   eax
        mov     [edx], eax
        ret

; ******************************************************************************
cFetch: m_getTOS   edx
        mov         TOS, [edx]
        and         TOS, $ff
        jmp         wcRun

; ******************************************************************************
cStore: m_pop  edx
        m_pop   eax
        mov     BYTE [edx], al
        ret

; ******************************************************************************
; Number input
num:    sub     al, '0'
        and     eax, $FF
        mov     edx, eax
.l:     mov     al, [esi]
        mov     bx, '09'
        call    betw
        cmp     bl, 0
        je      .x
        sub     al, '0'
        imul    edx, edx, 10
        add     edx, eax
        inc     esi
        jmp     .l
.x:     m_push  edx
        ret

; ******************************************************************************
; Quote
doQt:   lodsb
        cmp     al, '_'
        je      .x
        call    p1
        jmp     doQt
.x:     ret

; ******************************************************************************
betw:   cmp     al, bl
        jl      betF
        cmp     al, bh
        jg      betF
        mov     bl, 1
        ret
betF:   mov     bl, 0
        ret

; ******************************************************************************
doFor:  m_pop   ebx
        ;cmp     ebx, 0
        ;je      fSkip
        ;mov     [forIndex], ebx
        ;mov     [forStart], esi
        ;mov     al, 'i'
        ;call    setReg
        ret

fSkip:  lodsb
        cmp     al, ')'
        je      .x
        cmp     al, 0
        jne     fSkip
.x:     ret

; ******************************************************************************
doNext: ;mov     eax, [forIndex]
        ;dec     eax
        ;cmp     eax, 0
        ;je      .out
        ;mov     [forIndex], eax
        ;mov     esi, [forStart]
        ret
.out:   pop     eax
        ret

; ******************************************************************************
f_UnknownOpcode:
        jmp repl

; ******************************************************************************
bye:    invoke  ExitProcess, 0

; ******************************************************************************
doOK:   invoke  WriteConsole, [hStdOut], okStr, 5, NULL, NULL
        ret

; ******************************************************************************
doMult: m_pop   eax
        imul    TOS, eax
        ret

; ******************************************************************************
doSub:  m_pop   eax
        sub     TOS, eax
        ret

; ******************************************************************************
doAdd:  m_pop   eax
        add     TOS, eax
        ret

; ******************************************************************************
doMod:  m_pop   ebx
        cmp     ebx, 0
        je      wcRun
        m_pop   eax
        mov     edx, 0
        idiv    ebx
        m_push  edx
        ret

; ******************************************************************************
doDiv:  m_pop   ebx
        cmp     ebx, 0
        je      wcRun
        m_pop   eax
        mov     edx, 0
        idiv    ebx
        m_push  eax
        ret

; ******************************************************************************
doAnd:  m_pop   eax
        and     TOS, eax
        ret

; ******************************************************************************
doOr:   m_pop   eax
        or      TOS, eax
        ret

; ******************************************************************************
doXOR:  m_pop   eax
        xor     TOS, eax
        ret

; ******************************************************************************
doNeg:  neg     TOS
        ret

; ******************************************************************************
doInv:  not     TOS
        ret

; ******************************************************************************
cmpT:   mov     TOS, 1
        ret

; ******************************************************************************
cmpF:   mov     TOS, 0
        ret

; ******************************************************************************
doEQ:   m_pop   eax
        cmp     TOS, eax
        je      cmpT
        jmp     cmpF

; ******************************************************************************
doLT:   m_pop   eax
        cmp     TOS, eax
        jl      cmpT
        jmp     cmpF

; ******************************************************************************
doGT:   m_pop   eax
        cmp     TOS, eax
        jg      cmpT
        jmp     cmpF

; ******************************************************************************
doCrLf: mov     al, 13
        call    p1
        mov     al, 10
        jmp    p1

; ******************************************************************************
doDot:  push    eax
        push    ebx
        push    ecx
        push    edx
        m_pop   eax
        call    iToA
        invoke  WriteConsole, [hStdOut], ecx, ebx, NULL, NULL
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        ret

; ******************************************************************************
doTimer: invoke GetTickCount
        m_push  eax
        ret

; ******************************************************************************
doDup:  m_push  TOS
        ret

; ******************************************************************************
doSwap: m_get2ND    eax
        m_set2ND    TOS
        m_setTOS    eax
        ret

; ******************************************************************************
doOver: m_get2ND    eax
        m_push      eax
        ret

; ******************************************************************************
doDrop: mov     TOS, [STKP]
        sub     STKP, CELL_SIZE
        cmp     STKP, dStack
        jg      .X
        mov     STKP, dStack
.X:     ret

; ******************************************************************************
doEmit: m_pop   eax
        mov     [buf1], al
        invoke  WriteConsole, [hStdOut], buf1, 1, NULL, NULL
        ret

; ******************************************************************************
p1:     push    eax
        m_push  eax
        call    doEmit
        pop     eax
        ret

; ******************************************************************************
doJmp:  lodsd
        mov     esi, eax
        ret

; ******************************************************************************
doLit:  lodsd
        m_push eax
        ret

; ******************************************************************************
; ******************************************************************************
primEnd:
; ******************************************************************************
; ******************************************************************************

; ******************************************************************************
s_SYS_INIT:
        ret        

; ******************************************************************************
start:
        mov     [InitialESP], esp

        invoke GetStdHandle, STD_INPUT_HANDLE
        mov    [hStdIn], eax
        
        invoke GetStdHandle, STD_OUTPUT_HANDLE
        mov    [hStdOut], eax

        mov     ebx, THE_CODE
        mov     [HERE], ebx
        mov     ebx, THE_VARS
        mov     [VHERE], ebx

warm:   mov     esp, [InitialESP]
        mov     eax, rStack
        mov     [rStackPtr], eax
        mov     STKP, dStack
        mov     TOS, 0
        mov     esi, THE_ROM
        call    wcRun
        invoke  ExitProcess, 0
        ret

; ******************************************************************************

repl:   call    doOK
        cmp     STKP, dStack
        jge     .rd
        mov     STKP, dStack
.rd:    invoke  ReadConsole, [hStdIn], [HERE], 128, bytesRead, 0
        sub     [bytesRead], 2
        mov     ebx, [HERE]
        add     ebx, [bytesRead]
        mov     [HERE1], ebx
        cld
        mov     esi, [HERE]
        ret

; ******************************************************************************
section '.mem' data readable writable

hStdIn      dd  ?
hStdOut     dd  ?
okStr       db  " ok", 13, 10, 0
crlf        db  13, 10
outBuf      db  256 dup ?
bytesRead   dd  ?
unkOP       db  "-unk-"
dDepth      dd  0
rDepth      dd  0
rStackPtr   dd  ?
HERE        dd  THE_CODE
VHERE       dd  ?
LAST        dd  LastInit
HERE1       dd  ?
InitialESP  dd    0

buf1        db   16 dup 0       ; Buffer
dStack      dd   64 dup 0
buf2        dd    4 dup 0       ; Buffer
rStack      dd   64 dup 0
buf3        dd    4 dup 0       ; Buffer

THE_ROM:
xMAIN       dd xHA, xDot, xSPC, xHERE, xDot, xHERE, doLit, 1, doAdd, xDot, xOK, doExit
xOK         dd doOK, doExit
xSPC        dd $70000020, doEmit, doExit
xCR         dd $7000000D, doEmit, $7000000A, doEmit, doExit
xHA         dd doLit, HERE, doExit
xHERE       dd xHA, doFetch, doExit
xDot        dd doDot, xSPC, doExit
            dd doExit

THE_CODE    rd 64*1024
CODE_END:

THE_DICT    rb 32
; TODO add dictionary entries
LastInit:
; TODO add last dictionary entry
rb 64*1024
DICT_END:

THE_VARS    rb 256*1024
VARS_END:

.end start
