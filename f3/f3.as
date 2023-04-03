; **************************************************************************
;
; This is a program that implements a Forth CPU/VM.
; The VM's memory space is the reserved buffer at THE_MEMORY.
;
; **************************************************************************
;                 x86 Register usage
; **************************************************************************
;
IP   equ esi       ; Program-Counter/Instruction-Pointer
SP   equ esp       ; Stack-Pointer
RSP  equ ebp       ; Return Stack-Pointer
TOS  equ [esp]     ; Top Of Stack
CELL_SIZE = 4
;
; **************************************************************************

; FOR_OS equ WINDOWS
; FOR_OS equ LINUX

match =WINDOWS, FOR_OS { include 'win-su.inc' }
match =LINUX,   FOR_OS { include 'lin-su.inc' }

; -------------------------------------------------------------------------------------
macro NEXT
{
       lodsd
       jmp DWORD [eax]
}

; ------------------------------------------------------------------------------
macro rPUSH val
{
    add ebp, CELL_SIZE
    mov [ebp], DWORD val
}

; ------------------------------------------------------------------------------
macro rPOP val
{
    mov val, DWORD [ebp]
    sub ebp, CELL_SIZE
}

; ------------------------------------------------------------------------------
macro DefWord Name, Length, Flags, Tag
{
        d_#Tag: dd LastTag     ; Link
                dd Tag         ; XT / Code-Field-Address (CFA)
                db Flags       ; Flags
                db Length      ; Length
                db Name        ; Name
                db 0           ; NULL-terminator
        align CELL_SIZE
        LastTag equ d_#Tag
        Tag:    dd  DOCOL
}

; ------------------------------------------------------------------------------
macro DefVar Name, Length, Flags, Tag
{
        d_#Tag: dd LastTag      ; Link
                dd Tag          ; XT / Code-Field-Address (CFA)
                db Flags        ; Flags
                db Length       ; Length
                db Name         ; Name
                db 0            ; NULL-terminator
        align CELL_SIZE
        LastTag equ  d_#Tag
        Tag:    dd   c_#Tag
        c_#Tag: push v_#Tag
        NEXT
}

; ------------------------------------------------------------------------------
macro DefCode Name, Length, Flags, Tag
{
        d_#Tag: dd LastTag     ; Link
                dd Tag         ; XT / Code-Field-Address (CFA)
                db Flags       ; Flags
                db Length      ; Length
                db Name        ; Name
                db 0           ; NULL-terminator
        align CELL_SIZE
        LastTag equ  d_#Tag
        Tag:    dd   c_#Tag
        c_#Tag:
}

; ------------------------------------------------------------------------------
LastTag equ 0

MEM_SZ    equ 4*1024*1024
IMMEDIATE equ 1
INLINE    equ 2

; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { include 'win-io.inc' }
match =LINUX,   FOR_OS { include 'lin-io.inc' }

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { section '.code' code readable executable }
match =LINUX,   FOR_OS { segment readable executable }

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
entry $
        mov [InitialESP], esp
        mov [InitialEBP], ebp

        mov ebp, rStack
        
        cld
        mov esi, coldStart
        NEXT

; -------------------------------------------------------------------------------------
coldStart:
        dd QUIT

; -------------------------------------------------------------------------------------
DefWord "QUIT",4,0,QUIT
        dd zRSP
q1:     dd INTERPRET
        dd BRANCH, q1

; -------------------------------------------------------------------------------------
DOCOL:
        rPUSH esi           ; push current esi on to the return stack
        add eax, CELL_SIZE  ; eax points to codeword, so add (CELL_SIZE) to make
        mov esi, eax        ; esi point to first data word
        NEXT

; -------------------------------------------------------------------------------------
DefCode "EXIT",4,0,EXIT
        rPOP esi            ; get esi back
        NEXT

; -------------------------------------------------------------------------------------
DefCode "0RSP",4,0,zRSP
        mov [lSP], lStack
        mov ebp, rStack
        NEXT

; -------------------------------------------------------------------------------------
DefCode "0SP",3,0,zSP
        mov esp, [InitialESP]
        NEXT

; -------------------------------------------------------------------------------------
DefWord "CR",2,0,CR
        dd LIT, 13, EMIT, LIT, 10, EMIT, EXIT

; -------------------------------------------------------------------------------------
DefWord "BL",2,0,fBL
        dd LIT, 32, EXIT

; -------------------------------------------------------------------------------------
DefWord "SPACE",5,0,SPACE
        dd fBL, EMIT, EXIT

; -------------------------------------------------------------------------------------
DefWord "OK",2,0,OK
        dd SPACE, LIT, 'O', EMIT, LIT, 'K', EMIT
        dd CR, EXIT

