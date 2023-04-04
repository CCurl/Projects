; I/O support for Windows
; ------------------------------------------------------------------------------

macro ioEMIT
{
    call [putchar]
    pop eax
}

macro ioKEY
{
    push ecx
    push edx
    call [_getch]
    pop edx
    pop ecx
    push eax
}

macro ioKEYq
{
    call [_kbhit]
    push eax
}

macro TRC ch
{
    push ecx
    push edx
    push ch
    ioEMIT
    pop edx
    pop ecx
}

macro ioACCEPT
{
    pop eax             ; ( buffer len--buffer )
    ; pop eax
    call [gets]
}

macro ioTYPE
{
            pop ecx
            pop edx
    type01: test ecx, ecx
            jz typeX
            movzx eax, BYTE [edx]
            TRC eax
            inc edx
            dec ecx
            jmp type01
    typeX:  
}

macro ioTIMER
{
    call [GetTickCount]
    push eax
}
