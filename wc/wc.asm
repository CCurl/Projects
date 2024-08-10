; WC - a Tachyon Forth inspired 32-bit system

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
iToA:   mov     ecx, i2aBuf+63  ; output string start
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
doFetch: m_getTOS   edx
        m_setTOS    [edx]
        ret

; ******************************************************************************
doStore: m_pop  edx
        m_pop   eax
        mov     [edx], eax
        ret

; ******************************************************************************
doCFetch: m_getTOS   edx
        mov         TOS, [edx]
        and         TOS, $ff
        jmp         wcRun

; ******************************************************************************
doCStore: m_pop  edx
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
doFor:  add     [lStackPtr], CELL_SIZE*3
        mov     edx, [lStackPtr]
        mov     [edx], DWORD 0
        m_pop   eax
        mov     [edx-CELL_SIZE], eax
        mov     [edx-(CELL_SIZE*2)], esi
        ret

; ******************************************************************************
doI:    mov     edx, [lStackPtr]
        mov     eax, [edx]
        m_push  eax
        ret

; ******************************************************************************
doNext: mov     edx, [lStackPtr]
        mov     eax, [edx]
        inc     eax
        cmp     eax, [edx-CELL_SIZE]
        jge     doUnloop
        mov     [edx], eax
        mov     esi, [edx-(CELL_SIZE*2)]
        ret

; ******************************************************************************
doUnloop:
        mov     edx, [lStackPtr]
        sub     edx, CELL_SIZE*3
        cmp     edx, lStack
        jge     .XX
        mov     edx, lStack
.XX:    mov     [lStackPtr], edx
        ret

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
doInc:  inc     TOS
        ret

; ******************************************************************************
doDec:  dec     TOS
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
doTrue: mov     TOS, 1
        ret

; ******************************************************************************
doFalse: mov     TOS, 0
        ret

; ******************************************************************************
doEQ:   m_pop   eax
        cmp     TOS, eax
        je      doTrue
        jmp     doFalse

; ******************************************************************************
doLT:   m_pop   eax
        cmp     TOS, eax
        jl      doTrue
        jmp     doFalse

; ******************************************************************************
doGT:   m_pop   eax
        cmp     TOS, eax
        jg      doTrue
        jmp     doFalse

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
doLen:  m_pop   edx             ; ( addr--len )
        xor     ecx, ecx
.L:     cmp     [edx], BYTE 0
        je      .X
        inc     ecx
        inc     edx
        jmp     .L
.X:     m_push  ecx
        ret

; ******************************************************************************
doType: m_pop   eax     ; Len ( addr len-- )
        m_pop   ebx     ; Addr
        invoke  WriteConsole, [hStdOut], ebx, eax, NULL, NULL
        ret

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
i2aBuf      db  64 dup ?
bytesRead   dd  ?
unkOP       db  "-unk-"
rStackPtr   dd  rStack
lStackPtr   dd  lStack
HERE        dd  THE_CODE
VHERE       dd  THE_VARS
LAST        dd  LastInit
HERE1       dd  ?
InitialESP  dd  0

buf1        db   16 dup 0       ; Buffer
dStack      dd   64 dup 0       ; Data stack
buf2        dd    4 dup 0       ; Buffer
rStack      dd   64 dup 0       ; Return stack
buf3        dd    4 dup 0       ; Buffer
lStack      dd   64 dup 0       ; Loop stack
buf4        dd    4 dup 0       ; Buffer

; ----------------------------------------------------------------
THE_ROM:
xCold       dd xHA, xDot, xHere, xDot, xLast, xDot, xCell, xDot
                dd xLast, doLit, 12, doFor, xCR, xDeShow, doNext, doDrop
                dd xCR, doLit, 'F', doEmit, doLit, 10, doFor, doI, doInc, xDot, doNext
                dd xCR, doLit, 'S', doEmit, doLit, 500000000, doFor, doNext
                dd doLit, 'E', doEmit
                dd doExit
xDeShow     dd doDup, xDot, doDup, xDeXT, xDot          ; XT
                dd doDup, xDeSize, xDot                 ; Size
                dd doDup, xDeFlags, xDot                ; Flags
                dd doDup, xDeName                       ; First char of name
                dd doDup, doLen, doType                 ; Name length
                dd doDup, xDeSize, doAdd, doExit
xOK         dd doOK, doExit
xDeXT       dd doFetch, doExit                                             ; dict entry size  ( de--xt )
xDeSize     dd xCell, doAdd, doCFetch, doExit                              ; dict entry size  ( de--size )
xDeFlags    dd xCell, doAdd, $70000001, doAdd, doCFetch, doExit            ; dict entry flags ( de--flags )
xDeName     dd xCell, doAdd, $70000002, doAdd, doExit                      ; dict entry name  ( de--addr )
xSpace      dd $70000020, doEmit, doExit
xCR         dd $7000000D, doEmit, $7000000A, doEmit, doExit
xHA         dd doLit, HERE, doExit
xHere       dd xHA, doFetch, doExit
xLA         dd doLit, LAST, doExit
xLast       dd xLA, doFetch, doExit
xDot        dd doDot, xSpace, doExit
xCell       dd $70000000+CELL_SIZE, doExit
; ----------------------------------------------------------------

; A dictionary entry looks like this:
; XT/4, Size/1, Flags/1, Name/?, NTPad/?
; Note: NTPad must be at least 1 (NTPad means Null-Terminator/Pad)
; Examples: 
; - For "!",    it would be xxtt<08>0!0
; - For "CELL", it would be xxtt<12>0CELL00

macro addDict XT, Flags, Len, Name, NTPad
{
        dd      XT
        db      CELL_SIZE+2+Len+NTPad, Flags, Name
        db      NTPad dup 0
}

THE_DICT    rb 8*1024
            ; Pre-allocated for user-defined entries
LastInit:
        addDict doInc,    0, 2, "1+",    4
        addDict doDec,    0, 2, "1-",    4
        addDict doFetch,  0, 1, "@",     1
        addDict doFor,    0, 3, "FOR",   3
        addDict doI,      0, 1, "I",     1
        addDict doNext,   0, 4, "NEXT",  2
        addDict xCell,    0, 4, "CELL",  2
        addDict xHere,    0, 4, "HERE",  2
        addDict xHA,      0, 2, "HA",    4
        addDict xLast,    0, 4, "LAST",  2
        addDict xLA,      0, 2, "LA",    4
        addDict doLen,    0, 5, "S-LEN", 1
; TODO add more built-in dictionary entries here
        dd 0, 0
DICT_END:


THE_CODE    rd 64*1024
CODE_END:

THE_VARS    rb 256*1024
VARS_END:

.end start