; -------------------------------------------------------------------------------------
DefWord "BENCH",5,0,BENCH
        dd LIT, 's', EMIT
        dd LIT, 500000000, LIT, 0, xtDO, xtLOOP
        dd LIT, 'e', EMIT
        dd EXIT

; -------------------------------------------------------------------------------------
DefCode ",",1,0,COMMA
        pop eax
        mov edx, [v_HERE]
        mov [edx], eax
        add edx, CELL_SIZE
        mov [v_HERE], edx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "EXEC",1,0,EXEC
        pop eax                         ; **TODO**
        NEXT

; -------------------------------------------------------------------------------------
DefWord "INTERPRET",9,0,INTERPRET
        dd OK
        dd TIB, LIT, 128, ACCEPT, DROP
        dd TIB, TOIN, fSTORE
in01:   dd xtWORD                       ; ( --str len )
        dd DUP1, zBRANCH, inX           ; dup 0= if drop2 exit then
        ; dd OVER, OVER, LIT, '(', EMIT, TYPE, LIT, ')', EMIT
        dd OVER, OVER, NUMq             ; ( str len--str len num flg )
        dd zBRANCH, in02                ; ( str len num f--str len num )
        dd NIP, NIP                     ; ( str len num--num )
        dd STATE, FETCH, LIT, 1, EQUALS ; if state=1, compile LIT, <num>
        dd zBRANCH, in01
        dd LIT, LIT, COMMA, COMMA        ; Compile LIT <num>
        dd BRANCH, in01
in02:   ; Not a number                  ; ( --str len num )
        ; Try to find it in the dictionary ...
        dd DROP                         ; Discard garbage 'num'
        ; dd OVER, OVER, LIT, '-', EMIT, TYPE, LIT, '-', EMIT
        dd FIND                         ; ( str len--[str len 0] | [xt flags 1] )
        dd zBRANCH, inERR               ; ( a b f--a b )
        dd LIT, 1, EQUALS               ; if flags=1, execute (immediate)
        dd nzBRANCH, inCX               ; ( xt f--xt )
        dd STATE, FETCH, LIT, 1, EQUALS ; STATE @ 1 = IF COMMA THEN
        dd zBRANCH, inCX
        dd COMMA                        ; ( xt -- ): compile
        dd BRANCH, in01
inCX:   dd EXEC                         ; ( xt-- ): execute
        dd BRANCH, in01
inERR:  ; Not a number or word - ERROR  ; ( --str len )
        dd BENCH                        ; **TEMP**
        dd LIT, '[', EMIT, TYPE, LIT
        dd ']', EMIT, LIT, '?', DUP1, EMIT, EMIT
        dd QUIT
inX:    dd DROP2, EXIT

; -------------------------------------------------------------------------------------
; digitQ: Set EDX=1 if EAX is a digit in base EBX, else EDX=0.
;           NOTE: EAX will be the converted digit if EDX=1.
digitQ: cmp eax, '0'                    ; <'0' means no
        jl dqNo
        sub eax, '0'                    ; Convert to number
        cmp eax, ebx                    ; 0 .. (base-1) => OK
        jl dqYes
        cmp ebx, 10                     ; BASE>10 needs more checking
        jle dqNo
        sub eax, 7                      ; Hex: 'A'-'0'-7 => 10
        cmp eax, 9
        jle dqNo
        cmp eax, ebx
        jge dqNo
dqYes:  mov edx, 1
        ret
dqNo:   mov edx, 0
        ret

; -------------------------------------------------------------------------------------
; NUMBER? try to convert (str len) into a number.
;         Stack effect: ( str len--num flg )
DefCode "NUMBER?",7,0,NUMq
        pop ecx                     ; len
        pop edi                     ; str
        push 0                      ; num
        push 0                      ; flg
        mov ebx, [v_BASE]
nq01:   test ecx, ecx               ; ecx=0 => end of string
        jz nqX
        movzx  eax, BYTE [edi]      ; eax => char
        call digitQ
        test edx, edx               ; edx=0 => invalid char
        jz nqNo
        pop edx                     ; Discard flg
        pop edx                     ; TOS => the current number
        imul edx, ebx               ; TOS = (TOS * BASE) + DIGIT
        add edx, eax
        push edx                    ; New num
        push 1                      ; New flg
        inc edi
        dec ecx
        jmp nq01
nqNo:   pop eax                     ; Discard flg
        push 0                      ; flg=0
nqX:    NEXT

; -------------------------------------------------------------------------------------
toLower: ; Make DL lower-case if between A-Z
        cmp dl, 'A'
        jl tlX
        cmp dl, 'Z'
        jg tlX
        add dl, 0x20
tlX:    ret

