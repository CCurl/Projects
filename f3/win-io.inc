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

macro ACCEPTx
{
    pop eax
    pop eax
    push 0
}

macro TYPEx
{
    pop eax
    pop eax
}

macro ioTIMER
{
    call [GetTickCount]
    push eax
}
