; I/O support for Windows
; ------------------------------------------------------------------------------

macro TRC ch
{
    push ch
    call [putchar]
    pop eax
}

s_EMIT:
    TRC
    NEXT
