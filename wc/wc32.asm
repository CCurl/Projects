; WC32 - a Tachyon Forth inspired 32-bit system

; FOR_OS equ WINDOWS
; FOR_OS equ LINUX

match =WINDOWS, FOR_OS {
        format PE console
        include 'win32ax.inc'
        .code
}

match =LINUX, FOR_OS {
        format ELF executable 3
        segment readable executable
}

; ******************************************************************************
; ******************************************************************************

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
macro m_getTOS val { mov val, TOS }
macro m_setTOS val { mov TOS, val }
macro m_get2ND val { mov val, [STKP] }
macro m_set2ND val { mov [STKP], val }

macro m_push val {
       add STKP, CELL_SIZE
       mov [STKP], TOS
       m_setTOS val
}

; ******************************************************************************
macro m_pop val {
       m_getTOS val
       mov TOS, [STKP]
       sub STKP, CELL_SIZE
}

; ******************************************************************************
entry $
start:
match =WINDOWS, FOR_OS {
        invoke GetStdHandle, STD_INPUT_HANDLE
        mov    [hStdIn], eax
        
        invoke GetStdHandle, STD_OUTPUT_HANDLE
        mov    [hStdOut], eax
}
        mov     ebx, THE_CODE
        mov     [HERE], ebx
        mov     ebx, THE_VARS
        mov     [VHERE], ebx

warm:   mov     eax, rStack
        mov     [rStackPtr], eax
        mov     STKP, dStack
        mov     TOS, 0
        mov     esi, THE_ROM
        cld
        jmp     wcRun
        call    doBye
        ret

; ******************************************************************************
; ** The bread and butter of the system
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
        ; mov     esp, [InitialESP]
        mov     esi, xWarm
        ret

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
        jnc     .L
        inc     BYTE [esp]
        neg     eax
.L:     push    ebx
        mov     ebx, 10
        mov     edx, 0
        div     ebx
        add     dl, '0'
        dec     ecx 
        mov     BYTE [ecx], dl
        pop     ebx
        inc     ebx
        test    eax, eax
        jnz     .L
        pop     eax             ; get the negative flag
        test    eax, eax        ; 0 means not negative
        jz      .X
        dec     ecx
        mov     BYTE [ecx], '-'
        inc     ebx
.X:     ret

; ******************************************************************************
doItoA: m_pop   eax
        call    iToA
        m_push  ecx
        m_push  ebx
        ret

; ******************************************************************************
doJmp:  m_pop  ebx
        lodsd
doJ:    mov    esi, eax
        ret

; ******************************************************************************
doJmpZ: m_pop  ebx
        lodsd
        test   ebx, ebx
        jz     doJ
        ret

; ******************************************************************************
doJmpNZ: m_pop  ebx
        lodsd
        test    ebx, ebx
        jnz     doJ
        ret

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
doCFetch: xor     eax, eax
        mov     al, BYTE [TOS]
        mov     TOS, eax
        ret

; ******************************************************************************
doCStore: m_pop edx
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
doQt:   lodsb   ; TODO
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

doOK:   m_push  okStr
        m_push  5
        call    doType
        ret

; ******************************************************************************
match =WINDOWS, FOR_OS {
        doBye:  invoke  ExitProcess, 0
                ret
        doTimer: invoke GetTickCount
                 m_push  eax
                 ret
        doEmit: m_pop   eax
                mov     [buf1], al
                invoke  WriteConsole, [hStdOut], buf1, 1, NULL, NULL
                ret
        doType: m_pop   eax     ; Len ( addr len-- )
                m_pop   ebx     ; Addr
                invoke  WriteConsole, [hStdOut], ebx, eax, NULL, NULL
                ret
        doReadL: m_pop  edx              ; buffer size
                 m_pop  ecx              ; buffer
                 invoke ReadConsole, [hStdIn], ecx, edx, bytesRead, 0
                 mov    eax, [bytesRead]
                 m_push eax
                 ret
        doQKey: invoke _kbhit
                m_push  eax
                ret
        doKey:  invoke  _getch
                m_push  eax
                ret
}

match =LINUX, FOR_OS {
        doBye:  ; invoke  LinuxExit, 0
                ret
        doTimer: ; invoke LinuxTimer
                m_push  eax
                ret
        doEmit: m_pop   eax
                mov	[buf1], al      ; put char in message
                mov	eax,4		; system call number (sys_write)
                mov	ebx,1		; file descriptor (stdout)
                mov	ecx,buf1        ; message to write
                mov	edx,1		; message length
                int	0x80		; call kernel
                ret
        doType: m_pop   edx             ; Len ( string len-- )
                m_pop   ecx             ; String
                mov	eax,4           ; system call number (sys_write)
                mov	ebx,1           ; file descriptor (stdout)
                int     0x80
                ret
        doReadL: m_pop edx              ; buffer size
                 m_pop  ecx             ; buffer
                 mov    ebx, 0          ; stdin
                 mov    eax, 3          ; sys_read
                 int    0x80
                 m_push eax
    ret
        ; doQKey: ; invoke LinuxKey
        ;         m_push  eax
        ;         ret
        ; doKey: ; invoke LinuxQKey
        ;         m_push  eax
        ;         ret
}

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
        m_push  ecx
        m_push  ebx
        call    doType
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
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
doLit:  lodsd
        m_push eax
        ret

; ******************************************************************************
; ******************************************************************************
primEnd:

