; MINT - a Minimal Interpreter

format PE console
include 'win32ax.inc'

.data

hStdIn      dd  ?
hStdOut     dd  ?
okStr       db  "mint>", 0
crlf        db  13, 10
outBuf      db  256 dup ?
bytesRead   dd  ?
unkOP       db  "-unk-"
dDepth      dd  0
rDepth      dd  0
rStackPtr   dd  ?
HERE        dd  ?
HERE1       dd  ?

; ******************************************************************************
; ******************************************************************************
section '.code' code readable executable

; ------------------------------------------------------------------------------
; MACROS

CELL_SIZE equ 4
REG1 equ eax         ; Free register #1
REG2 equ ebx         ; Free register #2
REG3 equ ecx         ; Free register #3
REG4 equ edx         ; Free register #4
TOS  equ edi         ; Top-Of-Stack
PCIP equ esi         ; Program-Counter/Instruction-Pointer
STKP equ ebp         ; Stack-Pointer

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
macro m_drop
{
       dec [dDepth]
       mov TOS, [STKP]
       sub STKP, CELL_SIZE
}

macro m_pop val
{
       m_getTOS val
       m_drop
}

; ******************************************************************************
rpush:  push    eax
        add     [rStackPtr], CELL_SIZE
        mov     eax, [rStackPtr]
        mov     [eax], esi
        pop     eax
        ret
 
rpop:   push    eax
        mov     eax, [rStackPtr]
        mov     esi, [eax]
        sub     [rStackPtr], CELL_SIZE
        pop     eax
        ret

; ******************************************************************************
mNEXT:  cmp     [dDepth], 1
        jge     nxtOK
        mov     [dDepth], 0
        mov     STKP, dStack
nxtOK:  cmp     esi, [HERE1]
        jge     s0
        lodsb
        cmp     al, 126
        jg      s0
        movzx   edx, al
        mov     ebx, [jmpTable+edx*4]
        jmp     ebx

; ******************************************************************************
doNop:  jmp     mNEXT

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
        pop     eax
        cmp     eax, 0
        je      i2aX
        dec     ecx
        mov     BYTE [ecx], '-'
        inc     ebx
i2aX:   ret

; ******************************************************************************
regAddr: movzx   edx, al
        sub edx, 'a'
        shl     edx, 2
        add     edx, regs
        ret

; ******************************************************************************
setReg: call regAddr
        mov [edx], ebx
        ret

; ******************************************************************************
getReg: call    regAddr
        mov     ebx, [edx]
        ret

; ******************************************************************************
; register
reg:    call    regAddr
        m_push  edx
        jmp     mNEXT

; ******************************************************************************
fnAddr: movzx  edx, al
        sub     edx, 'A'
        shl     edx, 2
        add     edx, functions   
        ret

; ******************************************************************************
doCol:  lodsb
        mov     bx, 'AZ'
        call    betw
        cmp     bl, 0
        je      colX
        call    fnAddr
        mov     [edx], esi
col1:   cmp     esi, [HERE1]
        jge     colX
        lodsb
        cmp     al, ';'
        jne     col1
        mov     [HERE], esi
        mov     al, 'h'
        mov     ebx, esi
        call    setReg
colX:   jmp     mNEXT

; ******************************************************************************
doRet:  call    rpop
        jmp     mNEXT

; ******************************************************************************
; command
cmd:    call    fnAddr
        mov     ebx, [edx]
        cmp     ebx, 0
        je      mNEXT
        call    rpush
        mov     esi, ebx
        jmp     mNEXT

; ******************************************************************************
doFetch: m_getTOS   edx
        m_setTOS    [edx]
        jmp         mNEXT

; ******************************************************************************
doStore: m_pop  edx
        m_pop   eax
        mov     [edx], eax
        jmp     mNEXT

; ******************************************************************************
cFetch: m_getTOS   edx
        mov         TOS, [edx]
        and         TOS, $ff
        jmp         mNEXT

; ******************************************************************************
cStore: m_pop  edx
        m_pop   eax
        mov     BYTE [edx], al
        jmp     mNEXT

; ******************************************************************************
; Number input
num:    sub     al, '0'
        and     eax, $FF
        mov     edx, eax
n1:     mov     al, [esi]
        mov     bx, '09'
        call    betw
        cmp     bl, 0
        je      nx
        sub     al, '0'
        imul    edx, edx, 10
        add     edx, eax
        inc     esi
        jmp     n1
nx:     m_push  edx
        jmp     mNEXT

; ******************************************************************************
; Quote
doQt:   lodsb
        cmp     al, '_'
        je      qx
        call    p1
        jmp     doQt
qx:     jmp     mNEXT

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
        cmp     ebx, 0
        je      fSkip
        push    esi
        mov     al, 'i'
        call    setReg
        jmp     mNEXT

fSkip:  lodsb
        cmp     al, ')'
        jne     fSkip

; ******************************************************************************
doNext: mov     al, 'i'
        call    getReg
        dec     ebx
        call    setReg
        cmp     ebx, 0
        jz      nxtX
        mov     esi, [esp]
        jmp     mNEXT