; -------------------------------------------------------------------------------------
; strEqI: Case-insensitive string-equals.
;         Params: string 1: EAX/EBX (string/len)
;                 string 2: ECX/EDX (string/len)
;         Return: EAX=0 => Not equal, or EAX=1 => equal
strEqI: cmp ebx, edx            ; Are the lengths the same?
        jne eqiNo
eqi01:  test ebx, ebx           ; No more chars?
        jz eqiYes
        mov dl, [eax]           ; char1
        call toLower
        mov dh, dl
        mov dl, [ecx]           ; char 2
        call toLower
        cmp dl, dh
        jne eqiNo
        inc eax                 ; So far, so good ... next chars
        inc ecx
        dec ebx
        jmp eqi01
eqiNo:  xor eax, eax            ; Strings are not equal
        ret
eqiYes: mov eax, 1              ; Strings are equal
        ret

; -------------------------------------------------------------------------------------
; FIND: Look for a word in the dictionary.
;       Stack: if found,     ( str len--xt flags 1 )
;              if not found, ( str len--str len 0 )
DefCode "FIND",4,0,FIND
        pop edx                 ; string
        pop ecx                 ; length
        mov eax, [v_LAST]       ; EAX: the current dict entry
fw01:   test eax, eax           ; end of dictionary?
        jz fwNo
        push edx                ; Save string
        push ecx                ; Save length
        push eax                ; Save the current word
        add eax, CELL_SIZE*2+1  ; add length offset
        movzx ebx, BYTE [eax]   ; length2
        inc eax                 ; string2
        call strEqI
        pop ebx                 ; Get current dict entry (was EAX)
        pop ecx                 ; Get length back
        pop edx                 ; Get string back
        test eax, eax           ; Not 0 means they are equal
        jnz fwYes
        mov eax, [ebx]          ; Not equal, move to the next word
        jmp fw01
fwNo:   push ecx                ; Not found, return (--name len 0)
        push edx
        push 0
        NEXT
fwYes:  add ebx, CELL_SIZE      ; Offset to XT
        push DWORD [ebx]        ; XT
        add ebx, CELL_SIZE      ; Offset to Flags
        movzx eax, BYTE [ebx]   ; Flags
        push eax
        push 1                  ; FOUND!
        NEXT

; -------------------------------------------------------------------------------------
DefWord "TIB",3,0,TIB
        dd LIT, tib, EXIT

; -------------------------------------------------------------------------------------
DefWord ">IN",3,0,TOIN
        dd LIT, toIn, EXIT

; -------------------------------------------------------------------------------------
; WORD: Parse the next word from TIB.
;       Stack: ( --str len )
;       NOTE: len=0 means end of line
DefCode "WORD",4,0,xtWORD
        mov ebx, curWord
        push ebx                ; addr
        push DWORD 0            ; len
        mov edx, [toIn]
        xor eax, eax
wd01:   mov al, [edx]           ; Skip any leading whitespace
        cmp al, 32
        jg wd02
        cmp al, 13
        je wdX
        cmp al, 10
        je wdX
        cmp al, 0
        je wdX
        inc edx
        jmp wd01
wd02:   mov [ebx], al           ; Collect word
        inc ebx
        inc edx
        inc DWORD TOS           ; Increment len
        mov al, [edx]           ; Next char
        cmp al, 33
        jl wdX
        cmp TOS, DWORD 32
        jl wd02
wdX:    mov [toIn], edx
        mov [ebx], BYTE 0       ; Add NULL terminator
        NEXT

; -------------------------------------------------------------------------------------
DefCode "ACCEPT",6,0,ACCEPT     ; ( addr sz--num )
        ioACCEPT
        NEXT

; -------------------------------------------------------------------------------------
DefCode "BRANCH",6,0,BRANCH
        lodsd
        mov esi, eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "0BRANCH",7,0,zBRANCH
        lodsd
        pop edx
        test edx, edx
        jnz zBX
        mov esi, eax
zBX:    NEXT

; -------------------------------------------------------------------------------------
DefCode "?BRANCH",7,0,nzBRANCH
        lodsd
        pop edx
        test edx, edx
        jz nzBX
        mov esi, eax
nzBX:   NEXT

; -------------------------------------------------------------------------------------
DefCode "DUP",3,0,DUP1
        mov eax, TOS
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "2DUP",4,0,DUP2
        pop ebx
        pop eax
        push eax
        push ebx
        push eax
        push ebx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "?DUP",4,0,qDUP
        mov eax, TOS
        test edx, edx
        jz qdX
        push eax
qdX:    NEXT

; -------------------------------------------------------------------------------------
DefCode "SWAP",4,0,SWAP
        pop eax
        pop ebx
        push eax
        push ebx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "OVER",4,0,OVER
        pop eax
        mov ebx, TOS
        push eax
        push ebx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "NIP",3,0,NIP
        pop ebx
        pop eax
        push ebx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "DROP",4,0,DROP
        pop eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "2DROP",5,0,DROP2
        pop eax
        pop eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "LIT",3,0,LIT
        lodsd
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "1+",2,0,INCTOS
        inc DWORD TOS
        NEXT