; ******************************************************************************
; ******************************************************************************
; ******************************************************************************
match =WINDOWS, FOR_OS {
        section '.idata' import data readable writeable
        library kernel32, 'kernel32.dll', msvcrt, 'msvcrt.dll', conio, 'conio32.dll'
        import kernel32, GetTickCount, 'GetTickCount', ExitProcess, 'ExitProcess', \
                WriteConsole, 'WriteConsoleA', ReadConsole, 'ReadConsoleA', \
                GetStdHandle, 'GetStdHandle'
        import msvcrt, _getch, '_getch', _kbhit, '_kbhit'
        .data
}
match =LINUX,   FOR_OS { segment readable writable }

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
LAST        dd  d_tg999
HERE1       dd  ?
TIB         dd  128 dup 0       ; TIB

buf1        db   16 dup 0       ; Buffer
dStack      dd   64 dup 0       ; Data stack
buf2        dd    4 dup 0       ; Buffer
rStack      dd   64 dup 0       ; Return stack
buf3        dd    4 dup 0       ; Buffer
lStack      dd   64 dup 0       ; Loop stack
buf4        dd    4 dup 0       ; Buffer

; ----------------------------------------------------------------
x0 equ $70000000
x1 equ $70000001
x2 equ $70000002

THE_ROM:
xCold       dd xHA, xDot, xHere, xDot, xLast, xDot, xCell, xDot
                dd xCR, xWords
                dd xCR, xCR, $7000000A, doFor, doI, doInc, xDot, doNext
xWarm       dd xInterp, xBench, doJmp, xWarm
xInterp     dd doOK, xTIB, xTIBSZ, xAccept, doDec, doDec, xTIB, doAdd, x0, doSwap, doCStore
                dd xTIB, doDup, doLen, doType, xSpace
                dd doExit
xDeShow     dd doDup, xDeName                           ; First char of name    ( a1--a2 )
                dd doDup, doLen, doType                 ; Name length
                dd xDeNext, doExit                      ; Next entry
xDeShowVB   dd xCR, doDup, xDeNext, xDot                ; Next    ( a1--a2 )
                dd doDup, xDeXT, xDot                   ; XT
                dd doDup, xDeFlags, xDot                ; Flags
                dd doDup, xDeName                       ; First char of name
                dd doDup, doLen, doType                 ; Name length
                dd xDeNext, doExit                      ; Next entry
xDeNext     dd doFetch, doExit                                          ; dict entry Next  ( de--next )
xDeXT       dd xCell, doAdd, doFetch, doExit                            ; dict entry XT    ( de--xt )
xDeFlags    dd x2, xCells, doAdd, doCFetch, doExit                      ; dict entry flags ( de--flags )
xDeLen      dd x2, xCells, doAdd, x1, doAdd, doCFetch, doExit           ; dict entry len   ( de--len )
xDeName     dd x2, xCells, doAdd, x2, doAdd, doExit                     ; dict entry name  ( de--addr )
xSpace      dd $70000020, doEmit, doExit
xCR         dd $7000000D, doEmit, $7000000A, doEmit, doExit
xTab        dd $70000009, doEmit, doExit
xHA         dd doLit, HERE, doExit
xHere       dd xHA, doFetch, doExit
xLA         dd doLit, LAST, doExit
xLast       dd xLA, doFetch, doExit
xDot        dd doDot, xSpace, doExit
xCell       dd $70000000+CELL_SIZE, doExit
xCells      dd $70000000+CELL_SIZE, doMult, doExit
x2Cells     dd x2, xCells, doExit
xOK         dd doOK, doExit
xTIB        dd doLit, TIB, doExit
xTIBSZ      dd $70000080, doExit
xAccept     dd doReadL, doExit
xWords      dd xLast
xWdsLoop        dd xDeShowVB, xTab, doDup, doJmpNZ, xWdsLoop
                dd doDrop, doExit
xBench      dd doTimer, doLit, 500000000, doDup, xDot, doFor, doNext
            dd doTimer, doSwap, doSub, xDot, doExit
; ----------------------------------------------------------------
; A dictionary entry looks like this:
; Next/4, XT/4, Flags/1, Len/1, Name/?, NULL/1

LastTag equ 0

macro addDict XT, Flags, Len, Name, Tag
{
        align CELL_SIZE
        d_#Tag: dd LastTag
                dd XT
                db Flags, Len, Name, 0
        LastTag equ d_#Tag
}

THE_DICT:
        addDict doBye,    0, 3, "BYE",   tg099
        addDict doInc,    0, 2, "1+",    tg100
        addDict doDec,    0, 2, "1-",    tg101
        addDict doFetch,  0, 1, "@",     tg102
        addDict doFor,    0, 3, "FOR",   tg103
        addDict doI,      0, 1, "I",     tg104
        addDict doNext,   0, 4, "NEXT",  tg105
        addDict xTIB,     0, 3, "TIB",   tg106
        addDict xTab,     0, 3, "TAB",   tg107
        addDict xCR,      0, 2, "CR",    tg108
        addDict xWords,   0, 5, "WORDS", tg109
        addDict xCell,    0, 4, "CELL",  tg110
        addDict doItoA,   0, 3, "I>A",   tg111
        addDict xHere,    0, 4, "HERE",  tg112
        addDict xHA,      0, 2, "HA",    tg113
        addDict xLast,    0, 4, "LAST",  tg114
        addDict xLA,      0, 2, "LA",    tg115
        addDict doLen,    0, 5, "S-LEN", tg116
        addDict doKey,    0, 3, "KEY",   tg117
        addDict doQKey,   0, 4, "QKEY",  tg118
        addDict doDup,    0, 3, "DUP",   tg999
; TODO add more built-in dictionary entries here
        rd  64*1024
DICT_END:


THE_CODE    rd 64*1024
CODE_END:

THE_VARS    rb 256*1024
VARS_END:
