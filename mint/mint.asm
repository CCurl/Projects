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
mNEXT:  lodsb
        and     eax, 0x7f
        mov     ebx, [jmpTable+eax*4]
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
        sub     edx, 'a'
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
        je      .E
        cmp     al, 0
        jne     col1
.E:     mov     [HERE], esi
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
        jmp     mNEXT

; ******************************************************************************
; Quote
doQt:   lodsb
        cmp     al, '_'
        je      .x
        call    p1
        jmp     doQt
.x:     jmp     mNEXT

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
        mov     [forIndex], ebx
        mov     [forStart], esi
        ;mov     al, 'i'
        ;call    setReg
        jmp     mNEXT

fSkip:  lodsb
        cmp     al, ')'
        je      .x
        cmp     al, 0
        jne     fSkip
.x:     jmp     mNEXT

; ******************************************************************************
doNext: mov     eax, [forIndex]
        dec     eax
        cmp     eax, 0
        je      .out
        mov     [forIndex], eax
        mov     esi, [forStart]
        jmp     mNEXT
.out:   pop     eax
        jmp     mNEXT

; ******************************************************************************
f_UnknownOpcode:
        jmp repl

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
        mov     [HERE], ebx
        mov     al, 'm'
        call    setReg
        mov     al, 'h'
        call    setReg

; ******************************************************************************
repl:   call    ok
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
        jmp     mNEXT

; ******************************************************************************
section '.mem' data readable writable

jmpTable:
dd f_UnknownOpcode            ;   0
dd f_UnknownOpcode            ;   1
dd f_UnknownOpcode            ;   2
dd f_UnknownOpcode            ;   3
dd f_UnknownOpcode            ;   4
dd f_UnknownOpcode            ;   5
dd f_UnknownOpcode            ;   6
dd f_UnknownOpcode            ;   7
dd f_UnknownOpcode            ;   8
dd doNop                      ;   9
dd f_UnknownOpcode            ;  10
dd f_UnknownOpcode            ;  11
dd f_UnknownOpcode            ;  12
dd f_UnknownOpcode            ;  13
dd f_UnknownOpcode            ;  14
dd f_UnknownOpcode            ;  15
dd f_UnknownOpcode            ;  16
dd f_UnknownOpcode            ;  17
dd f_UnknownOpcode            ;  18
dd f_UnknownOpcode            ;  19
dd f_UnknownOpcode            ;  20
dd f_UnknownOpcode            ;  21
dd f_UnknownOpcode            ;  22
dd f_UnknownOpcode            ;  23
dd f_UnknownOpcode            ;  24
dd f_UnknownOpcode            ;  25
dd f_UnknownOpcode            ;  26
dd f_UnknownOpcode            ;  27
dd f_UnknownOpcode            ;  28
dd f_UnknownOpcode            ;  29
dd f_UnknownOpcode            ;  30
dd f_UnknownOpcode            ;  31
dd doNop                      ;  32 ( )
dd doStore                    ;  33 (!)
dd doDup                      ;  34 (")
dd doOver                     ;  35 (#)
dd doSwap                     ;  36 ($)
dd doMod                      ;  37 (%)
dd doAnd                      ;  38 (&)
dd doDrop                     ;  39 (')
dd doFor                      ;  40 (()
dd doNext                     ;  41 ())
dd doMult                     ;  42 (*)
dd doAdd                      ;  43 (+)
dd emit                       ;  44 (,)
dd doSub                      ;  45 (-)
dd doDot                      ;  46 (.)
dd doDiv                      ;  47 (/)
dd num                        ;  48 (0)
dd num                        ;  49 (1)
dd num                        ;  50 (2)
dd num                        ;  51 (3)
dd num                        ;  52 (4)
dd num                        ;  53 (5)
dd num                        ;  54 (6)
dd num                        ;  55 (7)
dd num                        ;  56 (8)
dd num                        ;  57 (9)
dd doCol                      ;  58 (:)
dd doRet                      ;  59 (;)
dd doLT                       ;  60 (<)
dd doEQ                       ;  61 (=)
dd doGT                       ;  62 (>)
dd f_UnknownOpcode            ;  63 (?)
dd doFetch                    ;  64 (@)
dd cmd                        ;  65 (A)
dd cmd                        ;  66 (B)
dd cmd                        ;  67 (C)
dd cmd                        ;  68 (D)
dd cmd                        ;  69 (E)
dd cmd                        ;  70 (F)
dd cmd                        ;  71 (G)
dd cmd                        ;  72 (H)
dd cmd                        ;  73 (I)
dd cmd                        ;  74 (J)
dd cmd                        ;  75 (K)
dd cmd                        ;  76 (L)
dd cmd                        ;  77 (M)
dd cmd                        ;  78 (N)
dd cmd                        ;  79 (O)
dd cmd                        ;  80 (P)
dd cmd                        ;  81 (Q)
dd cmd                        ;  82 (R)
dd cmd                        ;  83 (S)
dd cmd                        ;  84 (T)
dd cmd                        ;  85 (U)
dd cmd                        ;  86 (V)
dd cmd                        ;  87 (W)
dd cmd                        ;  88 (X)
dd cmd                        ;  89 (Y)
dd cmd                        ;  90 (Z)
dd f_UnknownOpcode            ;  91 ([)
dd bye                        ;  92 (\)
dd f_UnknownOpcode            ;  93 (])
dd doXOR                      ;  94 (^)
dd doQt                       ;  95 (_)
dd f_UnknownOpcode            ;  96 (`)
dd reg                        ;  97 (a)
dd reg                        ;  98 (b)
dd reg                        ;  99 (c)
dd reg                        ; 100 (d)
dd reg                        ; 101 (e)
dd reg                        ; 102 (f)
dd reg                        ; 103 (g)
dd reg                        ; 104 (h)
dd reg                        ; 105 (i)
dd reg                        ; 106 (j)
dd reg                        ; 107 (k)
dd reg                        ; 108 (l)
dd reg                        ; 109 (m)
dd reg                        ; 110 (n)
dd reg                        ; 111 (o)
dd reg                        ; 112 (p)
dd reg                        ; 113 (q)
dd reg                        ; 114 (r)
dd reg                        ; 115 (s)
dd reg                        ; 116 (t)
dd reg                        ; 117 (u)
dd reg                        ; 118 (v)
dd reg                        ; 119 (w)
dd reg                        ; 120 (x)
dd reg                        ; 121 (y)
dd reg                        ; 122 (z)
dd cFetch                     ; 123 ({)
dd doOr                       ; 124 (|)
dd cStore                     ; 125 (})
dd doInv                      ; 126 (~)
dd f_UnknownOpcode            ; 127

buf1        dd    4 dup(0)    ; Buffer
dStack      dd   32 dup 0
buf2        dd    4 dup(0)    ; Buffer
rStack      dd   32 dup 0
buf3        dd    4 dup(0)    ; Buffer

functions   dd  26 dup 0
regs        dd  36 dup 0
forStart    dd   1 dup 0
forIndex    dd   1 dup 0

THE_MEMORY  rb 64*1024
MEM_END:

.end start