; -------------------------------------------------------------------------------------
DefCode "1-",2,0,DECTOS
        dec DWORD TOS
        NEXT

; -------------------------------------------------------------------------------------
DefCode "@",1,0,FETCH
        mov edx, TOS
        mov eax, [edx]
        mov TOS, eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "C@",2,0,CFETCH
        mov edx, TOS
        movzx eax, BYTE [edx]
        mov TOS, eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "!",1,0,fSTORE
        pop edx
        pop eax
        mov [edx], eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "C!",2,0,CSTORE
        pop edx
        pop eax
        mov [edx], al
        NEXT

; -------------------------------------------------------------------------------------
DefCode "+",1,0,xtADD
        pop edx
        pop eax
        add eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "-",1,0,xtSUB
        pop edx
        pop eax
        sub eax, edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "*",1,0,xtMUL
        pop edx
        pop eax
        mul edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "/",1,0,xtDIV
        pop ebx
        pop eax
        xor edx, edx
        idiv ebx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "=",1,0,EQUALS
        pop ebx
        pop eax
        cmp eax, ebx
        je fTrue
fFalse: push 0
        NEXT
fTrue:  push -1
        NEXT

; -------------------------------------------------------------------------------------
DefCode "/MOD",4,0,xSLMOD
        pop ebx
        pop eax
        xor edx, edx
        idiv ebx
        push edx
        push eax
        NEXT

; -------------------------------------------------------------------------------------
DefCode "KEY",3,0,KEY         ; ( ch-- )
        KEYx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "KEY?",4,0,KEYQ         ; ( ch-- )
        KEYq
        NEXT

; -------------------------------------------------------------------------------------
DefCode "EMIT",4,0,EMIT         ; ( ch-- )
        EMITx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "TYPE",4,0,TYPE         ; ( addr len-- )
        ioTYPE
        NEXT

; -------------------------------------------------------------------------------------
DefCode "TIMER",5,0,TIMER     ; ( --n )
        ioTIMER
        NEXT

; -------------------------------------------------------------------------------------
DefWord "WORDS",5,0,WORDS
        ; **TODO**
        dd EXIT

; -------------------------------------------------------------------------------------
DefCode "DO", 2, 0, xtDO
        mov edx, [lSP]
        pop ecx                 ; From / I
        pop ebx                 ; To
        add edx, CELL_SIZE
        mov [edx], DWORD esi    ; loop start
        add edx, CELL_SIZE
        mov [edx], DWORD ebx
        add edx, CELL_SIZE
        mov [edx], DWORD ecx
        mov [lSP], DWORD edx
        NEXT

; -------------------------------------------------------------------------------------
DefCode "LOOP", 4, 0, xtLOOP
        mov edx, [lSP]
        mov ecx, DWORD [edx]            ; I
        inc ecx
        mov [edx], ecx
        cmp ecx, DWORD [edx-CELL_SIZE]  ; TO
        jge lpDone
        mov esi, DWORD [edx-CELL_SIZE*2]            ; Loop start
        NEXT
lpDone: sub edx, CELL_SIZE*3
        mov [lSP], DWORD edx
        NEXT

; -------------------------------------------------------------------------------------
; Some variables ...
; -------------------------------------------------------------------------------------
DefVar "(HERE)",6,0,HERE
DefVar "(LAST)",6,0,LAST
DefVar "STATE",5,0,STATE
DefVar "BASE",4,0,BASE
DefVar "PAD",4,0,PAD

; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------
match =WINDOWS, FOR_OS { section '.bdata' readable writable }
match =LINUX,   FOR_OS { segment readable writable }

; -------------------------------------------------------------------------------------
ver db 'f3 v0.1', 13, 10, 0

InitialESP dd 0
InitialEBP dd 0
SaveEBP    dd 0

rStack   dd 64 dup (0)           ; The return stack
lStack   dd 64 dup (0)           ; The loop stack
lSP      dd lStack               ; The loop stack pointer

regs     dd  100 dup (0)           ; My pseudo-registers
rbase    dd    0

toIn     dd    0                ; >IN - current char ptr
tib      db  128 dup (0)        ; The Tef Input Buffer
curWord  db   32 dup (0)        ; The current word
buf4     db    4 dup (0)        ; A buffer for EMIT (LINUX)

v_HERE:  dd xHERE
v_LAST:  dd LastTag
v_STATE: dd 0
v_BASE:  dd 10
v_PAD:   dd 64 dup(0)

xHERE:
MEM:     rb MEM_SZ  
MEM_END:
