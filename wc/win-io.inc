; I/O support for Windows
; ------------------------------------------------------------------------------

xtEMIT:
    dPOP eax
    push eax
    call [putchar]
    pop eax
    ; invoke putchar, eax
    ret

xtKEY:
    invoke _getch
    dPUSH eax
    ret

xtKEYQ:
    invoke _kbhit
    dPUSH eax
    ret

xtACCEPT:
    ret ; TODO


xtTIMER:
    invoke GetTickCount
    dPUSH eax
    ret