nxtX:   pop     eax
        jmp     mNEXT

; ******************************************************************************
f_UnknownOpcode:
        jmp s0

; ******************************************************************************
bye:    invoke  ExitProcess, 0

; ******************************************************************************
ok:     call    doCrLf
        invoke  WriteConsole, [hStdOut], okStr, 5, NULL, NULL
        ret

; ******************************************************************************
doMult: m_pop   eax
        imul    TOS, eax
        jmp     mNEXT

; ******************************************************************************
doSub:  m_pop   eax
        sub     TOS, eax
        jmp     mNEXT

; ******************************************************************************
doAdd:  m_pop   eax
        add     TOS, eax
        jmp     mNEXT

; ******************************************************************************
doMod:  m_pop   ebx
        cmp     ebx, 0
        je      mNEXT
        m_pop   eax
        mov     edx, 0
        idiv    ebx
        m_push  edx
        jmp     mNEXT

; ******************************************************************************
doDiv:  m_pop   ebx
        cmp     ebx, 0
        je      mNEXT
        m_pop   eax
        mov     edx, 0
        idiv    ebx
        m_push  eax
        jmp     mNEXT

; ******************************************************************************
doAnd:  m_pop   eax
        and     TOS, eax
        jmp     mNEXT

; ******************************************************************************
doOr:   m_pop   eax
        or      TOS, eax
        jmp     mNEXT

; ******************************************************************************
doXOR:  m_pop   eax
        xor     TOS, eax
        jmp     mNEXT

; ******************************************************************************
doNeg:  neg     TOS
        jmp     mNEXT

; ******************************************************************************
doInv:  not     TOS
        jmp     mNEXT

; ******************************************************************************
cmpT:   mov     TOS, 1
        jmp     mNEXT

; ******************************************************************************
cmpF:   mov     TOS, 0
        jmp     mNEXT

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
emit:   m_pop   eax
        call    p1
        jmp     mNEXT

; ******************************************************************************
doCrLf: mov     al, 13
        call    p1
        mov     al, 10
        jmp    p1

; ******************************************************************************
sDot:   push    eax
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
doDot:  call    sDot
        jmp     mNEXT

; ******************************************************************************
doTimer: invoke GetTickCount
        m_push  eax
        jmp     mNEXT

; ******************************************************************************
doDup:  m_push  TOS
        jmp     mNEXT

; ******************************************************************************
doSwap: m_get2ND    eax
        m_set2ND    TOS
        m_setTOS    eax
        jmp         mNEXT

; ******************************************************************************
doOver: m_get2ND    eax
        m_push      eax
        jmp         mNEXT

; ******************************************************************************
doDrop: m_drop
        jmp         mNEXT

; ******************************************************************************
p1:     push eax
        mov     [outBuf], al
        invoke  WriteConsole, [hStdOut], outBuf, 1, NULL, NULL
        pop eax
        ret

; ******************************************************************************
s_SYS_INIT:
            ; Return stack
            mov eax, rStack - CELL_SIZE
            mov [rStackPtr], eax
            mov [rDepth], 0
            ; Data stack
            mov STKP, dStack
            mov TOS, 0
            ret        

; ******************************************************************************
start:
        invoke GetStdHandle, STD_INPUT_HANDLE
        mov    [hStdIn], eax
        
        invoke GetStdHandle, STD_OUTPUT_HANDLE
        mov    [hStdOut], eax

        call    s_SYS_INIT
        mov     ebx, THE_MEMORY
        mov     al, 'm'
        call    setReg
        mov     [HERE], ebx
    
s0:     call    ok
        invoke  ReadConsole, [hStdIn], [HERE], 128, bytesRead, 0
        sub     [bytesRead], 2
        mov     ebx, [HERE]
        add     ebx, [bytesRead]
        mov     [HERE1], ebx
        cld
        mov     esi, [HERE]
        jmp     mNEXT
    

; ******************************************************************************
section '.mem' data readable writable

