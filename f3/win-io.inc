; I/O support for Windows
; ------------------------------------------------------------------------------

macro EMITx
{
    call [putchar]
    pop eax
}

macro KEYx
{
    push ecx
    push edx
    call [_getch]
    pop edx
    pop ecx
    push eax
}

macro KEYq
{
    call [_kbhit]
    push eax
}

macro TRC ch
{
    push ecx
    push edx
    push ch
    EMITx
    pop edx
    pop ecx
}