jmpTable:
dd f_UnknownOpcode            ; # 00 ()
dd f_UnknownOpcode            ; # 01 (☺)
dd f_UnknownOpcode            ; # 02 (☻)
dd f_UnknownOpcode            ; # 03 (♥)
dd f_UnknownOpcode            ; # 04 (♦)
dd f_UnknownOpcode            ; # 05 (♣)
dd f_UnknownOpcode            ; # 06 (♠)
dd f_UnknownOpcode            ; # 07 ()
dd f_UnknownOpcode            ; # 08 ()
dd doNop                      ; # 09 ()
dd f_UnknownOpcode            ; # 010 ()
dd f_UnknownOpcode            ; # 011 ()
dd f_UnknownOpcode            ; # 012 ()
dd f_UnknownOpcode            ; # 013 ()
dd f_UnknownOpcode            ; # 014 ()
dd f_UnknownOpcode            ; # 015 ()
dd f_UnknownOpcode            ; # 016 (►)
dd f_UnknownOpcode            ; # 017 (◄)
dd f_UnknownOpcode            ; # 018 (↕)
dd f_UnknownOpcode            ; # 019 (‼)
dd f_UnknownOpcode            ; # 020 (¶)
dd f_UnknownOpcode            ; # 021 (§)
dd f_UnknownOpcode            ; # 022 (▬)
dd f_UnknownOpcode            ; # 023 (↨)
dd f_UnknownOpcode            ; # 024 (↑)
dd f_UnknownOpcode            ; # 025 (↓)
dd f_UnknownOpcode            ; # 026 (→)
dd f_UnknownOpcode            ; # 027 (
dd f_UnknownOpcode            ; # 028 (∟)
dd f_UnknownOpcode            ; # 029 (↔)
dd f_UnknownOpcode            ; # 030 (▲)
dd f_UnknownOpcode            ; # 031 (▼)
dd doNop                      ; # 032 ( )
dd doStore                    ; # 033 (!)
dd doDup                      ; # 034 (")
dd doOver                     ; # 035 (#)
dd doSwap                     ; # 036 ($)
dd doMod                      ; # 037 (%)
dd doAnd                      ; # 038 (&)
dd doDrop                     ; # 039 (')
dd doFor                      ; # 040 (()
dd doNext                     ; # 041 ())
dd doMult                     ; # 042 (*)
dd doAdd                      ; # 043 (+)
dd emit                       ; # 044 (,)
dd doSub                      ; # 045 (-)
dd doDot                      ; # 046 (.)
dd doDiv                      ; # 047 (/)
dd num                        ; # 048 (0)
dd num                        ; # 049 (1)
dd num                        ; # 050 (2)
dd num                        ; # 051 (3)
dd num                        ; # 052 (4)
dd num                        ; # 053 (5)
dd num                        ; # 054 (6)
dd num                        ; # 055 (7)
dd num                        ; # 056 (8)
dd num                        ; # 057 (9)
dd doCol                      ; # 058 (:)
dd doRet                      ; # 059 (;)
dd doLT                       ; # 060 (<)
dd doEQ                       ; # 061 (=)
dd doGT                       ; # 062 (>)
dd f_UnknownOpcode            ; # 063 (?)
dd doFetch                    ; # 064 (@)
dd cmd                        ; # 065 (A)
dd cmd                        ; # 066 (B)
dd cmd                        ; # 067 (C)
dd cmd                        ; # 068 (D)
dd cmd                        ; # 069 (E)
dd cmd                        ; # 070 (F)
dd cmd                        ; # 071 (G)
dd cmd                        ; # 072 (H)
dd cmd                        ; # 073 (I)
dd cmd                        ; # 074 (J)
dd cmd                        ; # 075 (K)
dd cmd                        ; # 076 (L)
dd cmd                        ; # 077 (M)
dd cmd                        ; # 078 (N)
dd cmd                        ; # 079 (O)
dd cmd                        ; # 080 (P)
dd cmd                        ; # 081 (Q)
dd cmd                        ; # 082 (R)
dd cmd                        ; # 083 (S)
dd cmd                        ; # 084 (T)
dd cmd                        ; # 085 (U)
dd cmd                        ; # 086 (V)
dd cmd                        ; # 087 (W)
dd cmd                        ; # 088 (X)
dd cmd                        ; # 089 (Y)
dd cmd                        ; # 090 (Z)
dd f_UnknownOpcode            ; # 091 ([)
dd bye                        ; # 092 (\)
dd f_UnknownOpcode            ; # 093 (])
dd doXOR                      ; # 094 (^)
dd doQt                       ; # 095 (_)
dd f_UnknownOpcode            ; # 096 (`)
dd reg                        ; # 097 (a)
dd reg                        ; # 098 (b)
dd reg                        ; # 099 (c)
dd reg                        ; # 100 (d)
dd reg                        ; # 101 (e)
dd reg                        ; # 102 (f)
dd reg                        ; # 103 (g)
dd reg                        ; # 104 (h)
dd reg                        ; # 105 (i)
dd reg                        ; # 106 (j)
dd reg                        ; # 107 (k)
dd reg                        ; # 108 (l)
dd reg                        ; # 109 (m)
dd reg                        ; # 110 (n)
dd reg                        ; # 111 (o)
dd reg                        ; # 112 (p)
dd reg                        ; # 113 (q)
dd reg                        ; # 114 (r)
dd reg                        ; # 115 (s)
dd reg                        ; # 116 (t)
dd reg                        ; # 117 (u)
dd reg                        ; # 118 (v)
dd reg                        ; # 119 (w)
dd reg                        ; # 120 (x)
dd reg                        ; # 121 (y)
dd reg                        ; # 122 (z)
dd cFetch                     ; # 123 ({)
dd doOr                       ; # 124 (|)
dd cStore                     ; # 125 (})
dd doInv                      ; # 126 (~)

buf1        dd    4 dup(0)    ; Buffer
dStack      dd   32 dup 0
buf2        dd    4 dup(0)    ; Buffer
rStack      dd   32 dup 0
buf3        dd    4 dup(0)    ; Buffer

functions   dd  26 dup 0
regs        dd  26 dup 0

THE_MEMORY  rb 64*1024
MEM_END:

.end start
